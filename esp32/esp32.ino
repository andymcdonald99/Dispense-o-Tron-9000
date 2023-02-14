#include <WiFi.h>
#include <HTTPClient.h>


const char* ssid = "insert ssid";
const char* password = "insert password";

String serverName = "https://dispens-o-tron-default-rtdb.europe-west1.firebasedatabase.app";
String path = "/users.json";
String args = "?auth=Insert API key";
String serverPath = serverName + path + args;

int sleepTime = 20000;

HTTPClient http;
WiFiClientSecure client;


void setup(){
    Serial.begin(115200);
    delay(1000);

    
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

    client.setInsecure();
}

void loop(){
  delay(sleepTime);
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