#include <M5Core2.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <stdint.h>
#include <pgmspace.h>

extern const uint16_t logo[];
extern const uint16_t north_arrow[];
extern const uint16_t east_arrow[];
extern const uint16_t south_arrow[];
extern const uint16_t west_arrow[];
extern const uint16_t northeast_arrow[];
extern const uint16_t northwest_arrow[];
extern const uint16_t southeast_arrow[];
extern const uint16_t southwest_arrow[];
extern const unsigned char swap_sound[];
extern const int swap_sound_len;
bool IN_QUEUE = false;
bool WAS_MOVING = false;
extern String DISTANCE_A;
extern String DISTANCE_B;
extern String DISTANCE_C;
extern String DISTANCE_D;
extern int TAG_ID;

extern void playSwapSound();
extern void playFinishGameSound();

// Relative tag position audio
// extern void play_move_right_down(); // GOOD
// extern void play_move_left_down(); // GOOD
// extern void play_move_right_up(); // GOOD
// extern void play_move_left_up(); // GOOD
// extern void play_move_up(); // GOOD
// extern void play_move_down(); // GOOD
// extern void play_move_left(); // GOOD 
// extern void play_move_right(); // GOOD

// // Relative tag position audio box
// extern void play_move_down_to_box(); // GOOD
// extern void play_move_up_to_box(); // GOOD
// extern void play_move_left_to_box(); // GOOD
// extern void play_move_right_to_box(); // GOOD

// extern void play_move_right_down_to_box(); // GOOD
// extern void play_move_right_up_to_box(); // GOOD
// extern void play_move_left_down_to_box(); // GOOD
// extern void play_move_left_up_to_box(); // GOOD


using namespace websockets;
extern WebsocketsClient client;
void displayCustomMessage(String message);
void connectWebSocket();
void sendHeartbeat();
void sendConfirmAllowMove(int TAG_ID);
void initWifi();
void enableUWB();
void disableUWB();
void configureUWB();
bool getPosition();
void sendDataToServer(String tagId, String dA, String dB, String dC, String dD);
void sendRequestMove(int TAG_ID);
void sendMovementDone(int TAG_ID);
void resetTopScreen();

void showDirection(String direction) {
  resetTopScreen();

  if (direction == "North") {
    M5.lcd.drawBitmap(0, 0, 114, 150, north_arrow);
  } else if (direction == "East") {
    M5.lcd.drawBitmap(0, 0, 198, 150, east_arrow);
  } else if (direction == "South") {
    M5.lcd.drawBitmap(0, 0, 115, 150, south_arrow);
  } else if (direction == "West") {
    M5.lcd.drawBitmap(0, 0, 198, 150, west_arrow);
  } else if (direction == "EastNorth") {
    M5.lcd.drawBitmap(0, 0, 164, 150, northeast_arrow);
  } else if (direction == "EastSouth") {
    M5.lcd.drawBitmap(0, 0, 150, 150, southeast_arrow);
  } else if (direction == "WestNorth") {
    M5.lcd.drawBitmap(0, 0, 147, 150, northwest_arrow);
  } else if (direction == "WestSouth") {
    M5.lcd.drawBitmap(0, 0, 158, 150, southwest_arrow);
  } else if (direction == "finish_game") {
    resetTopScreen();
    M5.Axp.SetVibration(true);
    delay(3000);
    M5.Axp.SetVibration(false);
    playFinishGameSound();
  } else if (direction == "inplace") {
    resetTopScreen();
  } else {
    Serial.println("Unknown direction: " + direction);
  }
}

void doMove() {
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
  resetTopScreen();
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
  String id = doc["tag_id"].as<String>();
  // Check if message is meant for this device
  if (id == String(TAG_ID)) {

    if(type == "tag_2_placement") {
      resetTopScreen();
      String message = doc["message"].as<String>();
      Serial.println(message);
      displayCustomMessage(message);
    }

    if (type == "movement_message") {
      resetTopScreen();
      String message = doc["message"].as<String>();
      Serial.println(message);
      if(message == "Move it right, up next to tag 2"){
        // play_move_right_up();
        displayCustomMessage("Move up and place on right to tag 2");
      }
      if(message == "Move it right, down next to tag 2"){
        displayCustomMessage("Move down and place on right to tag 2");
        // play_move_right_down();
      }
      if(message == "Move it left, up next to tag 2"){
        displayCustomMessage("Move up and place on left to tag 2");
        // play_move_left_up();
      }
      if(message == "Move it left, down next to tag 2"){
        displayCustomMessage("Move down and place on left to tag 2");
        // play_move_left_down();
      }
      if(message == "Move it right next to tag 2"){
        displayCustomMessage("Move right to tag 2");
        // play_move_right();
      }
      if(message == "Move it left next to tag 2"){
        displayCustomMessage("Move left to tag 2");
        // play_move_left();
      }
      if(message == "Move it up next to tag 2"){
        displayCustomMessage("Move up to tag 2");
        // play_move_up();
      }
      if(message == "Move it down next to tag 2"){
        displayCustomMessage("Move down to tag 2");
        // play_move_down();
      }
    }

    if (type == "movement_message_box") {
      resetTopScreen();
      String message = doc["message"].as<String>();
      displayCustomMessage(message);
    }

    if (type == "swap") {
      String direction = doc["direction"];
      showDirection(direction);
      playSwapSound();
    }

    if (type == "direction") {
      String direction = doc["direction"];
      showDirection(direction);
    }

    if (type == "allow_move") {
      Serial.println("Received allow move");
      sendConfirmAllowMove(TAG_ID);
    }

    if (type == "do_move") {
      doMove();
    }

  } else if (type == "error") {
    // Handle error message
    Serial.print("Error: ");
    Serial.println(doc["message"].as<String>());
  } else {
    // Handle unknown message type
    // Serial.println("Message was meant for: " + id);
  }
}

void startM5Stack() {
  M5.begin();
  Serial2.begin(115200, SERIAL_8N1, 33, 32);
  delay(100);
}

void setup() {
  startM5Stack();
  M5.Spk.InitI2SSpeakOrMic(MODE_SPK);
  delay(100);
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.setTextSize(2);
  // M5.Lcd.drawBitmap(0,0,114,150,north_arrow);

  // Init WIFI
  initWifi();
  // Callback for messages
  client.onMessage(onMessageCallback);
  // Send ping
  client.ping();
  // Start IMU
  M5.IMU.Init();
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
  M5.update();
  client.poll();
  // If not in queue
  if (!IN_QUEUE) {
    bool moving = isMoving();
    // printMovingStatus(moving);

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