#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal.h>

const char* ssid = "ssid";
const char* password = "password";

String serverName = "https://dispens-o-tron-default-rtdb.europe-west1.firebasedatabase.app";
String path = "/ready_to_dispense.json";
String args = "?auth=api key";
String serverPath = serverName + path + args;

LiquidCrystal lcd(19, 23, 18, 1, 3, 15);

int sleepTime = 5000;
//int motorStep = 8;

HTTPClient http;
WiFiClientSecure client;

void printLcd(String line_one, String line_two);
//void moveMotor(int steps);

void setup(){
    Serial.begin(9600);
//    pinMode(motorStep, OUTPUT);
//    digitalWrite(motorStep, LOW);
    delay(1000);

    lcd.begin(16, 2);
    printLcd("Dispense-O-Tron", "9000");
    
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
    if(payload != "null"){
      printLcd(payload, "");
//      moveMotor(50);
    }
    else{
      printLcd("Dispense-O-Tron", "9000");
    }
  }
  else {
    Serial.println("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();

}

void printLcd(String line_one, String line_two){
      line_one += "                            ";
      line_two += "                            ";
      lcd.setCursor(0, 0);
      lcd.print(line_one);
      lcd.setCursor(0, 1);
      lcd.print(line_two);
}
/*
void moveMotor(int steps){
  digitalWrite(motorStep, LOW);
  for(int i = 1; i < steps; i++){
    digitalWrite(motorStep, HIGH);
    delayMicroseconds(300);
    digitalWrite(motorStep, LOW);
    delayMicroseconds(300);
  }
}
*/

// // include the library code:
// #include <LiquidCrystal.h>
 
// // initialize the library with the numbers of the interface pins
// LiquidCrystal lcd(19, 23, 18, 17, 16, 15);
 
// void setup() {
//   // set up the LCD's number of columns and rows:
//   lcd.begin(16, 2);
//   // Print a message to the LCD.
//   lcd.print("circuitschools.");
// }
 
// void loop() {
//   // set the cursor to column 0, line 1
//   // (note: line 1 is the second row, since counting begins with 0):
//   lcd.setCursor(0, 1);
//   // print the number of seconds since reset:
//   lcd.print(millis() / 1000);
// }