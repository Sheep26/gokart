const api_url = "https://gokart.sheepland.xyz"

class Connection {
    login = ""

    constructor(api_url = "", login) {
        this.data = null;
        this.api_url = api_url;
        this.login = login;
    }

    async update_data() {
        // Implementation for updating data

        try {
            // Request data from api
            const response = await fetch(this.api_url + "/api/get_data", {
                method: "GET",
                headers: {
                    "X-API-KEY-B64": this.login,
                }
            });

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

// [element, unit_of_mesurement(str)]
// We store the elements we are using to display data here so we don't have to call for them later.
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

let connection = null;
let flvPlayer = null;

function login(login_details) {
    // Login here, return api key, api key, encode api key base 64.
    connection = new Connection(api_url, "" /* Encoded base 64 key*/);
}

function update_statistics() {
    connection.update_data();
    // Check if online
    if (connection.data != null && !connection.get_online()) {
        // Set element data to 0.
        for (let key in elements) {
            elements[key][0].innerHTML = "0" + elements[key][1]
        }
        return;
    }

    // Update elements
    for (let key in elements) {
        // Check if json data isn't null.
        if (connection.data != null && connection.get_data()[key] != undefined) {
            elements[key][0].innerHTML = connection.get_data()[key] + elements[key][1];
        }
    }
}

setInterval(update_statistics, 100);
setInterval(check_online, 100);

function check_online() {
    if (connection.data != null == null) return;
    if (connection.get_online()) {
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