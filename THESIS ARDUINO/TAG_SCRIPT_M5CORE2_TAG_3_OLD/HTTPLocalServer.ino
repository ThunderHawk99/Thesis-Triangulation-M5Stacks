#include <WiFi.h>
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>

// WiFi settings
const char* WIFI_SSID = "dafarbel";
const char* WIFI_PASSWORD = "f11021974";
const char* WEBSOCKETS_SERVER_HOST = "192.168.178.182";  // Server address
const uint16_t WEBSOCKETS_SERVER_PORT = 5000;            // Server port

using namespace websockets;
WebsocketsClient client;

void sendHeartbeat() {
  Serial.println("Heartbeat");
  StaticJsonDocument<200> doc;
  doc["type"] = "heartbeat_confirmed";
  doc["tag_id"] = TAG_ID;  // Your specific tag ID

  String message;
  serializeJson(doc, message);
  client.send(message);
}

void sendConfirmAllowMove(int TAG_ID) {
  Serial.println("Send confirm allow move");
  StaticJsonDocument<200> doc;
  doc["type"] = "allow_move_confirmed";
  doc["tag_id"] = TAG_ID;  // Your specific tag ID

  String message;
  serializeJson(doc, message);
  client.send(message);
}

void sendRequestMove(int TAG_ID) {
  StaticJsonDocument<200> doc;
  doc["type"] = "request_move";
  doc["tag_id"] = TAG_ID;  // Your specific tag ID

  String message;
  serializeJson(doc, message);
  client.send(message);
}

void sendMovementDone(int TAG_ID) {
  StaticJsonDocument<200> doc;
  doc["type"] = "movement_done";
  doc["tag_id"] = TAG_ID;  // Your specific tag ID

  String message;
  serializeJson(doc, message);
  client.send(message);
}

// Initialize WiFi connection
void initWifi() {
  Serial.print("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);  // Explicitly set WiFi mode to STA
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nConnected to WiFi");
  connectWebSocket();  // Connect to WebSocket server after WiFi is connected
}

void onEventsCallback(WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionOpened) {
    Serial.println("Connnection Opened");
  } else if (event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("Connnection Closed");
    initWifi();
  } else if (event == WebsocketsEvent::GotPing) {
    Serial.println("Got a Ping!");
    client.pong();  // Respond with a Pong
  } else if (event == WebsocketsEvent::GotPong) {
    Serial.println("Got a Pong!");
  }
}

// Connect to the WebSocket server
void connectWebSocket() {
  bool connected = client.connect(WEBSOCKETS_SERVER_HOST, WEBSOCKETS_SERVER_PORT, "/");

  if (connected) {
    // Callback for events
    client.onEvent(onEventsCallback);
    Serial.println("Connected to WebSocket server!");
  } else {
    Serial.println("Failed to connect to WebSocket server!");
  }
}

// Convert string to measurement value
int stringToMeasurementValue(String value) {
  float f_value = value.toFloat() * 100;
  return static_cast<int>(f_value);
}

// Send relative positions to server
void sendDataToServer(String tagId, String dA, String dB, String dC, String dD) {
  if (WiFi.status() == WL_CONNECTED) {
    StaticJsonDocument<200> doc;
    doc["type"] = "update_measurements";  // Specify the event type
    JsonObject data = doc.createNestedObject("data");
    data["tagID"] = tagId.toInt();
    data["distance1"] = stringToMeasurementValue(dA);
    data["distance2"] = stringToMeasurementValue(dB);
    data["distance3"] = stringToMeasurementValue(dC);
    data["distance4"] = stringToMeasurementValue(dD);

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    Serial.println("Sending payload: " + jsonPayload);  // Debug: print the payload
    client.send(jsonPayload);
  } else {
    Serial.println("WiFi disconnected or WebSocket not available.");
  }
}
