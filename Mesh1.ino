// Make list for queue
// Fill list

// THIS IS NODE 2741409788

// LIBRARIES
#include "painlessMesh.h"      // Mesh WiFi --> look into this library!! very important
#include <cppQueue.h>          // Queue --> look into this library!! also very important
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
  int potPin = A0; // potentiometer
  int B2Pin = 0;
  int B1Pin = 2;
  int ledPin = 16;

  // PIN INPUT VALUES
  int potSensorValue = 0; //potentiometer
  int lastPotSensorValue = 0;

  // TIMES
  //unsigned = variable must be positive, cannot be negative
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
  SimpleList<uint32_t> connectedNodes;
  String msg;
  String receivedMsg;
  String receivedSubMsg;
  String frontOfStack = "";
  bool connectedStatus = false;
  const int numNodesAllowed = 2;

// INITIALIZING OBJECTS
// objects have their own functions that can be called for them
cppQueue  sending_queue1(sizeof(String), 20); // Message queue (this is used because wifi only send every 1-2 seconds. That means data could be lost if it is changed more frequently than every 1-2 seconds. Using a queue fixes that.)
cppQueue  sending_queue2(sizeof(String), 20); // queue seems to be just kind of like a looped delay timer?
cppQueue queueList[numNodesAllowed] = {sending_queue1, sending_queue2}; // Makes list of queues for each node (has max connection of 2 nodes for now)

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh; 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
void sendMessage() ; // Prototype so PlatformIO doesn't complain
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  for(int queueNum = 0; queueNum < numNodesAllowed; queueNum++) {
    // Popping Queue when hits size limit
    if((queueList[queueNum]).isFull()) {
      Serial.printf("queue is full. popping\n");
      (queueList[queueNum]).pop(&msg); // equivalent to removing a value from the list
    }
  
    (queueList[queueNum]).peek(&frontOfStack); //reads the next character in the stream, doesn't extract
    Serial.printf("frontOfStack %s. receivedMsg %s\n", frontOfStack, receivedMsg); //outputs the current value and message recieved
  
    // Send message if 
      // 1. received message is not the same as the front of the stack 
      // 2. neither front of stack or message is 0
      // 3. stack isnt empty
    // else
      // send -2 and pop if queue isnt empty
      /*is there a reason for -2 specifically?*/
      
    if(frontOfStack != receivedMsg && frontOfStack != "0" && msg != "0") {
      if(!((queueList[queueNum]).isEmpty())) {
        (queueList[queueNum]).peek(&frontOfStack);
        SimpleList<uint32_t>::iterator node = connectedNodes.begin(); // the name after :: is a function in a called library, begin returns iterator pointing to first element in sequence
        int nodeNum = 0;
        while (node != connectedNodes.end()) { // end returns iterator pointing past the end of a sequence
          if(nodeNum == queueNum) {
            Serial.printf("sending %s to %u from stack %d\n", frontOfStack, *node, queueNum);
            mesh.sendSingle(*node, frontOfStack); //sendSingle 
          }
          node++;
          nodeNum++;
        }
        taskSendMessage.setInterval( random( TASK_SECOND * 0.5, TASK_SECOND * 1 )); 
      } else {
        msg = "-2";
        mesh.sendBroadcast( msg );
      }
    } else {
      msg = "-2";
      mesh.sendBroadcast( msg );
      if(!((queueList[queueNum]).isEmpty())) {
        (queueList[queueNum]).pop(&msg);
      }  
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

  for(int queueNum = 0; queueNum < numNodesAllowed; queueNum++) {
    // Update message received counter and pop off stack
    timeSinceMsgReceived = millis();
    Serial.printf("substring %s\n", receivedSubMsg);
    (queueList[queueNum]).peek(&frontOfStack);
    if(frontOfStack == receivedSubMsg) {
      Serial.printf("Matches top of stack. popping\n");
      (queueList[queueNum]).pop(&msg); 
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

  // Makes this node the root
  mesh.setRoot();
  
  // Tells nodes that there is a root and to connect to it
  mesh.setContainsRoot();

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
  // Updating Mesh
  if(millis() - timeSinceUpdate > 50) {
    mesh.update();
    timeSinceUpdate = millis(); 
  }

  // Get list of nodes in mesh
  connectedNodes = mesh.getNodeList();
  /*Serial.printf("\nConnection list (%d):", connectedNodes.size());
  SimpleList<uint32_t>::iterator node = connectedNodes.begin();
  while (node != connectedNodes.end()) {
    //Serial.printf(" %u", *node);
    node++;
  }*/

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
  for(int queueNum = 0; queueNum < numNodesAllowed; queueNum++) {
    // This is for testing purposes.
    /*if(queueNum == 0) {
      msg = "50";
    } else {
      msg = "254";
    }*/
    (queueList[queueNum]).peek(&frontOfStack);
    //printf("frontofstack %s, msg %s\n", frontOfStack, msg);
    if(frontOfStack != msg && msg != receivedMsg && frontOfStack != "0" && msg != "0" && !((queueList[queueNum]).isFull())) {
      printf("adding %s to stack %d\n", msg, queueNum);
      (queueList[queueNum]).push(&msg);
    }
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
