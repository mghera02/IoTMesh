#include "ShiftIn.h"
#include <math.h>

int LATCH = 16; // (1) (esp8266 was 16(D0) -> huzzah is IO16 (pin4))
int DATA  = 12; // (9) (esp8266 was 12(D6) -> huzzah is IO12 (pin 6))
int CLOCK = 14; // (2) (esp8266 was 14(D5) -> huzzah is IO14 (pin 5))
int clockEnablePin = 15; // (esp8266 was 5(D1) -> huzzah is IO15 (pin10))

ShiftIn<2> shift;
String numPadStr = "";
int currStrLen = 0;

void setup() {
  Serial.begin(115200);
  shift.begin(LATCH, clockEnablePin, DATA, CLOCK);
}

int displayValues() {
  int shiftedInDecimalNumber = 0;
  for(int i = 0; i < shift.getDataWidth(); i++) {
    shiftedInDecimalNumber += shift.state(i) * pow(2, shift.getDataWidth() - i - 1);
    Serial.print( shift.state(i) );
  }
  Serial.println();
  Serial.println("That number in dec was: "+ String(shiftedInDecimalNumber));
  return shiftedInDecimalNumber;
}

void addNumToStr(int num) {
  if(numPadStr.length() <= 3) {
      Serial.printf("adding %d to the number pad stack\n", num);
      numPadStr += String(num);
      currStrLen++;
  }
}

void loop() {
  int numpadNumber = 0;
  if(shift.update()) {
    numpadNumber = displayValues();
  }
  
  switch(numpadNumber) {
    case 4096: // keypad number 1 equates to #0
      addNumToStr(0);
      break;
    case 8192: // keypad number 2 equates to #1
      addNumToStr(1);
      break;
    case 16384: // keypad number 3 equates to #2
      addNumToStr(2);
      break;
    case 32768: // keypad number 4 equates to #3
      addNumToStr(3);
      break;
    case 2048: // keypad number 5 equates to #4
      addNumToStr(4);
      break;
    case 1024: // keypad number 6 equates to #5
      addNumToStr(5);
      break;
    case 512: // keypad number 7 equates to #6
      addNumToStr(6);
      break;
    case 256: // keypad number 8 equates to #7
      addNumToStr(7);
      break;
    case 16: // keypad number 9 equates to #8
      addNumToStr(8);
      break;
    case 32: // keypad number 10 equates to #9
      addNumToStr(9);
      break;
    case 64: // keypad number 11 equates to DELETE
      numPadStr = numPadStr.substring(0, --currStrLen);
      break;
    case 128: // keypad number 12 equates to LEFT
      // Left code here
      break;
    case 8: // keypad number 13 equates to RIGHT
      // right code here
      break;
    case 4: // keypad number 14 equates to SUBMIT
      // submit code here
      break;
  }

  Serial.println("Current num pad string: " + numPadStr);
  
  delay(50);
}
