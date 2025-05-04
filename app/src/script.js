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
    speed: [document.getElementById("speed-data"), "km/h"],
    speed_avg: [document.getElementById("speed-data-avg"), "km/h"],
    speed_max: [document.getElementById("speed-data-max"), "km/h"],
    rpm: [document.getElementById("rpm-data"), "rpm"],
    rpm_avg: [document.getElementById("rpm-data-avg"), "rpm"],
    rpm_max: [document.getElementById("rpm-data-max"), "rpm"],
    power: [document.getElementById("power-data"), "W"],
    power_avg: [document.getElementById("power-data-avg"), "W"],
    power_max: [document.getElementById("power-data-max"), "W"],
    throttle: [document.getElementById("throttle-data"), "%"],
    throttle_avg: [document.getElementById("throttle-data-avg"), "%"],
    throttle_max: [document.getElementById("throttle-data-max"), "%"],
}

let data = new DATA(api_url);

let flvPlayer = null;

function update_statistics() {
    data.update_data();
    // Check if online
    if (data.data != null && !data.get_online()) return;

    // Update elements
    for (let key in elements) {
        // Check if json data isn't null.
        if (data.data != null && data.get_data()[key] != undefined) {
            elements[key][0].innerHTML = data.get_data()[key] + elements[key][1];
        }
    }
}

setInterval(update_statistics, 100);
setInterval(check_online, 100);

function check_online() {
    if (data.data != null == null) return;
    if (data.get_online()) {
        if (flvPlayer == null) {
            create_flv();
        }
    } else if (flvPlayer != null) {
        flvPlayer.destroy();
        flvPlayer = null;
    }
}

function create_flv() {
    if (flvjs.isSupported()) {
        var videoElement = document.getElementById('videoElement');
        flvPlayer = flvjs.createPlayer({
            type: 'flv',
            url: api_url + '/live/stream.flv'
        });
        flvPlayer.attachMediaElement(videoElement);
        flvPlayer.load();
        flvPlayer.play();
    }
}