// THIS IS NODE 2741409788

// LIBRARIES
#include "painlessMesh.h"      // Mesh WiFi
#include <cppQueue.h>          // Queue
#include <Wire.h>              // Communicating with I2C for OLED Screen
#include <Adafruit_GFX.h>      // For OLED Screen
#include <Adafruit_SSD1306.h>  // For OLED Screen

// CONSTANTS
#define   MESH_PREFIX     "IoTHub"     // Mesh WiFI name
#define   MESH_PASSWORD   "IOAIHTHG"   // Mesh password
#define   MESH_PORT       5555         // Mesh port
#define   SCREEN_WIDTH 128             // OLED display width, in pixels
#define   SCREEN_HEIGHT 64             // OLED display height, in pixels

// GLOBAL VARIABLES
  // PINS
  int potPin = A0;
  int B2Pin = 0;
  int B1Pin = 2;
  int ledPin = 16;

  // PIN INPUT VALUES
  int potSensorValue = 0;
  int lastPotSensorValue = 0;

  // TIMES
  unsigned long startTime = millis();
  unsigned long timeSinceOn = millis();
  unsigned long timeSinceMsgReceived = millis();
  unsigned long forceRebootTime = millis();
  unsigned long timeSinceUpdate = millis();

  // OLED DISPLAY
  int currentPage = 1;
  int numPages = 3;
  int displayContent = -1;

  // WiFi COMMUNICATION MESSAGE
  String msg;
  String receivedMsg;
  String receivedSubMsg;
  String frontOfStack = "";
  bool connectedStatus = false;

// INITIALIZING OBJECTS
cppQueue  sending_queue(sizeof(String), 20); // Message queue (this is used because wifi only send every 1-2 seconds. That means data could be lost if it is changed more frequently than every 1-2 seconds. Using a queue fixes that.)
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
void sendMessage() ; // Prototype so PlatformIO doesn't complain
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  // Popping Queue when hits size limit
  if(sending_queue.isFull()) {
    Serial.printf("queue is full. popping\n");
    sending_queue.pop(&msg); 
  }

  sending_queue.peek(&frontOfStack);
  Serial.printf("frontOfStack %s. receivedMsg %s\n", frontOfStack, receivedMsg);

  // Send message if 
    // 1. received message is not the same as the front of the stack 
    // 2. neither front of stack or message is 0
    // 3. stack isnt empty
  // else
    // send 0 and pop only if message is not the same as front of the stack
  if(frontOfStack != receivedMsg && frontOfStack != "0" && msg != "0") {
    if(!(sending_queue.isEmpty())) {
      sending_queue.peek(&frontOfStack);
      Serial.printf("sending %s\n", frontOfStack);
      mesh.sendBroadcast( frontOfStack );
      taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 2 )); 
    } else {
      msg = "-2";
      mesh.sendBroadcast( msg );
    }
  } else {
    msg = "-2";
    mesh.sendBroadcast( msg );
    if(!(sending_queue.isEmpty())) {
      sending_queue.pop(&msg);
    }  
  }
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  receivedMsg = msg.c_str();

  // Incoming message will be in the form of: "Callback:xxxxxx". Only want xxxxxx
  if(receivedMsg.indexOf(':') >= -1) {
    receivedSubMsg = receivedMsg.substring(receivedMsg.indexOf(':') + 1);
    displayContent = atoi(receivedSubMsg.c_str());
    receivedMsg = receivedSubMsg;
  }

  // Update message received counter and pop off stack
  timeSinceMsgReceived = millis();
  Serial.printf("substring %s\n", receivedSubMsg);
  sending_queue.peek(&frontOfStack);
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

// Updating OLED Display
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

// Initializing OLED Display
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

  // For OLED Display Setup
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
  // Updating Mesg
  if(millis() - timeSinceUpdate > 50) {
    mesh.update();
    timeSinceUpdate = millis(); 
  }

  // Potentiometer
  potSensorValue = analogRead(potPin);
  if(potSensorValue > lastPotSensorValue + 30 || potSensorValue < lastPotSensorValue - 30) {
    Serial.printf("potSensorValue %d. LastPotSensorValue %d. Changing. \n", potSensorValue, lastPotSensorValue);
    lastPotSensorValue = potSensorValue;
  } else {
    //Serial.printf("potSensorValue %d. LastPotSensorValue %d. Staying the same. \n", potSensorValue, lastPotSensorValue);
    potSensorValue = lastPotSensorValue;
  }
  if(potSensorValue < 100) {
    potSensorValue = -1;
  }
  //Serial.printf("potSensorValue %d \n", potSensorValue);
  analogWrite(ledPin, potSensorValue/10);

  // Push to stack
  msg = String(potSensorValue);
  sending_queue.peek(&frontOfStack);
  printf("frontofstack %s, msg %s\n", frontOfStack, msg);
  if(frontOfStack != msg && msg != receivedMsg && frontOfStack != "0" && msg != "0" && !(sending_queue.isFull())) {
    printf("adding %s to stack\n", msg);
    sending_queue.push(&msg);
  }

  // Page Changing for Display
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

  // Updating Display
  initializeDisplay();
  updateDisplayContent(displayContent, connectedStatus, forceRebootTime);
  display.display();
}
