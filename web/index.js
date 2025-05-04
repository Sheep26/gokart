const express = require("express");
const NodeMediaServer = require('node-media-server');

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

// Encode and parse data.
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

const PORT = process.env.PORT || 8001;

let data = {
    online: false, 
    data: {}
};

app.get("/api/get_data", (req, res) => {
    if (Date.now() - last_online > 5000) {
        data.online = false;
    }
    res.send(data);
});

app.post("/api/update_data", (req, res) => {
    last_online = Date.now();
    data = {
        online: true,
        data: req.body
    };
    res.sendStatus(200);
});

app.listen(PORT, "0.0.0.0", function (err) {
    if (err) console.log(err);
    console.log("Server listening on PORT", PORT);
});

const nms = new NodeMediaServer(config);
nms.run();