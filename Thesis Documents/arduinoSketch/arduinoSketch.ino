#include <M5Core2.h>
/* After M5Core2 is started or reset
the program in the setUp () function will be run, and this part will only be run once.
After M5Core2 is started or reset, the program in the setup() function will be executed, and this part will only be executed once. */
void setup() {
  M5.Lcd.print("TEST");

  
}

void loop() {
  // put your main code here, to run repeatedly:

}
