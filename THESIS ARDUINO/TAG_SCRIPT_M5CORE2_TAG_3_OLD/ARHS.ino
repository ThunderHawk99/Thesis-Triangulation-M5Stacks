float pitch = 0.0F;
float roll  = 0.0F;
float yaw   = 0.0F;
int initRoll = 0;  // Initialize initRoll with a default value

void printAHRS(float pitchValue, float rollValue, float yawValue) {
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("pitch,  roll,  yaw");
  M5.Lcd.setCursor(0, 42);
  M5.Lcd.printf("%5.2f  %5.2f  %5.2f deg", pitchValue, rollValue, yawValue);
}

void initARHS() {
  int stableCount = 0;
  while (stableCount < 50) {
    float currentRoll = 0.0F;
    M5.IMU.getAhrsData(&pitch, &currentRoll, &yaw); 
    int roundedRoll = int(currentRoll + 0.5);  
    
    // Check if current roundedRoll is not 0 and matches previous initRoll
    if (roundedRoll != 0 && roundedRoll == initRoll) {
      stableCount++;
    } else if (roundedRoll != 0) {
      stableCount = 0;  // Reset stable count if not stable
      initRoll = roundedRoll;
    }
  }
  
  // M5.Lcd.setCursor(0, 62);
  // M5.Lcd.printf("Init roll value: %d", initRoll);
}

void printMovingStatus(bool moving) {  
  if (moving) {
    M5.Lcd.setCursor(0, 90);
    M5.Lcd.printf("Status: Moving      "); // Add spaces to overwrite any leftover characters
  } else {
    M5.Lcd.setCursor(0, 90);
    M5.Lcd.printf("Status: Not Moving      "); // Add spaces to overwrite any leftover characters
  }
}

void getAHRS() {
  M5.IMU.getAhrsData(&pitch, &roll, &yaw); 
  // printAHRS(pitch, roll, yaw);
}

bool isMoving() {
  getAHRS();
  int rollValue = int(roll + 0.5);
  bool moving = abs(rollValue - initRoll) >= 5;
  return moving;
}
