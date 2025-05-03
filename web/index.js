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
        port: 8000,
        allow_origin: '*'
    },
};

// Encode and parse data.
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

const PORT = process.env.PORT || 80;

let data = {};

app.get("/api/get_data", (req, res) => {
    res.send(data);
});

app.post("/api/update_data", (req, res) => {
    data = req.body;
    res.sendStatus(200);
});

app.listen(PORT, "0.0.0.0", function (err) {
    if (err) console.log(err);
    console.log("Server listening on PORT", PORT);
});

const nms = new NodeMediaServer(config);
nms.run();

console.log('Node Media Server running on RTMP port 1935 and HTTP port 8000');