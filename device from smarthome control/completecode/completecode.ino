#include <WiFi.h>
#include <WebSocketsClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

const int ledPin = 33;  // Assuming the LED is connected to GPIO pin 2
#define DHT_PIN 4
DHT dht(DHT_PIN, DHT22);
#define SOUND 2
// #define BUZZER_PIN 13 // Define the buzzer pin (you can change this to any available GPIO pin)
#define lockPin 32

const int pirPin = 25;  // Pin to which the PIR sensor is connected
int dust = 34;
const int finDeCourse = 5;

// connect with wifi and hhtp and websocket
const char* ssid = "RD-Team";
const char* password = "R&D-T3@m";
const char* serverAddress = "ws.elastic-watch.elastic-solutions.com";
const int serverPort = 14000;
WebSocketsClient webSocket;
unsigned long previousMillis = 0;
const long interval = 300000;  // 1 minute interval (in milliseconds)
const char* serverUrl = "https://backend.v2.elastic-watch.demo.elastic-solutions.com/api/devices/authenticate";
const char* serverData = "https://backend.v2.elastic-watch.demo.elastic-solutions.com/api/datacollection/event";
unsigned long doorOpenTime = 0;  // Variable to store the time when the door was opened
const char* Sound = "";
//DUST SENSOR
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;  //sampe 30s ;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

String jsonStr;
String tokenString;
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:

      Serial.println("[WebSocket] Disconnected");

      break;
    case WStype_CONNECTED:
      {
        Serial.println("[WebSocket] Connected");
        webSocket.sendTXT("{\"id\":\"3\",\"type\":\"device\",\"key\":\"key3\"}");
        break;
      }
    case WStype_TEXT:
      {
        String message = String((char*)(payload));
        Serial.println(message);



        if (message == "offlock") {
          digitalWrite(lockPin, LOW);
        }

        if (message == "onlock") {
          digitalWrite(lockPin, HIGH);
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
  //  pinMode(pirPin, INPUT);
  pinMode(dust, INPUT);
  pinMode(lockPin, OUTPUT);
  pinMode(SOUND, INPUT);
  dht.begin();
  digitalWrite(lockPin, HIGH);


  starttime = millis();  //get the current time;
  // Connect to Wi-Fi

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  delay(2000);
  // Initialize WebSocket
  webSocket.begin(serverAddress, serverPort, "/");
  webSocket.onEvent(webSocketEvent);

  doorOpenTime = millis();  // Set the initial time when the door was opened

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

  DynamicJsonDocument jsonDoc1(200);
  JsonObject root = jsonDoc1.to<JsonObject>();
  tokenString.trim();
  root["device_id"] = 3;
  root["token"] = String(tokenString);


  // Create the 'valeur' nested object and add all the key-value pairs within it
  JsonObject valeur = root.createNestedObject("valeur");
  valeur["temperature"] = 25;
  valeur["humidity"] = 60;
  valeur["msg"] = "";
  valeur["boolean"] = "";
  valeur["sound"] = "";
  valeur["lowpulseoccupancy"] = 0;
  valeur["ratio"] = 0;
  valeur["concentration"] = 0;
  valeur["status"] = "";



  String jsonStr1;
  serializeJson(root, jsonStr1);
  Serial.println(jsonStr1);
  Serial.println(tokenString);
  http.begin(serverData);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode1 = http.POST(jsonStr1);
  if (httpResponseCode1 > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode1);
    String response2 = http.getString();

    Serial.println(response2);
  } else {
    Serial.print("Error on sending POST request: ");
    Serial.println(httpResponseCode1);
  }

  http.end();
}


