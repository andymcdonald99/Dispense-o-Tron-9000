#include <WiFi.h>
#include <HTTPClient.h>


const char* ssid = "insert ssid";
const char* password = "insert password";

void setup(){
    Serial.begin(115200);
    delay(1000);

    String serverName = "https://dispens-o-tron-default-rtdb.europe-west1.firebasedatabase.app";
    String path = "/users.json";
    String args = "?auth=insert api key";

    Serial.println(ssid);
    Serial.println(password);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");

    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());

    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();

    String serverPath = serverName + path + args;
    
    http.begin(client, serverPath);
    
    int httpResponseCode = http.GET();
    Serial.println("response code");
    Serial.println(httpResponseCode);

    if (httpResponseCode > 0) {
      Serial.println("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    }
    else {
      Serial.println("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();

}

void loop(){}