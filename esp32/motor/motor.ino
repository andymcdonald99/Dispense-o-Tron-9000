#include <WiFi.h>
#include <Arduino.h>
#include <WebSocketsClient.h>
#include <Stepper.h>

WiFiClient wifiClient;
WebSocketsClient webSocket;

const int stepsPerRevolution = 2048;
Stepper stepperOne = Stepper(stepsPerRevolution, 25, 27, 26, 14);
Stepper stepperTwo = Stepper(stepsPerRevolution, 19, 5, 18, 17);

const char* ssid = "DispenseOTron";
const char* password = "90000000";

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
void moveMotor(int itemNumber);

void setup()
{
  Serial.begin(115200);
  stepperOne.setSpeed(15);
  stepperTwo.setSpeed(15);

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
    stepperOne.step(stepsPerRevolution * 2);
  }
  else{
    stepperTwo.step(-stepsPerRevolution * 2);
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
