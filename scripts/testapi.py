import requests
from time import sleep

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

session = requests.get("http://localhost:8001/api/login", headers={"username": "admin", "passwd": "admin"})
print(session.status_code)

# sending post request and saving response as response object
while True:
        headers={"id": session.text.split(",")[0], "session": session.text.split(",")[1]}
        print(headers)
        r = requests.post(url="http://localhost:8001/api/update_data", data=data, headers=headers)
        print(r.status_code)
        sleep(0.1)