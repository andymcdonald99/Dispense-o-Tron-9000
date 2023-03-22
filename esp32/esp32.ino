#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>

const char* ssid = "ssid";
const char* password = "password";

String serverName = "https://dispens-o-tron-default-rtdb.europe-west1.firebasedatabase.app";
String path = "/ready_to_dispense.json";
String args = "?auth=<api key>";
String serverPath = serverName + path + args;

LiquidCrystal lcd(13, 12, 14, 27, 15, 16);

int sleepTime = 5000;
const int DIR = 4;
const int STEPONE = 33;
const int STEPTWO = 32;
const int steps_per_rev = 200;
const int PAY_TIMEOUT = 20000;

HTTPClient http;
WiFiClientSecure client;    
StaticJsonDocument<200> doc;


void printLcd(String line_one, String line_two);
void moveMotor(int steps, int itemNumber);
bool awaitPayment(int num, String colour);

void setup(){
    Serial.begin(115200);
    pinMode(DIR, OUTPUT);
    digitalWrite(DIR, LOW);
    pinMode(STEPONE, OUTPUT);
    pinMode(STEPTWO, OUTPUT);
    digitalWrite(STEPONE, LOW);
    digitalWrite(STEPTWO, LOW);
    delay(1000);


    lcd.begin(16, 2);
    printLcd("Dispense-O-Tron", "9000");
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

  if (400 > httpResponseCode > 0) {
    Serial.println("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
    if(payload != "null"){
      deserializeJson(doc, payload);
      String name = doc["name"];
      String cost = doc["cost"];
      String itemNoStr = doc["item_no"];
      int itemNo = itemNoStr.toInt();
      printLcd("Welcome " + name, "Pay: " + cost);
      int payStartTime = millis();
      delay(2000);
      bool paid = awaitPayment(cost.toInt(), "blue");
      if(paid){
        printLcd("Dispensing...", "Item " + itemNoStr);
        moveMotor(1000, itemNo);
      }
      else{
        printLcd("Payment not", "recieved");
        delay(2000);
        printLcd("Place order", "again");
        delay(2000);
      }
      removeOrder();
      printLcd("Dispense-O-Tron", "9000");
    }
    else{
      printLcd("Dispense-O-Tron", "9000");
    }
  }
  else {
    Serial.println("Error code: ");
    Serial.println(httpResponseCode);
    printLcd("Dispense-O-Tron", "9000");
  }
  http.end();

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

void moveMotor(int steps, int itemNumber){
  Serial.println("dispensing " + itemNumber);
  int stepPin;
  if(itemNumber == 1){
    stepPin = STEPONE;
  }
  else{
    stepPin = STEPTWO;
  }
  digitalWrite(DIR, LOW);
  for(int i = 1; i < steps; i++){
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(2000);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(2000);
  }
}

bool awaitPayment(int num, String colour){
  return true;
}

void removeOrder(){
  http.begin(client, serverPath);
}