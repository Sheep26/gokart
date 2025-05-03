const express = require("express");

const app = express();

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