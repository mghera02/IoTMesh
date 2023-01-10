#include <SD.h>                   
#define SD_ChipSelectPin 4
#include <TMRpcm.h>           
#include <SPI.h>

// bit 16000 mono u8

TMRpcm tmrpcm;  

class LED {
  public: 
    int ledPin;
    LED(int pin);
    void turnOn();
    void turnOff();
};

LED::LED(int pin) {
  ledPin = pin;
  pinMode(ledPin, OUTPUT);
}

void LED::turnOn(){
  digitalWrite(ledPin, HIGH);
}

void LED::turnOff(){
  digitalWrite(ledPin, LOW);
}

int ahsokaPin = 2;
int anakinPin = 3;
int quigonPin = 5;
int obiwanPin = 6;
int macePin = 7;
int yodaPin = 8;
int sec = 1000;

LED ahsoka(ahsokaPin);
LED anakin(anakinPin);
LED quigon(quigonPin);
LED obiwan(obiwanPin);
LED mace(macePin);
LED yoda(yodaPin);

void setup(){
  tmrpcm.setVolume(5);
  tmrpcm.speakerPin = 9; //5,6,11 or 46 on Mega, 9 on Uno, Nano, etc
  
  Serial.begin(9600);
  if (!SD.begin(SD_ChipSelectPin)) {  // see if the card is present and can be initialized:
    Serial.println("SD fail");  
    return;   // don't do anything more if not
  } 
}

void waveRight() {
  ahsoka.turnOn();
  delay(sec * .15);
  ahsoka.turnOff();
  anakin.turnOn();
  delay(sec * .13);
  anakin.turnOff();
  quigon.turnOn();
  delay(sec * .11);
  quigon.turnOff();
  obiwan.turnOn();
  delay(sec * .09);
  obiwan.turnOff();
  mace.turnOn();
  delay(sec * .07);
  mace.turnOff();
  yoda.turnOn();
  delay(sec * .05);
  yoda.turnOff();
  delay(sec * .35);
}

void waveLeft() {
  yoda.turnOn();
  delay(sec * .15);
  yoda.turnOff();
  mace.turnOn();
  delay(sec * .13);
  mace.turnOff();
  obiwan.turnOn();
  delay(sec * .11);
  obiwan.turnOff();
  quigon.turnOn();
  delay(sec * .09);
  quigon.turnOff();
  anakin.turnOn();
  delay(sec * .07);
  anakin.turnOff();
  ahsoka.turnOn();
  delay(sec * .05);
  ahsoka.turnOff(); 
  delay(sec * .35);
}

void waveRightFast() {
  ahsoka.turnOn();
  delay(sec * .13);
  ahsoka.turnOff();
  anakin.turnOn();
  delay(sec * .11);
  anakin.turnOff();
  quigon.turnOn();
  delay(sec * .09);
  quigon.turnOff();
  obiwan.turnOn();
  delay(sec * .07);
  obiwan.turnOff();
  mace.turnOn();
  delay(sec * .05);
  mace.turnOff();
  yoda.turnOn();
  delay(sec * .03);
  yoda.turnOff();
  delay(sec * .35);
}

void randomClash() {
  mace.turnOn();
  delay(sec * .5);
  mace.turnOff();
  anakin.turnOn();
  delay(sec * .5);
  anakin.turnOff();
  yoda.turnOn();
  delay(sec * .75);
  yoda.turnOff();
  obiwan.turnOn();
  delay(sec * .5);
  obiwan.turnOff();
  ahsoka.turnOn();
  delay(sec * .5);
  ahsoka.turnOff();
  quigon.turnOn();
  delay(sec * .25);
  quigon.turnOff();
  anakin.turnOn();
  delay(sec * .25);
  anakin.turnOff();
  mace.turnOn();
  delay(sec * .25);
  mace.turnOff();
  ahsoka.turnOn();
  delay(sec * .5);
  ahsoka.turnOff();
  obiwan.turnOn();
  delay(sec * .5);
  obiwan.turnOff();
  mace.turnOn();
  delay(sec * .5);
  mace.turnOff();
  yoda.turnOn();
  delay(sec * .5);
  yoda.turnOff();
  ahsoka.turnOn();
  delay(sec * .25);
  ahsoka.turnOff();
  anakin.turnOn();
  delay(sec * .25);
  anakin.turnOff();
  quigon.turnOn();
  delay(sec * .25);
  quigon.turnOff();
  obiwan.turnOn();
  delay(sec * .25);
  obiwan.turnOff();
  mace.turnOn();
  delay(sec * .5);
  mace.turnOff();
}

void loop(){  
  /*tmrpcm.play("jed2.wav", 0); // Play first segment

  waveRight();
  waveLeft();
  delay(sec * 2.5);
  waveRightFast();
  delay(sec * .5);
  randomClash();*/
  yoda.turnOn();
  mace.turnOn();
  obiwan.turnOn();
  anakin.turnOn();
  ahsoka.turnOn();
  

  delay(sec * 1);
}
