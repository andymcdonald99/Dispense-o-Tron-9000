#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>
#include <WebSocketsServer.h>
#include <ESP_Signer.h>
#include <Preferences.h>
#include <WebServer.h>

const int PAYMENT_PIN = 18;
//Variables stored in Non-volatile-storage
String ssid;
String password;
String soft_ap_ssid;
String soft_ap_password;
String projectId;
String clientEmail;
String privateKey;

SignerConfig signerConfig;

String serverName;
String path = "/orders.json";
String args;
String serverPath;

LiquidCrystal lcd(13, 12, 14, 27, 15, 16);

int sleepTime = 5000;
int wifiTimeout = 20000;
unsigned long lastTime = 0;
uint8_t clientID = 0;

HTTPClient http;
WiFiClientSecure client; 
WebSocketsServer webSocket = WebSocketsServer(81);   
bool slaveReady = false;
StaticJsonDocument<200> doc;

Preferences preferences;


void printLcd(String line_one, String line_two);
void moveMotor(int steps, int itemNumber);
bool awaitPayment(int num);
void dispense(int itemNumber);
void handleFirebaseResponse(String payload);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void removeOrder(String key);
void awaitConfig();
void awaitWifi();
void tokenStatusCallback(TokenInfo info);

WebServer httpServer(80);
void sendHTML();
void handlePost();
void handleGet();

