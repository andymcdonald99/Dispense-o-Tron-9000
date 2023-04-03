import pandas as pd
import pyrebase
import requests

config = {
    "apiKey": "AIzaSyCYLCqr-XaeOfrklNBFfCpgC_MROqFzZyw",
    "authDomain": "dispens-o-tron.firebaseapp.com",
    "databaseURL": "https://dispens-o-tron-default-rtdb.europe-west1.firebasedatabase.app/",
    "projectId": "dispens-o-tron",
    "storageBucket": "dispens-o-tron.appspot.com",
    "messagingSenderId": "1048399231705",
    "appId": "1:1048399231705:web:846ee976b23e7db1089cc8",
}
firebase = pyrebase.initialize_app(config)
db = firebase.database()

auth = firebase.auth()

data = {
    'orderNo': "1",
    'user': 'andy',
    'item': 'item1',
    'price': '1'
}
#
# db.child("orders").child("order1").set(data)
email = "mcdonaa6@tcd.ie"
password = "vendme"
try:
    user = auth.create_user_with_email_and_password(email, password)
except requests.exceptions.HTTPError as err:
    print("Invalid Password")
print("bleh")
