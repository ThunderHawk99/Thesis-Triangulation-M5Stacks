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