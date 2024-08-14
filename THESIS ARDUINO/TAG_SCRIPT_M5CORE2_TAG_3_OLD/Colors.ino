// Function to show green color on the bottom part of the screen
void showGreen() {
  int screenHeight = M5.Lcd.height();
  int statusBarHeight = 50;                         // Height of the status bar
  int statusBarY = screenHeight - statusBarHeight;  // Y-coordinate of the status bar

  M5.Lcd.fillRect(0, statusBarY, M5.Lcd.width(), statusBarHeight, TFT_GREEN);
}

// Function to show red color on the bottom part of the screen
void showRed() {
  int screenHeight = M5.Lcd.height();
  int statusBarHeight = 50;                         // Height of the status bar
  int statusBarY = screenHeight - statusBarHeight;  // Y-coordinate of the status bar

  M5.Lcd.fillRect(0, statusBarY, M5.Lcd.width(), statusBarHeight, TFT_RED);
}

// Function to reset the top part of the screen to white
void resetTopScreen() {
  int screenHeight = M5.Lcd.height();
  int statusBarHeight = 50;                         // Height of the status bar
  int contentHeight = screenHeight - statusBarHeight; // Height of the area above the status bar

  // Set the background color to white
  M5.Lcd.fillRect(0, 0, M5.Lcd.width(), contentHeight, TFT_WHITE);
}

void showSwapMessage() {
    int screenHeight = M5.Lcd.height();
    int statusBarHeight = 50;                         // Height of the status bar
    int statusBarY = screenHeight - statusBarHeight;  // Y-coordinate of the status bar

    // Set the text color to white
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextSize(2);                            // Set the size of the text

    // Calculate the center position for the text
    int textWidth = M5.Lcd.textWidth("SWAP");         // Get the width of the text
    int textX = (M5.Lcd.width() - textWidth) / 2;     // X-coordinate for centered text
    int textY = statusBarY + (statusBarHeight - 16) / 2; // Y-coordinate for centered text, considering text height

    // Draw the text "SWAP" in the center of the status bar
    M5.Lcd.setCursor(textX, textY);
    M5.Lcd.print("SWAP");
}
