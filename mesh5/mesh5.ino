// THIS IS NODE 3944100465

// LIBRARIES
#include "painlessMesh.h" // For mesh wifi

// CONSTANTS
#define   MESH_PREFIX     "IoTHub"     // Mesh WiFI name
#define   MESH_PASSWORD   "IOAIHTHG"   // Mesh password
#define   MESH_PORT       5555         // Mesh port

// GLOBAL VARIABLES
  // PINS
  int ledPin = 13;

  // TIMES
  unsigned long timeSinceOn = millis();
  unsigned long timeSinceUpdate = millis();
  unsigned long timeSinceMsgReceived = millis();
  unsigned long timeSinceDisplayUpdate = millis();

  // WiFi COMMUNICATION MESSAGE
  int lastValue = 0;
  String receivedMsg;

// INITIALIZING OBJECTS
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
void sendMessage() ; // Prototype so PlatformIO doesn't complain
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

  // Tells nodes that there is a root and to connect to it
  mesh.setContainsRoot();

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
}

void setup() {
  Serial.begin(115200);
  
  pinMode(ledPin, OUTPUT);
  
  setUpMesh();
}

// Send message to root
void sendMessage() {
  String msg = "Callback:" + receivedMsg;
  uint32_t rootId = getRootId(mesh.asNodeTree());
  Serial.printf("sending %s to %u\n", msg, rootId);
  mesh.sendSingle(rootId, msg);
  taskSendMessage.setInterval( random( TASK_SECOND * 0.5, TASK_SECOND * 1 ));
}

// Recieve message
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

uint32_t getRootId(painlessmesh::protocol::NodeTree nodeTree) {
  if (nodeTree.root) return nodeTree.nodeId;
  for (auto&& s : nodeTree.subs) {
    auto id = getRootId(s);
    if (id != 0) return id;
  }
  return 0;
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

void loop() {
    mesh.update();
    analogWrite(ledPin, lastValue / 4);
    //Serial.printf("output value: %d\n", lastValue / 4);
}
