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

void setup() {
  Serial.begin(9600);
  
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
  broker.publish("progress/get", String(stepMotor.stepsTaken/950));
  broker.publish("mode/circadian", String(circadianMode));
  broker.publish("mode/schedule", String(scheduleMode));
  broker.publish("sunrise", mins_to_time(klok.sunrise));
  broker.publish("sunset" , mins_to_time(klok.sunset));
  */
}

// 1. Fix the MQTT callbacks
void callback(String topic, byte* message, unsigned int length) {
  topic = topic.substring(8);
  String msg;
  
  for (int i = 0; i < length; i++)  
    msg += (char)message[i];

  if(topic == "action"){
    if(msg == "start")   stepMotor.start();
    if(msg == "reverse") stepMotor.reverse();
    // Fixed mapping: "up" = OPEN, "down" = CLOSE
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

// 2. Fix the Schedule logic
void check_schedule(){
  if(klok.check(timeUp))
    stepMotor.roll(CURTAIN_OPEN);  // 10:00 AM -> Open the curtain
  if(klok.check(timeDown))
    stepMotor.roll(CURTAIN_CLOSE); // 16:00 PM -> Close the curtain
}

// 3. Fix the Sun logic
void check_sunTimes(){
  if(klok.check(klok.sunrise))
    stepMotor.roll(CURTAIN_OPEN);  // Sunrise -> Open the curtain
  if(klok.check(klok.sunset))
    stepMotor.roll(CURTAIN_CLOSE); // Sunset -> Close the curtain
}

String mins_to_time(int t) {
  char timeChars[6];
  sprintf(timeChars, "%d:%02d", t / 60, t % 60);
  return String(timeChars);
}

void sunLoop(){
  check_sunTimes();

  if (sunrise != klok.sunrise){
    sunrise = klok.sunrise;
    broker.publish("sunrise", mins_to_time(sunrise));
  }
  if (sunset != klok.sunset){
    sunset = klok.sunset;
    broker.publish("sunset", mins_to_time(sunset));
  }
}

int schedule(String messageTemp) {
  int h = 0, m = 0, s = 0;
  
  // sscanf returns how many items it successfully matched. 
  // We check for >= 2 so it works even if MQTT sends "10:30" without seconds.
  if (sscanf(messageTemp.c_str(), "%d:%d:%d", &h, &m, &s) >= 2) {
    return h * 60 + m;
  }
  
  return -1; // Fallback to avoid random times if the format is completely wrong
}

void open_curtain_partly(String messageTemp){
  int progress;
  sscanf(messageTemp.c_str(), "%d", &progress);
  stepMotor.open_partially(progress);
}

void publishProgress(void *parameter) {
  /*
  int stepSize = stepMotor.numSteps / 100; 
  int lastPublished = -1;

  while (!stepMotor.idle()) {
    int currentSegment = stepMotor.stepsTaken / stepSize;

    if (currentSegment != lastPublished) {
      broker.publish("progress/get", String(currentSegment));
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
  
  //Serial.print("progressTaskHandle: ");
  //Serial.println((uint32_t)progressTaskHandle, HEX);  // Print as a hexadecimal memory address

  if (progressTaskHandle == NULL) {
  xTaskCreatePinnedToCore(
    publishProgress,       // Function to run
    "PublishTask",         // Task name
    4096,                  // Stack size in bytes
    NULL,                  // Parameter to pass
    2,                   // Priority
    &progressTaskHandle,   // Task handle for external control
    0                      // Core ID (0 = Core 0)
  );
}
}

