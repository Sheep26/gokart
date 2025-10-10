import requests
from time import sleep
from random import randint

data = {
        "num": 12,
        "speed": 12,
        "satellites": 8,
        "batteryVolt": 46.43,
        "batteryPercent": 29.00
}

session = requests.get("https://gokart.sheepland.xyz/api/login", headers={"username": "rpi", "passwd": "admin"})
print(session.status_code)

# sending post request and saving response as response object
while True:
        data["speed"] = randint(0, 50)
        headers={"id": session.text.split(",")[0], "session": session.text.split(",")[1]}
        print(headers)
        r = requests.post(url="https://gokart.sheepland.xyz/api/update_data", data=data, headers=headers)
        print(r.status_code)
        sleep(1)