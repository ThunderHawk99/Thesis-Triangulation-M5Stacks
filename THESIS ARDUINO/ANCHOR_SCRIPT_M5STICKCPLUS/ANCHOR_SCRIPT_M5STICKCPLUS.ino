#include <M5StickCPlus.h>  // Adjust library for M5Stick C Plus

String DATA = " ";  // Used to store distance data
int UWB_MODE = 1;    // Used to set UWB mode
int ANCHOR_ID = 0;

// UI display
void displayBtnBase() {
    M5.Lcd.setCursor(0, 0);  // Set cursor to top-left
    M5.Lcd.setTextSize(2);   // Smaller text size due to smaller screen
    M5.Lcd.println("Base");
}

void displayBaseStationID() {
    M5.Lcd.setCursor(0, 20); // Adjust position for smaller screen
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("Base station: %d", ANCHOR_ID);
}

void displayBatteryLevel() {
    float voltage = M5.Axp.GetBatVoltage();
    int batteryPercentage = map(voltage * 1000, 3200, 4200, 0, 100);  // Convert voltage to millivolts and map to percentage
    batteryPercentage = constrain(batteryPercentage, 0, 100);  // Constrain percentage to valid range

    M5.Lcd.setCursor(0, 40);
    M5.Lcd.printf("Battery: %d%%", batteryPercentage);

    M5.Lcd.setCursor(0, 60);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("Voltage: %.2fV", voltage);
}

// Display and data clear.
void logAndClearDataUWB() {
    if (Serial2.available()) {
        delay(3);
        DATA = Serial2.readString();
        Serial.println(DATA);
    }
    DATA = "";
    M5.Lcd.fillScreen(BLACK);  // Fill entire screen with black
}

// AT command
void configureUWB() {
    ANCHOR_ID = ANCHOR_ID % 4;
    for (int b = 0; b < 2; b++) {
        delay(50);
        Serial2.printf("AT+anchor_tag=1,%d\r\n", ANCHOR_ID);  // Set the base station
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
    if (M5.BtnA.wasPressed()) {  // M5Stick C Plus uses BtnA and BtnB
        configureUWB();
    }
}

void startM5StickCPlus() {
    M5.begin();
    M5.Lcd.setRotation(1);  // Optional: Adjust rotation to your preference
    Serial2.begin(115200, SERIAL_8N1, 33, 32);
    delay(100);
}

void setup() {
    startM5StickCPlus();
    displayBtnBase();
}

void loop() {
    M5.update();
    UWB_Keyscan();
    displayBatteryLevel();
}