void loop() {

  webSocket.loop();
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int soundValue = digitalRead(SOUND);
  duration = pulseIn(dust, LOW);
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" °C");
    }
    DynamicJsonDocument jsonDoc1(200);
    JsonObject root = jsonDoc1.to<JsonObject>();
    tokenString.trim();
    root["device_id"] = 3;
    root["token"] = String(tokenString);
    JsonObject valeur = root.createNestedObject("valeur");
    valeur["temperature"] = temperature;
    valeur["humidity"] = humidity;
    int switchState = digitalRead(finDeCourse);

    if (switchState == LOW) {
      // If the switch is pressed
      valeur["status"] = "open";
      Serial.println("Door open");
    } else {
      Serial.println("Door closed");
      valeur["status"] = "closed";
    }


    if (soundValue == HIGH) {
      // Sound is detected
      Sound = "Sound is detected";

      delay(1000);  // Keep the buzzer on for 1 second
    } else {
      Sound = "Sound not detected";
    }
    valeur["sound"] = Sound;
    // int motionDetected = digitalRead(pirPin);

    //   if (motionDetected == HIGH) {
    //     Serial.println("Motion detected");
    //     valeur["msg"] = "Motion detected";
    //   } else {
    //     Serial.println("No motion detected");
    //     valeur["msg"] = "No motion detected";
    //   }



    lowpulseoccupancy = lowpulseoccupancy + duration;

    if ((millis() - starttime) > sampletime_ms)  //if the sampel time == 30s
    {
      ratio = lowpulseoccupancy / (sampletime_ms * 10.0);                              // Integer percentage 0=>100
      concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62;  // using spec sheet curve
      Serial.print(lowpulseoccupancy);
      Serial.print("lowpulseoccupancy:");
      Serial.print(ratio);
      Serial.print("ratio:");
      Serial.println(concentration);
      Serial.print("concentration:");
      lowpulseoccupancy = 0;
      starttime = millis();
    }

    valeur["lowpulseoccupancy"] = lowpulseoccupancy;
    valeur["ratio"] = ratio;
    valeur["concentration"] = concentration;
    String jsonStr1;
    serializeJson(root, jsonStr1);
    Serial.println(jsonStr1);
    Serial.println(tokenString);
    HTTPClient http;
    http.begin(serverData);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode1 = http.POST(jsonStr1);
    if (httpResponseCode1 > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode1);
      String response2 = http.getString();

      Serial.println(response2);
    } else {
      Serial.print("Error on sending POST request: ");
      Serial.println(httpResponseCode1);
    }

    http.end();
  }
  DynamicJsonDocument jsonDoc1(200);
  JsonObject root = jsonDoc1.to<JsonObject>();
  tokenString.trim();
  root["device_id"] = 3;
  root["token"] = String(tokenString);
  JsonObject valeur = root.createNestedObject("valeur");
  int switchState = digitalRead(finDeCourse);

  if (switchState == LOW) {
    // If the switch is pressed
    valeur["status"] = "open";
    Serial.println("Door open");

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" °C");
    }
    valeur["temperature"] = temperature;
    valeur["humidity"] = humidity;
    // noTone(BUZZER_PIN);
    if (soundValue == HIGH) {
      // Sound is detected
      Sound = "Sound is detected";

      delay(1000);  // Keep the buzzer on for 1 second
    } else {
      Sound = "Sound not detected";
    }
    valeur["sound"] = Sound;
    lowpulseoccupancy = lowpulseoccupancy + duration;

    if ((millis() - starttime) > sampletime_ms)  //if the sampel time == 30s
    {
      ratio = lowpulseoccupancy / (sampletime_ms * 10.0);                              // Integer percentage 0=>100
      concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62;  // using spec sheet curve
      Serial.print(lowpulseoccupancy);
      Serial.print("lowpulseoccupancy:");
      Serial.print(ratio);
      Serial.print("ratio:");
      Serial.println(concentration);
      Serial.print("concentration:");
      lowpulseoccupancy = 0;
      starttime = millis();
    }

    valeur["lowpulseoccupancy"] = lowpulseoccupancy;
    valeur["ratio"] = ratio;
    valeur["concentration"] = concentration;


    String jsonStr2;
    serializeJson(root, jsonStr2);
    Serial.println(jsonStr2);
    Serial.println(tokenString);
    HTTPClient http1;
    http1.begin(serverData);
    http1.addHeader("Content-Type", "application/json");
    int httpResponseCode2 = http1.POST(jsonStr2);
    if (httpResponseCode2 > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode2);
      String response2 = http1.getString();

      Serial.println(response2);
    } else {
      Serial.print("Error on sending POST request: ");
      Serial.println(httpResponseCode2);
    }

    http1.end();

    while (digitalRead(finDeCourse) == LOW) {
      // Wait for the switch to be released
      delay(10);
    }

    // Once released, print "Door open"
    valeur["status"] = "closed";
    Serial.println("Door closed");

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" °C");
    }
    valeur["temperature"] = temperature;
    valeur["humidity"] = humidity;
    // noTone(BUZZER_PIN);
    if (soundValue == HIGH) {
      // Sound is detected
      Sound = "Sound is detected";

      delay(1000);  // Keep the buzzer on for 1 second
    } else {
      Sound = "Sound not detected";
    }
    valeur["sound"] = Sound;
    lowpulseoccupancy = lowpulseoccupancy + duration;

    if ((millis() - starttime) > sampletime_ms)  //if the sampel time == 30s
    {
      ratio = lowpulseoccupancy / (sampletime_ms * 10.0);                              // Integer percentage 0=>100
      concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62;  // using spec sheet curve
      Serial.print(lowpulseoccupancy);
      Serial.print("lowpulseoccupancy:");
      Serial.print(ratio);
      Serial.print("ratio:");
      Serial.println(concentration);
      Serial.print("concentration:");
      lowpulseoccupancy = 0;
      starttime = millis();
    }

    valeur["lowpulseoccupancy"] = lowpulseoccupancy;
    valeur["ratio"] = ratio;
    valeur["concentration"] = concentration;

    // tone(BUZZER_PIN,1000);
    // delay(10000); // Add a 10-second delay

    // Store the time when the door was opened
    doorOpenTime = millis();

    String jsonStr1;
    serializeJson(root, jsonStr1);
    Serial.println(jsonStr1);
    Serial.println(tokenString);
    HTTPClient http;
    http.begin(serverData);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode1 = http.POST(jsonStr1);
    if (httpResponseCode1 > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode1);
      String response2 = http.getString();

      Serial.println(response2);
    } else {
      Serial.print("Error on sending POST request: ");
      Serial.println(httpResponseCode1);
    }

    http.end();
  }

  // // Check if the door has been open for more than 1 minute
  // if (doorOpenTime > 0 && (millis() - doorOpenTime) >= 60000) {
  //   // noTone(BUZZER_PIN); // Turn off the buzzer
  // }
}
