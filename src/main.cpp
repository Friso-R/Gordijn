#include "Files.h"

WiFiSetup wifi;
Broker    broker;
LocalTime klok;
StepMotor stepMotor;
EasyButton button(BUTTON_PIN);
BlockNot t60 (60, SECONDS);

bool circadianMode;
bool scheduleMode = 1;
int timeUp   = 10 * 60; 
int timeDown = 16 * 60;
int sunrise, sunset;

TaskHandle_t progressTaskHandle = NULL;

// Helper to format minutes into a standard C-string buffer
void mins_to_time(int t, char* buffer, size_t maxLen) {
  snprintf(buffer, maxLen, "%d:%02d", t / 60, t % 60);
}

void check_schedule();
void sunLoop();
void sync();
void monitor();

void setup() {
  Serial.begin(115200);
  
  wifi.setup();
  broker.begin();
  klok.setup();
  stepMotor.setup();
  button.begin();
  button.onPressedFor(1000, []() { stepMotor.reverse(); });
  button.onPressed   (      []() { stepMotor.start();   });
  
  sync();
}

void loop() {
  wifi.handleTime();
  broker.handleConnection();
  broker.update();
  button.read();
  stepMotor.idle() ? monitor() : stepMotor.update();
}

void monitor() {
  if(t60.TRIGGERED){
    klok.update();
    if (scheduleMode) check_schedule();
    if (circadianMode) sunLoop();
  }
}

void sync(){
  broker.publish("status", "online");
  
  /*
  char msgBuffer[16];
  
  snprintf(msgBuffer, sizeof(msgBuffer), "%d", stepMotor.stepsTaken/950);
  broker.publish("progress/get", msgBuffer);
  
  snprintf(msgBuffer, sizeof(msgBuffer), "%d", circadianMode);
  broker.publish("mode/circadian", msgBuffer);
  
  snprintf(msgBuffer, sizeof(msgBuffer), "%d", scheduleMode);
  broker.publish("mode/schedule", msgBuffer);

  char timeBuffer[16];
  mins_to_time(klok.sunrise, timeBuffer, sizeof(timeBuffer));
  broker.publish("sunrise", timeBuffer);
  
  mins_to_time(klok.sunset, timeBuffer, sizeof(timeBuffer));
  broker.publish("sunset" , timeBuffer);
  */
}

int schedule(String messageTemp) {
  int h = 0, m = 0, s = 0;
  
  // sscanf returns how many items it successfully matched. 
  if (sscanf(messageTemp.c_str(), "%d:%d:%d", &h, &m, &s) >= 2) {
    return h * 60 + m;
  }
  
  return -1; 
}

void open_curtain_partly(String messageTemp){
  int progress;
  sscanf(messageTemp.c_str(), "%d", &progress);
  stepMotor.open_partially(progress);
}

void callback(String topic, byte* message, unsigned int length) {
  topic = topic.substring(8);
  
  // Allocate memory once to prevent heap fragmentation on incoming messages
  char msgBuffer[length + 1];
  memcpy(msgBuffer, message, length);
  msgBuffer[length] = '\0';
  String msg = String(msgBuffer);

  if(topic == "action"){
    if(msg == "start")   stepMotor.start();
    if(msg == "reverse") stepMotor.reverse();
    if(msg == "up")      stepMotor.roll(CURTAIN_OPEN);
    if(msg == "down")    stepMotor.roll(CURTAIN_CLOSE);
  } 
  if(topic == "mode/circadian"){ circadianMode = msg.toInt(); sunLoop(); }
  if(topic == "mode/schedule")   scheduleMode = msg.toInt();
  if(topic == "schedule/up")     timeUp = schedule(msg);
  if(topic == "schedule/down")   timeDown = schedule(msg);
  if(topic == "progress/set")    open_curtain_partly(msg);
  if(topic == "status/sync")     sync();
}

void check_schedule(){
  if(klok.check(timeUp))
    stepMotor.roll(CURTAIN_OPEN);  
  if(klok.check(timeDown))
    stepMotor.roll(CURTAIN_CLOSE); 
}

void check_sunTimes(){
  if(klok.check(klok.sunrise))
    stepMotor.roll(CURTAIN_OPEN);  
  if(klok.check(klok.sunset))
    stepMotor.roll(CURTAIN_CLOSE); 
}

void sunLoop(){
  check_sunTimes();
  char timeBuffer[16];

  if (sunrise != klok.sunrise){
    sunrise = klok.sunrise;
    mins_to_time(sunrise, timeBuffer, sizeof(timeBuffer));
    broker.publish("sunrise", timeBuffer);
  }
  if (sunset != klok.sunset){
    sunset = klok.sunset;
    mins_to_time(sunset, timeBuffer, sizeof(timeBuffer));
    broker.publish("sunset", timeBuffer);
  }
}

void publishProgress(void *parameter) {
  /*
  int stepSize = stepMotor.numSteps / 100; 
  int lastPublished = -1;
  char msgBuffer[16];

  while (!stepMotor.idle()) {
    int currentSegment = stepMotor.stepsTaken / stepSize;

    if (currentSegment != lastPublished) {
      snprintf(msgBuffer, sizeof(msgBuffer), "%d", currentSegment);
      broker.publish("progress/get", msgBuffer);
      lastPublished = currentSegment;
    }

    // CRUCIAAL: Altijd een kleine delay buiten de if-statement!
    vTaskDelay(10 / portTICK_PERIOD_MS); 
  }
  */
  progressTaskHandle = NULL;
  vTaskDelete(NULL); 
}

void CreatePublishTask() {
  if (progressTaskHandle == NULL) {
    xTaskCreatePinnedToCore(
      publishProgress,       
      "PublishTask",         
      4096,                  
      NULL,                  
      2,                   
      &progressTaskHandle,   
      0                      
    );
  }
}