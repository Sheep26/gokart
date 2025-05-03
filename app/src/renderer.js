class DATA {
    constructor(api_url = "") {
        this.data = null;
        this.api_url = api_url;
    }

    async update_data() {
        // Implementation for updating data

        try {
            // Request data from api
            const response = await fetch(this.api_url + "/api/exported_data");

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

    get_speed() {
        return this.data.speed
    }

    get_rpm() {
        return this.data.rpm
    }

    get_power() {
        return this.data.power
    }

    get_throttle() {
        this.data.throttle
    }
}

var data = new DATA("https://gokart.sheepland.xyz");

