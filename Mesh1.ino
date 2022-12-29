// THIS IS NODE 2741409788
#include "painlessMesh.h"
#include <cppQueue.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int currentPage = 1;
int numPages = 3;

int potPin = A0;
int B2Pin = 0;
int B1Pin = 2;
int ledPin = 16;
int potSensorValue = 0;
int lastPotSensorValue = 0;

unsigned long startTime = millis();
unsigned long timeSinceOn = millis();
unsigned long timeSinceMsgReceived = millis();
unsigned long forceRebootTime = millis();
unsigned long timeSinceUpdate = millis();

long lastUpdateTime = millis();
String msg;
String receivedMsg;
String receivedSubMsg;
String frontOfStack = "";
int displayContent = -1;
bool connectedStatus = false;
cppQueue  sending_queue(sizeof(String), 40);

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * .1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  sending_queue.peek(&frontOfStack);
  if(sending_queue.isFull()) {
    Serial.printf("queue is full. popping\n");
    sending_queue.pop(&msg); 
  }
  msg = String(potSensorValue);
  sending_queue.push(&msg);
  
  if(!(sending_queue.isEmpty())) {
    sending_queue.peek(&msg);
    Serial.printf("sending %s\n", msg);
    mesh.sendBroadcast( msg );
    taskSendMessage.setInterval( random( TASK_SECOND * 0, TASK_SECOND * .1 )); 
  }
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  receivedMsg = msg.c_str();
  
  if(receivedMsg.indexOf(':') > -1) {
    receivedSubMsg = receivedMsg.substring(receivedMsg.indexOf(':') + 1);
    displayContent = atoi(receivedSubMsg.c_str());
  }

  timeSinceMsgReceived = millis();
  Serial.printf("substring %s\n", receivedSubMsg);
  if(frontOfStack == receivedSubMsg) {
    Serial.printf("Matches top of stack. popping\n");
    sending_queue.pop(&msg); 
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

void updateDisplayContent(int content, bool connectedStatus, unsigned long forceRebootTime) {
  display.setTextSize(1.4);
  display.setCursor(30, 20);
  display.print("Power: ");
  content /= 10;
  display.print(content);
  display.println("%");

  if(connectedStatus) {
    display.setCursor(30, 30);
    display.println("Connected");
  } else {
    display.setCursor(20, 30);
    display.println("Not connected");
    display.setCursor(20, 40);
    display.print("Reboot in ");
    display.print(forceRebootTime);
    display.println(" sec");
  }
}

void initializeDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(40, 55);
  display.print("Page ");
  display.println(currentPage);
  display.setCursor(0, 0);
  if(currentPage != 1) {
    display.print("B1 to go Node ");
    display.print(currentPage - 1); 
    display.println(" <- ");
  }
  if(currentPage < numPages) {
    display.print("B2 to go Node ");
    display.print(currentPage); 
    display.println(" -> ");
  }
}

void setUpMesh() {
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  //mesh.setDebugMsgTypes( CONNECTION | SYNC );

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.setRoot();

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
}

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(1000);
  display.clearDisplay();

  setUpMesh();

  pinMode(B1Pin, INPUT);
  pinMode(B2Pin, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  initializeDisplay();

  if(millis() - timeSinceUpdate > 50) {
    mesh.update();
    timeSinceUpdate = millis(); 
  }
  
  if(millis() - timeSinceMsgReceived < 1500) {
    //Serial.printf("Connected to child\n");
    connectedStatus = true;
    timeSinceOn = millis(); 
  } else {
    //Serial.printf("Not connected to child\n");
    if(millis() - timeSinceOn > 30000) {
      mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
    }
    forceRebootTime = 30 - ((millis() - timeSinceOn)/1000);
    if(forceRebootTime < 28) {
      connectedStatus = false; 
    }
  }
  
  potSensorValue = analogRead(potPin);
  if(potSensorValue > lastPotSensorValue + 2 || potSensorValue < lastPotSensorValue - 2) {
    lastPotSensorValue = potSensorValue;
  } else {
    potSensorValue = lastPotSensorValue;
  }
  analogWrite(ledPin, potSensorValue/10);
  if(potSensorValue < 10) {
    potSensorValue = -1;
  }

  int buttonState2 = digitalRead(B2Pin);
  if(millis() - startTime > 500 && !buttonState2) {
    if(currentPage < numPages) {
      currentPage++;
    }
    startTime = millis();
  }

  int buttonState1 = digitalRead(B1Pin);
  if(millis() - startTime > 500 && !buttonState1) {
    if(currentPage > 1) {
      currentPage--;
    }
    startTime = millis();
  }

  updateDisplayContent(displayContent, connectedStatus, forceRebootTime);
  display.display();
}
