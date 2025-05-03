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
        } catch (error) {
            console.error(error.message);
        }
    }

    get_data() {
        return this.data;
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

let data = new DATA("https://gokart.sheepland.xyz");

function update_statistics() {
    data.update_data();

    var json = data.get_data()

    // Update elements
    for (let key in elements) {
        // Check if json data isn't null.
        if (json == null && json[key] !== undefined) {
            elements[key].innerHTML = json[key];
        }
    }
}

//setInterval(update_statistics, 0.1)