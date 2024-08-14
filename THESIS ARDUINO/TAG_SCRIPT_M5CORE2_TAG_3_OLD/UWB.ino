String DISTANCE_A = "1";
String DISTANCE_B = "1";
String DISTANCE_C = "1";
String DISTANCE_D = "1";

bool UWB_ENABLED = false;
String DATA = " ";     // Used to store distance data .
int UWB_T_NUMBER = 1;  // Current number of UWB Tags detected
int TAG_ID = 3;
String commands[4] = { "AT+anchor_tag=0," + String(TAG_ID) + "\r\n", "AT+interval=5\r\n", "AT+switchdis=0\r\n", "AT+RST\r\n" };

int numCommands = 4;

const int startOffset = 2;          // Starting index for anchorID in the data string
const int anchorIdLength = 1;       // Length of the anchor ID
const int distanceStartOffset = 4;  // Starting index for distance in the data string
const int distanceLength = 4;       // Length of the distance string
const int dataPacketSize = 11;      // Size of one complete data packet
const int yBase = 50;               // Base y-coordinate for drawing strings
const int yIncrement = 40;          // Y increment per data entry

// Get position
bool getPosition() {
  if (Serial2.available() >= 11) {  // Ensure enough data is available
    delay(20);
    UWB_T_NUMBER = (Serial2.available() / 11);  // Count the number of Base stations, each base station sends a packet of 11 bytes to a tag
    delay(20);
    DATA = Serial2.readString();
    // Serial.println(DATA);
    delay(20);

    if (UWB_T_NUMBER > 0 && UWB_T_NUMBER < 5) {
      for (int i = 0; i < UWB_T_NUMBER; i++) {
        int anchorIdStart = startOffset + i * dataPacketSize;
        int distanceStart = distanceStartOffset + i * dataPacketSize;
        String anchorID = DATA.substring(anchorIdStart, anchorIdStart + anchorIdLength);
        String distance = DATA.substring(distanceStart, distanceStart + distanceLength);
        Serial.println("AnchorID: " + anchorID + " | Distance: " + distance + "m");
        if (anchorID == "0") {
          DISTANCE_A = distance;
        }
        if (anchorID == "1") {
          DISTANCE_B = distance;
        }
        if (anchorID == "2") {
          DISTANCE_C = distance;
        }
        if (anchorID == "3") {
          DISTANCE_D = distance;
        }
      }
      Serial.println("----------------------------------------------------------------");
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

void sendCommandWithDelay(const String& command, unsigned long delayMs) {
  Serial2.print(command);
  delay(delayMs);  // Consider alternatives if responsiveness is a concern.
}

void configureUWB() {
  for (int b = 0; b < 2; b++) {  // Repeat twice to stabilize the connection
    for (int i = 0; i < numCommands; i++) {
      // Send the reset command only in the first cycle.x
      if (i != (numCommands - 1) || b == 0) {
        sendCommandWithDelay(commands[i], 50);
      }
    }
  }
  delay(100);
}

// Function to disable Serial2
void disableUWB() {
  String TURN_OFF_COMMAND = "AT+switchdis=0\r\n";
  sendCommandWithDelay(TURN_OFF_COMMAND, 50);
  // Wait until u get OK response
  String response = "";
  while (true) {
    if (Serial2.available()) {
      response += Serial2.readString();
      if (response.indexOf("OK") != -1) {
        UWB_ENABLED = false;
        Serial.println("UWB disabled successfully.");
        return;
      }
    }
    delay(10);  // Small delay to avoid busy waiting
  }
}

void enableUWB() {
  String TURN_ON_COMMAND = "AT+switchdis=1\r\n";
  sendCommandWithDelay(TURN_ON_COMMAND, 50);

  // Wait until u get OK response
  String response = "";
  while (true) {
    if (Serial2.available()) {
      response += Serial2.readString();
      if (response.indexOf("OK") != -1) {
        UWB_ENABLED = true;
        Serial.println("UWB enabled successfully.");
        return;
      }
    }
    delay(10);  // Small delay to avoid busy waiting
  }
}

void UWB_clear() {
  if (Serial2.available()) {
    delay(3);
    DATA = Serial2.readString();
  }
  DATA = "";
}