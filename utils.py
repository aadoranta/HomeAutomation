import requests
import urllib

"""
    Eventually I would like each device to send its characteristic function 
    (pre-programmed) and the server would be able to recognize them and assign them roles
"""

# Makes a get request to a url+uri
def get_request(url, uri):

    return requests.get(url + uri)

# Makes a post request to a url+uri
def post_request(url, uri, payload):

    encoded_payload = urllib.parse.urlencode(payload)
    return requests.post(url + uri, data=encoded_payload, headers={'Content-Type': 'application/x-www-form-urlencoded'})