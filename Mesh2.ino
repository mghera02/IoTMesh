#include "painlessMesh.h" // For mesh wifi
#include <RBDdimmer.h> // For AC dimmer

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555
#define USE_SERIAL  Serial // For AC Dimmer
#define acdPin 4 // For AC Dimmer PSM Pin
#define zeroCrossPin  5 // For AC Dimmer ZC Pin

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
String receivedMsg;
void sendMessage() ; // Prototype so PlatformIO doesn't complain
dimmerLamp acd(acdPin,zeroCrossPin);
Task taskSendMessage( TASK_SECOND * .1 , TASK_FOREVER, &sendMessage );

void setup() {
  Serial.begin(115200);

//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
  
  acd.begin(NORMAL_MODE, ON);
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
  //analogWrite(16, (atoi(msg.c_str()))/10);
  changeDimmer(atoi(msg.c_str()));
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

void changeDimmer(int x){
  x = (x / 30) + 33;
  Serial.printf("Adjusted power: %d\n", x);
  acd.setPower(x);
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
  delay(10);
}
