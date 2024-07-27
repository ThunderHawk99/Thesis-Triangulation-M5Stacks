#include <M5Core2.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>

bool IN_QUEUE = false;
bool WAS_MOVING = false;
extern String DISTANCE_A;
extern String DISTANCE_B;
extern String DISTANCE_C;
extern String DISTANCE_D;
extern int TAG_ID;

using namespace websockets;
extern WebsocketsClient client;
void connectWebSocket();
void initWifi();
void enableUWB();
void disableUWB();
void configureUWB();
bool getPosition();
void sendDataToServer(String tagId, String dA, String dB, String dC, String dD);
void sendRequestMove(int TAG_ID);
void sendMovementDone(int TAG_ID);

void onEventsCallback(WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionOpened) {
    Serial.println("Connnection Opened");
  } else if (event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("Connnection Closed");
    connectWebSocket();
  } else if (event == WebsocketsEvent::GotPing) {
    Serial.println("Got a Ping!");
  } else if (event == WebsocketsEvent::GotPong) {
    Serial.println("Got a Pong!");
  }
}

// Define your callback function
void onMessageCallback(WebsocketsMessage message) {
  // Parse the received message
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message.data());

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Check the type of the message and handle accordingly
  String type = doc["type"].as<String>();

  if (type == "direction") {
    String direction = doc["direction"];
    // Handle test message
    Serial.println(direction);

  } else if (type == "allow_move") {
    // Handle allow_move message
    int tag_id = doc["tag_id"].as<int>();
    Serial.print("Tag ");
    Serial.print(tag_id);
    Serial.println(" is allowed to move.");
    if (tag_id == TAG_ID) {
      enableUWB();
      int valuesRead = 0;
      while (valuesRead <= 5) {  // Get 6 measurements
        bool read_position = getPosition();
        if (read_position) {
          valuesRead++;
        }
      }
      sendDataToServer(String(TAG_ID), DISTANCE_A, DISTANCE_B, DISTANCE_C, DISTANCE_D);
      disableUWB();
      delay(100);
      sendMovementDone(TAG_ID);
      showGreen();
      IN_QUEUE = false;
    }

  } else if (type == "error") {
    // Handle error message
    Serial.print("Error: ");
    Serial.println(doc["message"].as<String>());
  } else {
    // Handle unknown message type
    Serial.println("Unknown message type received. : " + type);
  }
}

void startM5Stack() {
  M5.begin();
  Serial2.begin(115200, SERIAL_8N1, 33, 32);
  delay(100);
}

void setup() {
  // Start M5Stack
  startM5Stack();
  // Init WIFI
  initWifi();
  // Callback for messages
  client.onMessage(onMessageCallback);
  // Callback for events
  client.onEvent(onEventsCallback);
  // Start IMU
  M5.IMU.Init();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setTextSize(2);
  // Init ARHS
  initARHS();
  // Configure UWB
  configureUWB();
  // Show Green LED
  sendRequestMove(TAG_ID);
  IN_QUEUE = true;
  showRed();

}

void loop() {
  if (client.available()) {
    client.poll();
  }
  M5.update();

  // If not in queue
  if (!IN_QUEUE) {
    bool moving = isMoving();
    printMovingStatus(moving);

    // Was moving and still moving
    if (moving && WAS_MOVING) {
      // Do nothing
    }

    // Was not moving but started moving
    if (moving && !WAS_MOVING) {
      WAS_MOVING = true;
      showRed();
    }

    // Was moving but stopped moving
    if (!moving && WAS_MOVING) {
      sendRequestMove(TAG_ID);
      IN_QUEUE = true;
      WAS_MOVING = false;
    }
  }
}