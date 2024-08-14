void displayCustomMessage(String message) {
    // Set text size. Adjust this value if needed.
    M5.Lcd.setTextSize(5);

    // Set text color to black
    M5.Lcd.setTextColor(TFT_BLACK);

    // Define the padding for the text
    int16_t x = 5; // Padding from the left side
    int16_t y = 5; // Initial padding from the top

    // Set the cursor position and display the message
    M5.Lcd.setCursor(x, y);
    M5.Lcd.println(message);
}