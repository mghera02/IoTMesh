//Current Branch:
//this branch is for Bree to make the submit button, 3/1/23
// Make list for queue
// Fill list

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
  int submitPin = 14; //number corresponds to the GPIO pin in esp8266 nodemcu pinout
  

  // PIN INPUT VALUES
  int potSensorValue = 0;
  int lastPotSensorValue = 0;
  bool submitClick = false;

  // TIMES
  unsigned long startTime = millis();
  unsigned long startTimeSub = millis();
  unsigned long timeSinceOn = millis();
  unsigned long timeSinceMsgReceived = millis();
  unsigned long timeSinceUpdate = millis();

  // WiFi COMMUNICATION MESSAGE
  SimpleList<uint32_t> connectedNodes;
  String msg;
  String receivedMsg;
  String receivedSubMsg;
  String frontOfStack = "";
  const int numNodesAllowed = 2;

  // OLED DISPLAY
  int currentPage = 0;
  int numPages = numNodesAllowed + 1;
  int displayContent = -1;

// INITIALIZING OBJECTS
cppQueue  sending_queue1(sizeof(String), 20); // Message queue (this is used because wifi only send every 1-2 seconds. That means data could be lost if it is changed more frequently than every 1-2 seconds. Using a queue fixes that.)
cppQueue  sending_queue2(sizeof(String), 20);
cppQueue queueList[numNodesAllowed] = {sending_queue1, sending_queue2}; // Makes list of queues for each node (has max connection of 2 nodes for now)
String receivedMessages[numNodesAllowed] = {"-1", "-1"};

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
void sendMessage() ; // Prototype so PlatformIO doesn't complain
Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );


/*
 * this is where I (Bree) am updating the current mesh file, sendMessage
 */
void sendMessage() {
  for(int queueNum = 0; queueNum < numNodesAllowed; queueNum++) {
    // Popping Queue when hits size limit
    if((queueList[queueNum]).isFull()) {
      Serial.printf("queue is full. popping\n");
      (queueList[queueNum]).pop(&msg); 
    }
  
    (queueList[queueNum]).peek(&frontOfStack);
    Serial.printf("frontOfStack %s. receivedMsg %s\n", frontOfStack, receivedMsg);
  
    // Send message if 
      // 1. received message is not the same as the front of the stack 
      // 2. neither front of stack or message is 0
      // 3. stack isnt empty
    // else
      // send -2 and pop if queue isnt empty

    if(frontOfStack != receivedMsg && frontOfStack != "0" && msg != "0") { // zero can't be sent as a message, essentially no message
      if(!((queueList[queueNum]).isEmpty())) { 
        (queueList[queueNum]).peek(&frontOfStack);
        SimpleList<uint32_t>::iterator node = connectedNodes.begin(); //searches through connectedNodes array, holds NodeIds
        //begin gets a pointer for the beginning of the array connectedNodes
        int nodeNum = 0; //an index (where you are in node), where node is a pointer 
        while (node != connectedNodes.end()) {
          if(nodeNum == queueNum) { //the index (Num == Index of that array, not the node ID)
            Serial.printf("sending %s to %u from stack %d\n", frontOfStack, *node, queueNum); //*node is that pointer for that value in array, = nodeID
            mesh.sendSingle(*node, frontOfStack); //sends the value in the frontOfStack (front of queNum array) to the node pointer (to nodeID), frontOfStack message sent to nodeID
          }
          node++;
          nodeNum++;
        }
        taskSendMessage.setInterval( random( TASK_SECOND * 0.1, TASK_SECOND * 0.2 )); 
      } else {
      /*
       * in the bottom else statement where it broadcasts, change it to no longer broadcasting
       * instead send individual messages to each node, like it does in body of if statement
       * things to note:
       *      the whole function is already part of a for loop going through each node but I'll still need to make another loop
       *      because they go through different parts of the nodes (queues vs. Node ID's)
       * 
       * do that in the else statement after addition of each node individually
        */
        //will have the exact same code as the else statement below (except for an included if in the else beneath)
        msg = "-2"; // -2 symbolizes nothing or no message (-1 is zero)
        SimpleList<uint32_t>::iterator node = connectedNodes.begin(); //searches through connectedNodes array, holds NodeIds
        //begin gets a pointer for the beginning of the array connectedNodes
        int nodeNum = 0; //an index (where you are in node), where node is a pointer 
        while (node != connectedNodes.end()) {
          if(nodeNum == queueNum) { //the index (Num == Index of that array, not the node ID)
            Serial.printf("sending %s to %u from stack %d\n", frontOfStack, *node, queueNum); //*node is that pointer for that value in array, = nodeID
            mesh.sendSingle(*node, frontOfStack); //sends the value in the frontOfStack (front of queNum array) to the node pointer (to nodeID), frontOfStack message sent to nodeID
          }
          node++;
          nodeNum++;
        }
        taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 1.5 )); 
      }
    } else {
      msg = "-2";
      if(!(queueList[queueNum]).isEmpty()){
        (queueList[queueNum]).pop(&msg);
      }
      SimpleList<uint32_t>::iterator node = connectedNodes.begin(); //searches through connectedNodes array, holds NodeIds
      //begin gets a pointer for the beginning of the array connectedNodes
      int nodeNum = 0; //an index (where you are in node), where node is a pointer 
      while (node != connectedNodes.end()) {
        if(nodeNum == queueNum) { //the index (Num == Index of that array, not the node ID)
          Serial.printf("sending %s to %u from stack %d\n", frontOfStack, *node, queueNum); //*node is that pointer for that value in array, = nodeID
          mesh.sendSingle(*node, frontOfStack); //sends the value in the frontOfStack (front of queNum array) to the node pointer (to nodeID), frontOfStack message sent to nodeID
        }
        node++;
        nodeNum++;
      }
      taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 1.5 )); 
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
      if(frontOfStack != "-2") { // Keeping track of last message sent/received for each node
        receivedMessages[queueNum] = frontOfStack;
      }
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
void updateDisplayContent() {
  if(currentPage >= 1) {
    display.setTextSize(1.4);
    if(currentPage == 1) { // broadcast page
      display.setCursor(30, 35); 
    } else { // every other node page
      display.setCursor(30, 25); 
    }
    display.print("Power: ");
    const char* receivedMessagesConverted;
    String content;
    if(currentPage == 1) { // broadcast page
      const char* receivedMessagesConverted = receivedMessages[0].c_str();
      content = String(atoi(receivedMessagesConverted) * 100 / 1024);
    } else { // every other node page
      const char* receivedMessagesConverted = receivedMessages[currentPage - 2].c_str();
      content = String(atoi(receivedMessagesConverted) * 100 / 1024);
    }
    display.print(content);
    display.println("%"); 
  }
}

