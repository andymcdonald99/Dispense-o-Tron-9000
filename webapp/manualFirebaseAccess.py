import requests
import json

app_id = "0e0bd04d"
app_key = "239446a4c5bd6b494b0bc004c551ae68"
language = "en-gb"
word_id = "father"
url = "https://od-api.oxforddictionaries.com:443/api/v2/entries/" + language + "/" + word_id.lower()
r = requests.get(url, headers={"app_id": app_id, "app_key": app_key})
json_object = json.loads(r.content)
pretty_json = json.dumps(json_object, indent=3)
test = json_object['results'][0]['lexicalEntries']
print(pretty_json)
