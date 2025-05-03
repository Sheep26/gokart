// Nginx proxy confs allow us to run 2 ports under one domain.
const api_url = "https://gokart.sheepland.xyz"

class DATA {
    constructor(api_url = "") {
        this.data = null;
        this.api_url = api_url;
    }

    async update_data() {
        // Implementation for updating data

        try {
            // Request data from api
            // Nginx proxy confs make us need to call /api/get_data instead of /get_data
            const response = await fetch(this.api_url + "/api/get_data");

            if (!response.ok) {
              throw new Error(`Response status: ${response.status}`);
            }
        
            const json = await response.json();

            // Set data variable
            this.data = json;

            return json;
        } catch (error) {
            console.error(error.message);
        }
    }

    get_data() {
        return this.data.data;
    }

    get_online() {
        return this.data.online;
    }
}

let elements = {
    speed: document.getElementById("speed-data"),
    speed_avg: document.getElementById("speed-data-avg"),
    speed_max: document.getElementById("speed-data-max"),
    rpm: document.getElementById("rpm-data"),
    rpm_avg: document.getElementById("rpm-data-avg"),
    rpm_max: document.getElementById("rpm-data-max"),
    power: document.getElementById("power-data"),
    power_avg: document.getElementById("power-data-avg"),
    power_max: document.getElementById("power-data-max"),
    throttle: document.getElementById("throttle-data"),
    throttle_avg: document.getElementById("throttle-data-avg"),
    throttle_max: document.getElementById("throttle-data-max"),
}

let data = new DATA(api_url);
let flvrunning = false;

function update_statistics() {
    data.update_data().then(json => {
        // Check if online
        if (!json.online) return;
        // Update elements
        for (let key in elements) {
            // Check if json data isn't null.
            if (json == null && json.data[key] !== undefined) {
                elements[key].innerHTML = json.data[key];
            }
        }
    });
}

setInterval(update_statistics, 100)
check_online_interval = setInterval(check_online, 100)

function check_online() {
    if (data.data != null && data.get_online() && !flvrunning) {
        clearInterval(check_online_interval);
        create_flv();
    }
}

function create_flv() {
    if (flvjs.isSupported()) {
        flvrunning = true;
        var videoElement = document.getElementById('videoElement');
        var flvPlayer = flvjs.createPlayer({
            type: 'flv',
            url: api_url + '/live/stream.flv'
        });
        flvPlayer.attachMediaElement(videoElement);
        flvPlayer.load();
        flvPlayer.play();
    }
}