// Initializing OLED Display
void initializeDisplay() {
  // Initializing the styling of the page
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(40, 55);
  display.print("Page ");
  display.println(currentPage);
  display.setCursor(0, 0);
  
  if(currentPage == 0) {// First Page
    display.setCursor(20, 20);
    display.println("Welcome to the");
    display.setCursor(40, 30);
    display.println("KTT remote");
    display.setCursor(0, 0);
  } else if(currentPage == 1) { // Back Button Header for Broadcasting Page
    display.print("B1 to Welcome"); 
    display.println(" <-");
  } else if(currentPage == 2) { // Back Button for first node
    display.print("B1 to Broadcasting"); 
    display.println(" <-");
  } else { // Back Button for all other nodes
    display.print("B1 to Node ");
    display.print(currentPage - 1); 
    display.println(" <-");
  }
  
  if(currentPage == 0) { // Next Page header for Welcome
    display.print("B2 to Broadcasting"); 
    display.println(" ->");
  } else if(currentPage < numPages) { // Next Page header for everyother page except last
    display.print("B2 to Node ");
    display.print(currentPage); 
    display.println(" ->");
  }

  // Broadcast Page header
  if(currentPage == 1) {
     display.setTextSize(1.4);
     display.setCursor(10, 20);
     display.println("Broadcast to all"); 
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
  // This is for testing connections
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


    //NOTE FOR BREE
  //understand the code below, will implement the submit button in a similar way
  //sends the current message (top of stack) if the button is pressed
  //sends -2 if not pressed
  //store if button was pressed, which node, and if broadcast
  //questions:
  //how do i reset the submit state to be 0 after pressed???
  //how does digitalRead actually work... can i get it to read 0 vs. 1?
  int submitState = digitalRead(submitPin); // 0 while being pressed, 1 when not
  if(millis() - startTimeSub > 100 && !submitState) {
    printf("Button is clicked\n"); //not 100% necessary but could be helpful for debugging
    submitClick = true;
    startTimeSub = millis();
  } else{
    submitClick = false;
  }

 // Push to stack
  msg = String(potSensorValue);
  if(submitClick == true){
    printf("Submitting message to send\n");
    for(int queueNum = 0; queueNum < numNodesAllowed; queueNum++) {
      // This is for testing purposes.
      /*if(queueNum == 0) {
        msg = "50";
      } else {
        msg = "254";
      }*/
      (queueList[queueNum]).peek(&frontOfStack);
      //printf("frontofstack %s, msg %s\n", frontOfStack, msg);
      
      // put message in stack if 
        // on the broadcast page or the current node page and 
        // the message is not on the front of the stack and 
        // the message is not the last received message and 
        // the frontof the stack doesn't equal 0 and
        // the message doesnt equal 0 and
        // the queue is not full
      if((queueNum == currentPage - 2 || currentPage == 1) && frontOfStack != msg && msg != receivedMsg && frontOfStack != "0" && msg != "0" && !((queueList[queueNum]).isFull())) {
        printf("adding %s to stack %d\n", msg, queueNum);
        (queueList[queueNum]).push(&msg);
      }
    }
  }
  submitClick = false;
  

  // Page Changing for Display
  int buttonState2 = digitalRead(B2Pin);
  if(millis() - startTime > 500 && !buttonState2) {
    if(currentPage < numPages) {
      printf("page right\n");
      currentPage++;
    }
    startTime = millis();
  }
  int buttonState1 = digitalRead(B1Pin);
  if(millis() - startTime > 500 && !buttonState1) {
    if(currentPage > 0) {
      printf("page left\n");
      currentPage--;
    }
    startTime = millis();
  }

  // Updating Display
  initializeDisplay();
  updateDisplayContent();
  display.display();
}
