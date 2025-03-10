#include <WiFi.h>
#include <WebSocketsClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
const char* ssid = "ssid";
const char* password = "password";
const char* serverAddress = "serverAddress";
const int serverPort = 4000;

WebSocketsClient webSocket;
const int ledPin = 2; // Assuming the LED is connected to GPIO pin 2
const int lockPin = 4;
const int pompePin = 16;
const int moteurPin = 17;
const int finDeCourse = 5;
const char* serverUrl = "serverUrl";
const char* serverData = "serverData";

String jsonStr; 
String tokenString;
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WebSocket] Disconnected");
      break;
    case WStype_CONNECTED: {
      Serial.println("[WebSocket] Connected");
      webSocket.sendTXT("{\"device_id\":\"3\",\"type\":\"device\",\"key\":\"key3\"}");
      break;
    }
    case WStype_TEXT: {
    String message = String((char*)( payload));
  Serial.println(message);
 
  if(message == "off") {
    digitalWrite(ledPin, LOW);
  }
 
  if(message == "on") {
    digitalWrite(ledPin, HIGH);
  }
  
   
  if(message == "offLock") {
    digitalWrite(lockPin, LOW);
  }

  if(message == "onLock") {
    digitalWrite(lockPin, HIGH);
  }
  if(message=="offpompe"){
  digitalWrite(pompePin, LOW);
  }
    if(message=="onpompe"){
  digitalWrite(pompePin, HIGH);
  }
  if(message=="offmoteur"){
  digitalWrite(moteurPin, LOW);
  }
    if(message=="onmoteur"){
  digitalWrite(moteurPin, HIGH);
  }
  break;
}
    case WStype_BIN:
    case WStype_ERROR:
      Serial.println("[WebSocket] Error");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(lockPin, OUTPUT);
  pinMode(pompePin, OUTPUT);
  pinMode(moteurPin, OUTPUT);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize WebSocket
  webSocket.begin(serverAddress, serverPort, "/");
  webSocket.onEvent(webSocketEvent);
  

    DynamicJsonDocument jsonDoc(128);
  JsonObject data = jsonDoc.to<JsonObject>();

  data["device_id"] = 3;
  data["key"] = "key3";
  serializeJson(data, jsonStr);
   // Send the POST request
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(jsonStr);
   if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String response = http.getString();
    DynamicJsonDocument doc(256);
  
    // Deserialize the JSON data
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
      Serial.println("Failed to parse JSON");
    }
    // Extract the token
    const char* token = doc["token"];
    
    // Save the token in a string variable
    tokenString = String(token);
    // Print the extracted token
    Serial.print("Token: ");
    Serial.println(tokenString);
    Serial.println(response);
  } else {
    Serial.print("Error on sending POST request: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  delay(3000);

  DynamicJsonDocument jsonDoc1(128);
  JsonObject root = jsonDoc1.to<JsonObject>();
  tokenString.trim();
  root["device_id"] = 4;
  root["token"] = String(tokenString);
}

void loop() {
  webSocket.loop();
  DynamicJsonDocument jsonDoc1(300);
  JsonObject root = jsonDoc1.to<JsonObject>();
  tokenString.trim();
  root["device_id"] = 3;
  root["token"] = String(tokenString);

  int switchState = digitalRead(finDeCourse);
    if (switchState == LOW) {
    // If the switch is pressed
    webSocket.sendTXT("{\"device_id\":\"3\",\"type\":\"device\",\"key\":\"key3\",\"door_status\":\"closed\"}");
    Serial.println("Door closed");
    while (digitalRead(finDeCourse) == LOW) {
      // Wait for the switch to be released
      delay(10);
    }
    // Once released, print "Door open"
    Serial.println("Door open");
     webSocket.sendTXT("{\"device_id\":\"3\",\"type\":\"device\",\"key\":\"key3\",\"door_status\":\"open\"}");

  }
}
