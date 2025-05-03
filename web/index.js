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

// Encode and parse data.
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

const PORT = process.env.PORT || 8001;

let data = {
    online: false, 
    data: {}
};

// Nginx proxy confs make us need to call /api/get_data
app.get("/get_data", (req, res) => {
    res.send(data);
});

// Nginx proxy confs make us need to call /api/update_data
app.post("/update_data", (req, res) => {
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