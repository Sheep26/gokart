const express = require("express");
const NodeMediaServer = require('node-media-server');
var fs = require('fs');
var json_config;

const app = express();

const config = {
    rtmp: {
        port: 1935,
        chunk_size: 60000,
        gop_cache: true,
        ping: 30,
        ping_timeout: 60
    },
    http: {
        port: 8002,
        allow_origin: '*',
    },
};

let last_online = Date.now();

var sessions = {}

fs.readFile('config.json', 'utf8', function (err, data) {
    if (err) throw err;
    json_config = JSON.parse(data);
});

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

app.get("/api/login", (req, res) => {
    var username = req.header("USERNAME");
    var passwd = "";
    sha256Hash(req.header("PASSWD")).then(data => passwd = data);

    for (user in json_config.login) {
        if (json_config.login[user].username == username && json_config.login[user].passwdsha256 == passwd) {
            sessions[user] = randStr(32);
            break;
        }
    }
});

app.get("/api/get_data", (req, res) => {
    var user_session = req.header("SESSION");
    var user_id = req.header("ID");
    for (let session in sessions) {
        if (session == user_id && sessions[session] == user_session) {
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
    var user_session = req.header("SESSION");
    var user_id = req.header("ID");
    for (let session in sessions) {
        if (session == user_id && sessions[session] == user_session) {
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

const nms = new NodeMediaServer(config);
nms.run();