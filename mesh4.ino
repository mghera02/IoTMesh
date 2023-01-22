// THIS IS NODE 2741408799

// LIBRARIES
#include "painlessMesh.h" // For mesh wifi
#include "Timer.h" 

// CONSTANTS
#define   MESH_PREFIX     "IoTHub"     // Mesh WiFI name
#define   MESH_PASSWORD   "IOAIHTHG"   // Mesh password
#define   MESH_PORT       5555         // Mesh port

// GLOBAL VARIABLES
  // PINS
  int ledPin = 12;
  // 7 SEGMENT DISPLAY PINS
  int cathode1 = 5;
  int cathode2 = 4;
  int cathode3 = 0;
  int cathode4 = 2;
  
  int latchPin = 15;
  int clockPin = 14;
  int dataPin = 13;
  int lastValue = 0;
  int cathodePins[] = {5, 4, 0, 2};

  // TIMES
  unsigned long timeSinceOn = millis();
  unsigned long timeSinceUpdate = millis();
  unsigned long timeSinceMsgReceived = millis();
  unsigned long timeSinceDisplayUpdate = millis();

  // 7 SEGMENT DISPLAY
  long number = 0; 
  int num1 = 0;
  int num2 = 0;
  int num3 = 0;
  int num4 = 0;
  int timer_event = 0;
  int numbers[4] ;
  byte table[10] {B11111100, B01100000, B11011010, B11110010, B01100110, B10110110, B10111110, B11100000, B11111110, B11110110};
  int count = 0;

  // WiFi COMMUNICATION MESSAGE
  String receivedMsg;

// INITIALIZING OBJECTS
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
void sendMessage() ; // Prototype so PlatformIO doesn't complain
Timer timer; 
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void setUpMesh() {
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  //mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.setDebugMsgTypes( CONNECTION | SYNC );

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
}

void setup() {
  Serial.begin(115200);
  
  pinMode(ledPin, OUTPUT);
  
  pinMode(cathode4, OUTPUT);
  pinMode(cathode3, OUTPUT);
  pinMode(cathode2, OUTPUT);
  pinMode(cathode1, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  digitalWrite(cathode4, HIGH);
  digitalWrite(cathode3, HIGH);
  digitalWrite(cathode2, HIGH);
  digitalWrite(cathode1, HIGH);
  
  setUpMesh();
}

void sendMessage() {
  String msg = "Callback:" + receivedMsg;
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 2 ));
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  receivedMsg = msg.c_str();
  timeSinceMsgReceived = millis();
  if(atoi(msg.c_str()) != 0) {
    if(atoi(msg.c_str()) == -1) {
      lastValue = 0;  
    } else if(atoi(msg.c_str()) != -2) {
      lastValue = atoi(msg.c_str());
    }
  }
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

// 7 Segment display
void separate(long num) { 
  num1 = num / 1000;
  numbers[0] = num1;
  int num1_remove = num - (num1 * 1000);
  num2 = num1_remove / 100;
  numbers[1] = num2;
  int num2_remove = num1_remove - (num2 * 100);
  num3 = num2_remove / 10;
  numbers[2] = num3;
  num4 = num2_remove - (num3 * 10);
  numbers[3] = num4;
}

// 7 segment display
void Display() {
  screenOff(); 
  digitalWrite(latchPin, LOW); 
  shiftOut(dataPin, clockPin,LSBFIRST, table[numbers[count]]); 
  digitalWrite(cathodePins[count], LOW); 
  digitalWrite(latchPin, HIGH); 
  count++; 
  if (count == 4) { 
    count = 0;
  }
}

// 7 segment diisplay
void screenOff() { 
  digitalWrite(cathode4, HIGH);
  digitalWrite(cathode3, HIGH);
  digitalWrite(cathode2, HIGH);
  digitalWrite(cathode1, HIGH);
}

void loop() {
    mesh.update();
    analogWrite(ledPin, lastValue / 4);
    Serial.printf("output value: %d\n", lastValue / 4);

    // Tracking how long connection has been successful
    if(millis() - timeSinceMsgReceived > 60000) {
      timeSinceOn = millis();
      timeSinceMsgReceived = millis();
    }
    long timeOn = (timeSinceMsgReceived - timeSinceOn) / 60000;
    //Serial.printf("this has been on for %d seconds\n", timeOn);

    /// Updating 7 segment display
    timer.update(); 
    if(millis() - timeSinceDisplayUpdate > 50) {
      timer.stop(timer_event); 
      screenOff(); 
      number = timeOn; 
      separate(number);
      timer_event = timer.every(1, Display);
      timeSinceDisplayUpdate = millis();
    }
}
