#include "ShiftIn.h"
int LATCH = 16; // (1)
int DATA  = 12; // (9)
int CLOCK = 14; // (2)
int clockEnablePin = 5; // (15)


ShiftIn<2> shift;

void setup() {
  Serial.begin(115200);
  // declare pins: pLoadPin, clockEnablePin, dataPin, clockPin
  //shift.begin(8, 9, 11, 12);
  shift.begin(LATCH, clockEnablePin, DATA, CLOCK);
}

void displayValues() {
  for(int i = 0; i < shift.getDataWidth(); i++)
    Serial.print( shift.state(i) ); // get state of button i
  Serial.println();
}

void loop() {
  if(shift.update()) // read in all values. returns true if any button has changed
    displayValues();
  delay(50);
}