void setup(){
  Serial.begin(115200);
  pinMode(PAYMENT_PIN, INPUT);
  preferences.begin("dispenseo", true);
  soft_ap_ssid = preferences.getString("ap_ssid", "DispenseOTron");
  soft_ap_password = preferences.getString("ap_password", "90000000");
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(soft_ap_ssid.c_str(), soft_ap_password.c_str());
  httpServer.on("/", HTTP_GET, handleGet);
  httpServer.on("/", HTTP_POST, handlePost);
  httpServer.onNotFound(handle_NotFound);
  httpServer.begin();

  webSocket.onEvent(webSocketEvent);
  webSocket.begin();
  
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  serverName = preferences.getString("project_server", "");
  projectId = preferences.getString("project_id", "");
  clientEmail = preferences.getString("client_email", "");
  privateKey = preferences.getString("private_key", "");
  preferences.end();

  Serial.print("ssid: ");
  Serial.println(ssid);
  Serial.print("password: ");
  Serial.println(password);
  Serial.print("project server: ");
  Serial.println(serverName);
  Serial.print("project id: ");
  Serial.println(projectId);
  Serial.print("client email: ");
  Serial.println(clientEmail);
  Serial.print("private key: ");
  Serial.println(privateKey);

  lcd.begin(16, 2);
  if(ssid == "" || password == "" || serverName == "" || projectId == "" || clientEmail == ""){
    awaitConfig();
  }

  privateKey = "-----BEGIN PRIVATE KEY-----\n" + privateKey + "\n-----END PRIVATE KEY-----\n";
  Serial.println(privateKey);

  printLcd("Dispense-O-Tron", "9000");
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.println("\nConnecting");

  int wifiStartTime = millis();
  while(WiFi.status() != WL_CONNECTED && millis() - wifiStartTime < wifiTimeout){
      Serial.print(".");
      delay(100);
  }
  if(WiFi.status() != WL_CONNECTED){
    WiFi.disconnect(true);
    awaitWifi();
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  client.setInsecure();
  Serial.print("ap ip: ");
  Serial.println(WiFi.softAPIP());

  signerConfig.service_account.data.client_email = clientEmail.c_str();
  signerConfig.service_account.data.project_id = projectId.c_str();
  signerConfig.service_account.data.private_key = privateKey.c_str();

  signerConfig.signer.expiredSeconds = 3600;

  signerConfig.signer.preRefreshSeconds = 60;

  signerConfig.signer.tokens.scope = "https://www.googleapis.com/auth/cloud-platform, https://www.googleapis.com/auth/userinfo.email";

  signerConfig.token_status_callback = tokenStatusCallback;
  Signer.begin(&signerConfig);
  Serial.println("Awaiting access token");
  while(!Signer.tokenReady()){
    Serial.print(".");
  }
  Serial.println("Got token succesfully");
}

void loop(){
  httpServer.handleClient();
  webSocket.loop();
  bool tokenReady = Signer.tokenReady();
  if((millis() - lastTime > sleepTime || millis() < lastTime) && tokenReady){
    lastTime = millis();
    serverPath = serverName + path + "?access_token=" + Signer.accessToken();
    Serial.println(serverPath);
    http.begin(client, serverPath);
      
    int httpResponseCode = http.GET();
    Serial.println("response code: ");
    Serial.println(httpResponseCode);

    if (400 > httpResponseCode && httpResponseCode > 0) {
      String payload = http.getString();
      http.end();
      handleFirebaseResponse(payload);
    }
    else {
      Serial.println("Error code: ");
      Serial.println(httpResponseCode);
      printLcd("Dispense-O-Tron", "9000");
      http.end();
    }
  }
}

void printLcd(String line_one, String line_two){
      line_one += "                            ";
      line_two += "                            ";
      Serial.print("printlcd ");
      Serial.print(line_one);
      Serial.println(line_two);
      lcd.setCursor(0, 0);
      lcd.print(line_one);
      lcd.setCursor(0, 1);
      lcd.print(line_two);
}

bool awaitPayment(int num){
  int numPayments = 0;
  Serial.println("awaiting payment");
  int paymentStartTime = millis();
  while(millis() - paymentStartTime < 30000 && numPayments < num){
    if(digitalRead(PAYMENT_PIN) == LOW){
      Serial.println("Recieved payment");
      numPayments++;
      String paid = "Paid: " + String(numPayments);
      String required = "Remaining: " + String(num - numPayments);
      printLcd(paid, required);
      paymentStartTime = millis();
      delay(1000);
    }
    if(numPayments == num){
      return true;
    }
  }
  return false;
}

void removeOrder(String key){
  String removePath = "/orders/" + key + ".json";
  String removeUri = serverName + removePath + "?access_token=" + Signer.accessToken();
  http.begin(client, removeUri);
    
  int httpResponseCode = http.sendRequest("DELETE");
  Serial.println("response code: ");
  Serial.println(httpResponseCode);

  if (400 > httpResponseCode && httpResponseCode > 0) {
    String payload = http.getString();
    http.end();
    handleFirebaseResponse(payload);
  }
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
        bool paid = awaitPayment(cost.toInt());
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
  if(type == WStype_TEXT){
    if(num == 0){
      slaveReady = true;
    }
  }
}

void delayAndHandleClient(int delayTime, int numTimes){
  for(int i = 0; i < numTimes; i++){
    httpServer.handleClient();
    webSocket.loop();
    delay(delayTime);
  }
}

void awaitConfig(){
  while(true){
    httpServer.handleClient();
    printLcd("config required", "");
    delayAndHandleClient(200, 15);
    httpServer.handleClient();
    printLcd(" Connect to:", soft_ap_ssid);
    delayAndHandleClient(200, 15);
    printLcd("Visit: ", WiFi.softAPIP().toString());
    delayAndHandleClient(200, 15);
  }
}

void awaitWifi(){
  while(true){
    httpServer.handleClient();
    printLcd("WiFi failed", "to connect");
    delayAndHandleClient(200, 15);
    httpServer.handleClient();
    printLcd("Connect to: ", soft_ap_ssid);
    delayAndHandleClient(200, 15);
    printLcd("Visit: ", WiFi.softAPIP().toString());
    delayAndHandleClient(200, 15);
  }
}

void handleGet() {
  httpServer.send(200, "text/html", SendHTML()); 
}

void handlePost(){
  String ssidArg = httpServer.arg("ssid");
  String passArg = httpServer.arg("password");
  String apSsidArg = httpServer.arg("ap_ssid");
  String apPassArg = httpServer.arg("ap_password");
  String projectServerArg = httpServer.arg("project_server");
  String projectIdArg = httpServer.arg("project_id");
  String clientEmailArg = httpServer.arg("client_email");
  String privateKeyArg = httpServer.arg("private_key");
  //important because html forms encode "\n" as "\\n" to escape newlines
  //we need the new lines...
  privateKeyArg.replace("\\n", "\n");
  String oldLocalSsidArg = httpServer.arg("old_local_ssid");
  String oldLocalPassArg = httpServer.arg("old_local_pass");
  if(ssidArg == "" || passArg == "" || apSsidArg == "" || apPassArg == ""   || projectServerArg == "" || projectIdArg == "" || clientEmailArg == "" || privateKeyArg == ""){
    httpServer.send(400, "text/plain", "missing required argument");
  }
  else if(oldLocalSsidArg != soft_ap_ssid || oldLocalPassArg != soft_ap_password){
    httpServer.send(401, "text/plain", "old ssid and pass must match currently on board");
  }
  else{
    httpServer.send(200, "text/plain", "success");
    preferences.begin("dispenseo", false);
    preferences.putString("ssid", ssidArg);
    preferences.putString("password", passArg);
    preferences.putString("ap_ssid", apSsidArg);
    preferences.putString("ap_password", apPassArg);
    preferences.putString("project_server", projectServerArg);
    preferences.putString("project_id", projectIdArg);
    preferences.putString("client_email", clientEmailArg);
    preferences.putString("private_key", privateKeyArg);
    preferences.end();
    Serial.println("Restarting in 5 seconds");
    delay(5000);
    ESP.restart();
  }
}

void handle_NotFound(){
  httpServer.send(404, "text/plain", "Not found");
}

String SendHTML(){
  return "<!DOCTYPE html>"
  "<html><head>"
  "<title>ESP Wi-Fi Manager</title>"
  "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
  "</head>"
  "<body>"
  "<div class=\"topnav\">"
  "<h1>Dispense-O-Tron 9000 Configurator</h1>"
  "</div>"
  "<div class=\"content\">"
  "<div class=\"card-grid\">"
  "<div class=\"card\">"
  "<form action=\"/\" method=\"POST\" enctype=\"application/x-www-form-urlencoded\">"
  "<p>"
  "<label for=\"ssid\">SSID</label>"
  "<input type=\"text\" id =\"ssid\" name=\"ssid\"><br>"
  "<label for=\"password\">Password</label>"
  "<input type=\"text\" id =\"password\" name=\"password\"><br>"
  "<label for=\"ap_ssid\">AP SSID</label>"
  "<input type=\"text\" id =\"ap_ssid\" name=\"ap_ssid\"><br>"
  "<label for=\"ap_password\">AP Password</label>"
  "<input type=\"text\" id =\"ap_password\" name=\"ap_password\"><br>"
  "<label for=\"project_server\">Firebase Project Server</label>"
  "<input type=\"text\" id =\"project_server\" name=\"project_server\"><br>"
  "<label for=\"project_id\">Firebase Project ID</label>"
  "<input type=\"text\" id =\"project_id\" name=\"project_id\"><br>"
  "<label for=\"client_email\">Service Account Client Email</label>"
  "<input type=\"text\" id =\"client_email\" name=\"client_email\"><br>"
  "<label for=\"private_key\">Service Account Private Key (Do not include begin and end private key)</label>"
  "<input type=\"text\" id =\"private_key\" name=\"private_key\"><br>"
  "<label for=\"old_local_ssid\">Existing Local SSID</label>"
  "<input type=\"text\" id =\"old_local_ssid\" name=\"old_local_ssid\"><br>"
  "<label for=\"old_local_pass\">Existing Local Password</label>"
  "<input type=\"text\" id =\"old_local_pass\" name=\"old_local_pass\"><br>"
  "<input type =\"submit\" value =\"Submit\">"
  "</p></form></div></div></body></html>";
}

void tokenStatusCallback(TokenInfo info)
{
    if (info.status == esp_signer_token_status_error)
    {
        Serial.printf("Token info: type = %s, status = %s\n", Signer.getTokenType(info).c_str(), Signer.getTokenStatus(info).c_str());
        Serial.printf("Token error: %s\n", Signer.getTokenError(info).c_str());
    }
    else
    {
        Serial.printf("Token info: type = %s, status = %s\n", Signer.getTokenType(info).c_str(), Signer.getTokenStatus(info).c_str());
        if (info.status == esp_signer_token_status_ready)
            Serial.printf("Token: %s\n", Signer.accessToken().c_str());
    }
}