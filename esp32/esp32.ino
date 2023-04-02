#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>
#include <WebSocketsServer.h>

const char* ssid = "ssid";
const char* password = "pass";
const char *soft_ap_ssid = "DispenseOTron";
const char *soft_ap_password = "90000000";

String serverName = "https://dispens-o-tron-default-rtdb.europe-west1.firebasedatabase.app";
String path = "/orders.json";
String args = "?auth=api key";
String serverPath = serverName + path + args;

LiquidCrystal lcd(13, 12, 14, 27, 15, 16);

int sleepTime = 5000;
unsigned long lastTime = 0;
uint8_t clientID = 0;

HTTPClient http;
WiFiClientSecure client; 
WebSocketsServer webSocket = WebSocketsServer(81);   
StaticJsonDocument<200> doc;


void printLcd(String line_one, String line_two);
void moveMotor(int steps, int itemNumber);
bool awaitPayment(int num, String colour);
void dispense(int itemNumber);
void handleFirebaseResponse(String payload);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void removeOrder(String key);

void setup(){
  Serial.begin(115200);

  lcd.begin(16, 2);
  printLcd("Dispense-O-Tron", "9000");
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(soft_ap_ssid, soft_ap_password);
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
  Serial.print("ap ip: ");
  Serial.println(WiFi.softAPIP());
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop(){
  webSocket.loop();
  if(millis() - lastTime > sleepTime || millis() < lastTime){
    lastTime = millis();
    http.begin(client, serverPath);
      
    int httpResponseCode = http.GET();
    Serial.println("response code: ");
    Serial.println(httpResponseCode);

    if (400 > httpResponseCode && httpResponseCode > 0) {
      String payload = http.getString();
      handleFirebaseResponse(payload);
    }
    else {
      Serial.println("Error code: ");
      Serial.println(httpResponseCode);
      printLcd("Dispense-O-Tron", "9000");
    }
    http.end();
  }

}

void printLcd(String line_one, String line_two){
      line_one += "                            ";
      line_two += "                            ";
      Serial.println("printlcd " + line_one + line_two);
      lcd.setCursor(0, 0);
      lcd.print(line_one);
      lcd.setCursor(0, 1);
      lcd.print(line_two);
}

bool awaitPayment(int num, String colour){
  delay(2000);
  return true;
}

void removeOrder(String key){
  return;
}

void dispense(String itemNumber){
  if (webSocket.connectedClients() > 0) {
    Serial.print("Sending message");
    Serial.println(itemNumber);
    webSocket.sendTXT(clientID, itemNumber.c_str());
  }
}


void handleFirebaseResponse(String payload){
      if(payload != "null"){
        deserializeJson(doc, payload);
        JsonObject docAsJson = doc.as<JsonObject>();
        String key = docAsJson.begin()->key().c_str();
        String name = doc[key]["user"];
        String cost = doc[key]["price"];
        String itemNoStr = doc[key]["item"];
        printLcd("Welcome " + name, "Pay: " + cost);
        bool paid = awaitPayment(cost.toInt(), "blue");
        if(paid){
          printLcd("Dispensing...", "Item " + itemNoStr);
          dispense(itemNoStr);
          delay(2000);
        }
        else{
          printLcd("Payment not", "recieved");
          delay(2000);
          printLcd("Place order", "again");
          delay(2000);
        }
        removeOrder(key);
        printLcd("Dispense-O-Tron", "9000");
      }
      else{
        printLcd("Dispense-O-Tron", "9000");
      }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  Serial.print("Got message type: ");
  Serial.println(type);
  Serial.print("With content: ");
  Serial.println(String((char *)payload));
  Serial.print("From client id: ");
  Serial.println(num);
  if(type == WStype_CONNECTED){ 
    Serial.println("client connected");
    clientID = num;
  }
  else if(type == WStype_DISCONNECTED){
    Serial.println("client disconnected");
  }
}