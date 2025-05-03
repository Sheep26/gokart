import requests

data = {
        "gaming": "gamer",
}

# sending post request and saving response as response object
r = requests.post(url="http://localhost/api/update_data", data=data)