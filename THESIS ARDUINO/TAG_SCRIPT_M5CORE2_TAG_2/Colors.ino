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