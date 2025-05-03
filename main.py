import requests

data = {'api_dev_key': "hi",
        'api_option': 'paste',
        'api_paste_code': "e",
        'api_paste_format': 'python'}

# sending post request and saving response as response object
r = requests.post(url="http://localhost/api/update_data", data=data)