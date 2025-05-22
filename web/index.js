const express = require("express");
const NodeMediaServer = require('node-media-server');
var fs = require('fs');
var json_config;
let last_online = Date.now();
var sessions = {}
const app = express();

fs.readFile('config.json', 'utf8', function (err, data) {
    if (err) throw err;
    json_config = JSON.parse(data);
});

const transformationConfig = {
  ffmpeg: "ffmpeg",
  tasks: [
    {
      app: "live",
      hls: true,
      hlsFlags: "[hls_time=2:hls_list_size=3:hls_flags=delete_segments]",
      hlsKeep: false,
    },
  ],
  MediaRoot: "/media",
};

const config = {
    rtmp: {
        port: 1935,
        chunk_size: 60000,
        gop_cache: true,
        ping: 10,
        ping_timeout: 60
    },
    http: {
        port: 8002,
        allow_origin: '*',
        mediaroot: '/media'
    },
    trans: transformationConfig
};

// Encode and parse data.
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

const PORT = process.env.PORT || 8001;

let data = {
    online: false, 
    data: {
        "speed": 0,
        "speed_avg": 0,
        "speed_max": 0,
        "rpm": 0,
        "rpm_avg": 0,
        "rpm_max": 0,
        "power": 0,
        "power_avg": 0,
        "power_max": 0,
        "throttle": 0,
        "throttle_avg": 0,
        "throttle_max": 0
    }
}

async function sha256Hash(text) {
  const encoder = new TextEncoder();
  const data = encoder.encode(text);
  const hashBuffer = await crypto.subtle.digest('SHA-256', data);
  const hashArray = Array.from(new Uint8Array(hashBuffer));
  const hashHex = hashArray.map(byte => byte.toString(16).padStart(2, '0')).join('');
  return hashHex;
}

function randStr(len) {
    const characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';

    let result = '';
    for (let i = 0; i < len; i++) {
        result += characters.charAt(Math.floor(Math.random() * characters.length));
    }

    return result;
}

app.post("/api/logout", (req, res) => {
    var id = req.header("id");
    var session = req.header("session");

    // Check if the session exists and matches
    if (sessions[id] && sessions[id] === session) {
        delete sessions[id]; // Remove the session
        res.sendStatus(200); // Successfully logged out
        return;
    }

    res.sendStatus(401); // Unauthorized
});

app.get("/api/login", async (req, res) => {
    var username = req.header("username");
    var passwd = await sha256Hash(req.header("passwd"));

    for (let user in json_config.login) {
        if (json_config.login[user].username == username && json_config.login[user].passwdsha256 == passwd) {
            sessions[user] = {
                "session": randStr(32),
                "timestamp": Date.now()
            };
            res.send(user + "," + sessions[user].session); // List [0] is ID [1] is Session.
            return;
        }
    }
    
    res.sendStatus(401);
});

app.get("/api/get_data", (req, res) => {
    var user_session = req.header("session");
    var user_id = req.header("id");
    for (let session in sessions) {
        if (session == user_id && sessions[session].session == user_session) {
            sessions[session].timestamp = Date.now();

            if (Date.now() - last_online > 5000) {
                data.online = false;
                for (key in data.data) {
                    data.data[key] = 0;
                }
            }

            res.send(data);
            return;
        }
    }
    
    res.sendStatus(401);
});

app.post("/api/update_data", (req, res) => {
    var user_session = req.header("session");
    var user_id = req.header("id");
    for (let session in sessions) {
        if (session == user_id && sessions[session].session == user_session) {
            sessions[session].timestamp = Date.now();
            last_online = Date.now();
            
            data = {
                online: true,
                data: req.body
            };
            res.sendStatus(200);
            return;
        }
    }
    
    res.sendStatus(401);
});

app.listen(PORT, "0.0.0.0", function (err) {
    if (err) console.log(err);
    console.log("Server listening on PORT", PORT);
});

function check_sessions() {
    for (let session in sessions) {
        if (Date.now() - sessions[session].timestamp > 5000) {
            delete sessions[session];
        }
    }
}

setInterval(check_sessions, 5000);

const nms = new NodeMediaServer(config);
nms.run();

nms.on('preConnect', (id, args) => {
    console.log('[NodeEvent on preConnect]', `id=${id} args=${JSON.stringify(args)}`);
    var found = false;
    
    for (let session in sessions) {
        if (session == args.user_id && sessions[session].session == args.session) {
            found = true;
            break
        }
    }

    if (!found) {
        let session = nms.getSession(id);
        session.reject();
    }
});