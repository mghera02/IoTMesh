// THIS IS NODE 2741661055
#include "painlessMesh.h" // For mesh wifi

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
String receivedMsg;
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * .1 , TASK_FOREVER, &sendMessage );

int outPin = 2;
int connectOutPin = 5;
int ledPin = 16;
int lastValue = 0;
unsigned long timeSinceOn = millis();
unsigned long lastTimeSinceReconnectAttempt = millis();
long timeSinceUpdate = millis();
unsigned long timeSinceMsgReceived = millis();

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

  setUpMesh();
}

void sendMessage() {
  String msg = "Callback:" + receivedMsg;
  //msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 0, TASK_SECOND * .1 ));
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  receivedMsg = msg.c_str();
  timeSinceMsgReceived = millis();
  //analogWrite(16, (atoi(msg.c_str()))/10);
  if(atoi(msg.c_str()) != 0) {
    if(atoi(msg.c_str()) == -1) {
      lastValue = 0;  
    } else {
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

void loop() {
    if(millis() - timeSinceUpdate > 50) {
      mesh.update();
      timeSinceUpdate = millis(); 
    }
     
    Serial.printf("output value: %d\n", lastValue / 4);
    analogWrite(outPin, lastValue / 5);
    analogWrite(ledPin, lastValue / 4);
    //yield();


    if(millis() - timeSinceMsgReceived < 1500) {
      Serial.printf("Connected to child\n");
      timeSinceOn = millis(); 
      analogWrite(connectOutPin, 250);
    } else {
      Serial.printf("Not connected to child\n");
      if(millis() - timeSinceOn > 10000) {
        mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
      }
      analogWrite(connectOutPin, 0);
    }
}
