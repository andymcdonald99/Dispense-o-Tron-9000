#include <WiFi.h>
#include <Arduino.h>
#include <WebSocketsClient.h>

WiFiClient wifiClient;
WebSocketsClient webSocket;

const int STEP_ONE = 21;
const int STEP_TWO = 22;
const int DIR = 16;
const int steps_per_rev = 4000;

const char* ssid = "DispenseOTron";
const char* password = "90000000";

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
void moveMotor(int itemNumber);

void setup()
{
  Serial.begin(115200);
  pinMode(STEP_ONE, OUTPUT);
  pinMode(STEP_TWO, OUTPUT);
  pinMode(DIR, OUTPUT);
  digitalWrite(DIR, LOW);

  WiFi.mode(WIFI_STA);

	WiFi.begin(ssid, password);
	
  Serial.println("Connecting to wifi");
	while(WiFi.status() != WL_CONNECTED) {
		delay(100);
    Serial.print(".");
	}
  Serial.println("");

	webSocket.begin(WiFi.gatewayIP().toString(), 81, "/");
	webSocket.onEvent(webSocketEvent);
	webSocket.setReconnectInterval(5000);
}

void loop()
{
  webSocket.loop();
}

void moveMotor(int itemNumber){
  int stepPin;
  if(itemNumber == 1){
    stepPin = STEP_ONE;
  }
  else{
    stepPin = STEP_TWO;
  }

  for(int i = 0; i<steps_per_rev; i++)
  {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(300);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(300);
  }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
	switch(type) {
		case WStype_DISCONNECTED:
			Serial.printf("Disconnected!\n");
			break;
		case WStype_CONNECTED:
			Serial.printf("Connected");
			break;
		case WStype_TEXT:
      String payloadString = String((char*)payload);
			Serial.print("got text: ");
      Serial.println(payloadString);
      if(payloadString == "1"){
        moveMotor(1);
      }
      else if(payloadString == "2"){
        moveMotor(2);
      }
      else{
        Serial.println("Invalid item recieved");
      }
			break;
	}
}
