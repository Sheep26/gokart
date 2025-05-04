import requests

data = {
        "speed": 20,
        "speed_avg": 20,
        "speed_max": 20,
        "rpm": 1000,
        "rpm_avg": 1000,
        "rpm_max": 1000,
        "power": 1000,
        "power_avg": 1000,
        "power_max": 1000,
        "throttle": 100,
        "throttle_avg": 100,
        "throttle_max": 100
}

# sending post request and saving response as response object
r = requests.post(url="https://gokart.sheepland.xyz/api/update_data", data=data)