#include <RBDdimmer.h> // For AC dimmer
#define acdPin 3 // For AC Dimmer PSM Pin
#define zeroCrossPin 2 // For AC Dimmer ZC Pin
#define USE_SERIAL  Serial

dimmerLamp acd(acdPin);

int powerPin = A0;
int connectedPin = A1;

long avgConnect = 0;
long avgPower = 0;
int avgCounter = -1;
long avgAvgPower = 0;
int avgAvgCounter = -1;
int lastAvgPower = -1;


void setup() {
  Serial.begin(115200);
  pinMode(powerPin, INPUT);
  pinMode(connectedPin, INPUT);
  acd.begin(NORMAL_MODE, ON);
}

void changeDimmer(int x){
  x = (x / 50) + 32 ;
  /*Serial.print("Adjusted power:");
  Serial.println(x);*/
  acd.setPower(x); 
}

void loop() {
  long mesh2Val = analogRead(powerPin);
  long connectVal = analogRead(connectedPin);
  //Serial.print("Input power:");
  //Serial.println(mesh2Val);

  avgConnect += connectVal;
  avgPower += mesh2Val;
  avgCounter += 1;
  
  if(avgCounter == 100) {
    avgPower /= 100;
    avgConnect /= 100;
    bool isConnect = avgConnect > 100;
    if(lastAvgPower != -1 && (!isConnect || lastAvgPower > avgPower + 200 || lastAvgPower < avgPower - 200)) {
      Serial.print("Preventing panic   ");
      Serial.print(" avgPower: ");
      Serial.print(avgPower);
      Serial.print(" lastAvgPower: ");
      Serial.println(lastAvgPower);
      delay(1000);
    } else {
      if(avgPower > 60) {
        avgAvgPower += avgPower;
        avgAvgCounter++;
        if(avgAvgCounter == 10) {
          avgAvgPower /= 10;
          avgAvgCounter = 0;
          Serial.print("Updated power:");
          Serial.println(avgAvgPower);
          changeDimmer(avgAvgPower); 
        }
      }
    }
    lastAvgPower = avgPower;
    avgCounter = 0;  
  } 
}
