#include <M5Core2.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

bool UWBEnabled = false;

// WIFI Settings
const char* ssid = "dafarbel";
const char* password = "f11021974";
const char* timeServerUrl = "http://192.168.178.43:3000/current-time";
const char* distanceServerUrl = "http://192.168.178.43:3000/distance";

String DATA  = " ";  // Used to store distance data . 
int UWB_MODE = 0;    // Used to set UWB mode . 
int UWB_T_NUMBER      = 0;  // Current number of UWB Tags detected
int TAG_ID = 1;
String commands[4] = {"AT+anchor_tag=0," + String(TAG_ID) + "\r\n", "AT+interval=5\r\n", "AT+switchdis=0\r\n", "AT+RST\r\n"};
String turnOffCommand = "AT+switchdis=0\r\n";
String turnOnCommand = "AT+switchdis=1\r\n";

int numCommands = 4;

const int startOffset = 2;  // Starting index for anchorID in the data string
const int anchorIdLength = 1;  // Length of the anchor ID
const int distanceStartOffset = 4;  // Starting index for distance in the data string
const int distanceLength = 4;  // Length of the distance string
const int dataPacketSize = 11;  // Size of one complete data packet
const int yBase = 50;  // Base y-coordinate for drawing strings
const int yIncrement = 40;  // Y increment per data entry

// Send relative positions to server
void sendDataToServer(String tagId, String dA, String dB, String dC) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(distanceServerUrl);
    http.addHeader("Content-Type", "application/json");

    // Create JSON object
    StaticJsonDocument<200> doc;
    doc["tagId"] = tagId;
    doc["dA"] = dA;
    doc["dB"] = dB;
    doc["dC"] = dC;

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    Serial.println("Sending payload: " + jsonPayload); // Debug: print the payload

    // Send HTTP POST request
    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("HTTP Response code: " + String(httpResponseCode));
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
    }

    // Free resources
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

// Get position
void getPosition() {
  if (UWB_T_NUMBER > 0 && UWB_T_NUMBER < 5) {
    String dA = "0";
    String dB = "0";
    String dC = "0";

    for (int i = 0; i < UWB_T_NUMBER; i++) {
      int anchorIdStart = startOffset + i * dataPacketSize;
      int distanceStart = distanceStartOffset + i * dataPacketSize;
      String anchorID = DATA.substring(anchorIdStart, anchorIdStart + anchorIdLength);
      String distance = DATA.substring(distanceStart, distanceStart + distanceLength);
      Serial.println("AnchorID: " + anchorID + " | Distance: " + distance + "m");
      if(anchorID == "1"){
        dA = distance;
      }
      if(anchorID == "2"){
        dB = distance;
      }
      if(anchorID == "3"){
        dC = distance;
      }
    }
    sendDataToServer(String(TAG_ID), dA, dB, dC);
    Serial.println("----------------------------------------------------------------");
  }
}

// Read UART data 
void readPositionData() {
  if (Serial2.available()) {
      delay(20);
      UWB_T_NUMBER = (Serial2.available() / 11);  // Count the number of Base stations, each base station sends a packet of 11 bytes to a tag
      delay(20);
      DATA = Serial2.readString();
      delay(2);
  }
}

void sendCommandWithDelay(const String& command, unsigned long delayMs) {
    Serial2.print(command);
    delay(delayMs);  // Consider alternatives if responsiveness is a concern.
}

void initWifi(){
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
}

void configureUWB() {
  if (UWB_MODE == 0) {
    for (int b = 0; b < 2; b++) {  // Repeat twice to stabilize the connection
      for (int i = 0; i < numCommands; i++) {
        // Send the reset command only in the first cycle.
        if (i != (numCommands - 1) || b == 0) {
          sendCommandWithDelay(commands[i], 50);
        }
      }
    }
  }
  delay(100);
}

// Function to disable Serial2
void disableUWB() {
    sendCommandWithDelay(turnOffCommand, 50);
    UWBEnabled = false;  // Update flag to indicate Serial2 is disabled
}

void enableUWB(){
    sendCommandWithDelay(turnOnCommand, 50);
    UWBEnabled = true;
}

void startM5Stack() {
    M5.begin();
    Serial2.begin(115200, SERIAL_8N1, 33, 32);
    delay(100);
}

void setup() {
    startM5Stack();
    configureUWB();
    initWifi();
}

void loop() {
  M5.update();
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(timeServerUrl);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      String response = http.getString();
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, response);

      int seconds = doc["seconds"];
      Serial.println(seconds);
      if(seconds >= 31 && seconds <= 59){
        if(UWBEnabled){
          readPositionData();
          getPosition();
        }else{
          enableUWB();
          readPositionData();
          getPosition();
        }
      }else{
        disableUWB();
      }

      // Example of integrating the seconds value into existing logic
      // For example, use `seconds` in your sendDataToServer function or other operations.
    } else {
      Serial.print("Error on HTTP request: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
  delay(350);
}