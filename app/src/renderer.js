class DATA {
    constructor(speed = 0, rpm = 0, power = 0, throttle = 0, api_url = "") {
        this.speed = speed;
        this.rpm = rpm;
        this.power = power;
        this.throttle = throttle;
    }

    async update_data() {
        // Implementation for updating data

        try {
            const response = await fetch(url);
            
            if (!response.ok) {
              throw new Error(`Response status: ${response.status}`);
            }
        
            const json = await response.json();
        } catch (error) {
            console.error(error.message);
        }
    }

    get_speed() {
        return this.speed
    }

    get_rpm() {
        return this.rpm
    }

    get_power() {
        return this.power
    }

    get_throttle() {
        this.throttle
    }
}

var data = new DATA();

