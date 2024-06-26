// LIBRARIES
#include "painlessMesh.h"      // Mesh WiFi
#include <cppQueue.h>          // Queue
#include <StackArray.h>        // stack (https://github.com/oogre/StackArray)
#include <Wire.h>              // Communicating with I2C for OLED Screen
#include <Adafruit_GFX.h>      // For OLED Screen
#include <Adafruit_SSD1306.h>  // For OLED Screen
#include "ShiftIn.h"
#include <math.h>

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
  int ledPin = 15;
  int submitPin = 14; //number corresponds to the GPIO pin in esp8266 nodemcu pinout
  int LATCH = 16; // (1) (esp8266 was 16(D0) -> huzzah is IO16 (pin4))
  int DATA  = 12; // (9) (esp8266 was 12(D6) -> huzzah is IO12 (pin 6))
  int CLOCK = 14; // (2) (esp8266 was 14(D5) -> huzzah is IO14 (pin 5))
  int clockEnablePin = 15; // (esp8266 was 5(D1) -> huzzah is IO15 (pin10))
  

  // PIN INPUT VALUES
  StackArray <int> numberPadStack;
  int potSensorValue = 0;
  int lastPotSensorValue = 0;
  int binaryToDecSum = 0;
  bool submitClick = false;
  bool ledState = false;

  // TIMES
  unsigned long startTime = millis();
  unsigned long startTimeSub = millis();
  unsigned long timeSinceOn = millis();
  unsigned long timeSinceMsgReceived = millis();
  unsigned long timeSinceUpdate = millis();
  unsigned long lastBlinkTime = millis();

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

  // Num pad
  String numPadStr = "";
  int currStrLen = 0;

// INITIALIZING OBJECTS
cppQueue  sending_queue1(sizeof(String), 20); // Message queue (this is used because wifi only send every 1-2 seconds. That means data could be lost if it is changed more frequently than every 1-2 seconds. Using a queue fixes that.)
cppQueue  sending_queue2(sizeof(String), 20);
cppQueue queueList[numNodesAllowed] = {sending_queue1, sending_queue2}; // Makes list of queues for each node (has max connection of 2 nodes for now)
String receivedMessages[numNodesAllowed] = {"-1", "-1"};
ShiftIn<2> shift;

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
void sendMessage() ; // Prototype so PlatformIO doesn't complain
Task taskSendMessage( TASK_SECOND * .5 , TASK_FOREVER, &sendMessage );


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
    if(frontOfStack != receivedMsg && frontOfStack != "0" && msg != "0" && !((queueList[queueNum]).isEmpty())) {
        (queueList[queueNum]).peek(&frontOfStack);
        SimpleList<uint32_t>::iterator node = connectedNodes.begin();
        int nodeNum = 0;
        while (node != connectedNodes.end()) {
          if(nodeNum == queueNum) {
            Serial.printf("sending %s to %u from stack %d\n", frontOfStack, *node, queueNum);
            mesh.sendSingle(*node, frontOfStack);
            taskSendMessage.setInterval( random( TASK_SECOND * 0.1, TASK_SECOND * 0.2 ));  
          }
          node++;
          nodeNum++;
        }
    } else {
      SimpleList<uint32_t>::iterator node = connectedNodes.begin();
        int nodeNum = 0;
        while (node != connectedNodes.end()) {
          if(nodeNum == queueNum) {
            //Serial.printf("sending %s to %u from stack %d\n", frontOfStack, *node, queueNum);
            String newMsg = "-2";
            mesh.sendSingle(*node, newMsg);
            taskSendMessage.setInterval( random( TASK_SECOND * 1, TASK_SECOND * 1.5 ));  
          }
          node++;
          nodeNum++;
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
  String content;
  int spacing;
  
  if(currentPage >= 1) {
    display.setTextSize(1.4);
    if(currentPage == 1) { // broadcast page
      display.setCursor(30, 35); 
      spacing = 10;
    } else { // every other node page
      display.setCursor(30, 25);
      spacing = 0; 
    }
    display.print("Send: ");
    content = numPadStr;
    display.println(content);

    display.setCursor(25, 35 + spacing); 
    display.print("Recieved: ");
    const char* receivedMessagesConverted;
    if(currentPage == 1) { // broadcast page
      const char* receivedMessagesConverted = receivedMessages[0].c_str();
      content = String(atoi(receivedMessagesConverted));
    } else { // every other node page
      const char* receivedMessagesConverted = receivedMessages[currentPage - 2].c_str();
      content = String(atoi(receivedMessagesConverted));
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

unsigned long blinkTempo(int tempo, unsigned long lastBlinkTime) {
  unsigned long newBlinkTime = lastBlinkTime;
  //Serial.printf("millis() - tempo/1024 * 1000 > lastBlinkTime: %d - %d > %d\n", millis(), tempo, lastBlinkTime);
  float bpm = (60000.0 / tempo) * 4;
  if (millis() * 1000 - bpm * 1000 > lastBlinkTime * 1000) {
    Serial.printf("ledState %d\n", ledState);
    digitalWrite(ledPin, ledState);
    ledState = !ledState;
    newBlinkTime = millis();
  }

  return newBlinkTime;
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
  shift.begin(LATCH, clockEnablePin, DATA, CLOCK);
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

  int numpadNumber = 0;
  if(shift.update()) {
    numpadNumber = displayValues();
  }

  switch(numpadNumber) {
    case 24576: // keypad number 0 equates to 0
      addNumToStr(0);
      break;
    case 16: // keypad number 1 equates to 1
      addNumToStr(1);
      break;
    case 32: // keypad number 2 equates to 2
      addNumToStr(2);
      break;
    case 64: // keypad number 3 equates to 3
      addNumToStr(3);
      break;
    case 128: // keypad number 4 equates to 4
      addNumToStr(4);
      break;
    case 8: // keypad number 5 equates to 5
      addNumToStr(5);
      break;
    case 4: // keypad number 6 equates to 6
      addNumToStr(6);
      break;
    case 2: // keypad number 7 equates to #7
      addNumToStr(7);
      break;
    case 1: // keypad number 8 equates to #8
      addNumToStr(8);
      break;
    //case 4096: // keypad number 9 equates to #9
      //addNumToStr(9);
      //break;
    case 1024: // keypad number 10 equates to delete
      Serial.printf("DELETE\n");
      numPadStr = numPadStr.substring(0, --currStrLen);
      break;
    case 16384: // keypad number 11 equates to LEFT
      // Left code here
      Serial.printf("LEFT\n");
      if(currentPage > 0) {
        currentPage--;
      }
      break;
    case 32768: // keypad number 12 equates to RIGHT
      // right code here
      Serial.printf("RIGHT\n");
      if(currentPage < numPages) {
        currentPage++;
      }
      break;
    case 4096: // keypad number 13 equates to SUBMIT
      // submit code here
      Serial.printf("SUBMIT\n");
      msg = numPadStr;
      for(int queueNum = 0; queueNum < numNodesAllowed; queueNum++) {
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
        } else {
          printf("didn't add to stack: %d %d %d %d %d %d\n", (queueNum == currentPage - 2 || currentPage == 1), frontOfStack != msg, msg != receivedMsg, frontOfStack != "0", msg != "0", !((queueList[queueNum]).isFull()));
        }
      }
      break;
  }

  Serial.println("Current num pad string: " + numPadStr);

  // Updating Display
  initializeDisplay();
  updateDisplayContent();
  display.display();
}
