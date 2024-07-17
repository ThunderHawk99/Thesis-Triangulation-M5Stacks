#include <M5Stack.h>
String DATA  = " ";  // Used to store distance data . 
int UWB_MODE = 0;    // Used to set UWB mode . 
int UWB_T_NUMBER      = 0;  // Current number of UWB Tags detected

String commands[4] = {"AT+anchor_tag=0\r\n", "AT+interval=5\r\n", "AT+switchdis=1\r\n", "AT+RST\r\n"};
int numCommands = 4;

const int startOffset = 2;  // Starting index for anchorID in the data string
const int anchorIdLength = 1;  // Length of the anchor ID
const int distanceStartOffset = 4;  // Starting index for distance in the data string
const int distanceLength = 4;  // Length of the distance string
const int dataPacketSize = 11;  // Size of one complete data packet
const int yBase = 50;  // Base y-coordinate for drawing strings
const int yIncrement = 40;  // Y increment per data entry

// Get position
void getPosition() {
  if (UWB_T_NUMBER > 0 && UWB_T_NUMBER < 5) {

    for (int i = 0; i < UWB_T_NUMBER; i++) {
      int anchorIdStart = startOffset + i * dataPacketSize;
      int distanceStart = distanceStartOffset + i * dataPacketSize;
      String anchorID = DATA.substring(anchorIdStart, anchorIdStart + anchorIdLength);
      String distance = DATA.substring(distanceStart, distanceStart + distanceLength);
      Serial.println("AnchorID: " + anchorID + " | Distance: " + distance + "m");
      Serial.println("---------------------------------------------------------------------------------------------------------------------------");
    }
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

void startM5Stack() {
    M5.begin();
    M5.Power.begin();
    Serial2.begin(115200, SERIAL_8N1, 22, 21);
    delay(100);
}

void setup() {
    startM5Stack();
    configureUWB();
}

void loop() {
  M5.update();
  readPositionData();
  getPosition();
  delay(350);
}