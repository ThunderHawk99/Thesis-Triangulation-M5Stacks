#include <M5Core2.h>
String DATA  = " ";  // Used to store distance data . 
int UWB_MODE = 1;    // Used to set UWB mode . 
int ANCHOR_ID = 0;

// UI display
void displayBtnBase() {
    M5.Lcd.drawString("Base", 130, 210, 4);
}

void displayBaseStationID(){
    M5.Lcd.drawString("Base station: " + String(ANCHOR_ID), 80, 50, 4);
}

//  Display and data clear.
void logAndClearDataUWB() {
    if (Serial2.available()) {
        delay(3);
        DATA = Serial2.readString();
        Serial.println(DATA);
    }
    DATA = "";
    M5.Lcd.fillRect(0, 50, 340, 150, BLACK);
}

// AT command
void configureUWB() {
    ANCHOR_ID = ANCHOR_ID % 4;
    for (int b = 0; b < 2; b++) {
        delay(50);
        Serial2.print("AT+anchor_tag=1," + String(ANCHOR_ID) + "\r\n");  // Set the base station
        delay(50);
        if (b == 0) {
            Serial2.write("AT+RST\r\n");
        }
    }
    logAndClearDataUWB();
    displayBaseStationID();
    ANCHOR_ID++;
}

void UWB_Keyscan() {
    if (M5.BtnB.isPressed()) {
        configureUWB();
    }
}

void startM5Stack() {
    M5.begin();
    Serial2.begin(115200, SERIAL_8N1, 33, 32);
    delay(100);
}

void setup() {
    startM5Stack();
    displayBtnBase();
}

void loop() {
  
    M5.update();
    UWB_Keyscan();
}