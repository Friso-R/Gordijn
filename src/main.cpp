#include <Arduino.h>
#include <BlockNot.h>
#include <time.h>

#include "WiFiSetup.h"
#include "broker.h"
#include "LocalTime.h"
#include "StepMotor.h"

WiFiSetup wifi;
Broker    broker;
LocalTime klok;
StepMotor stepMotor;

BlockNot   t5(5, SECONDS);
BlockNot   t60(60, SECONDS);

bool circadianMode = 1;
bool scheduleMode;
int timeUp, timeDown;
int sunrise, sunset;

void monitor();
void run();
int  schedule(String messageTemp);
void sync();
void open_curtain_partly(String messageTemp);
void check_schedule();
void check_sunTimes();
void sunLoop();
void callback(String topic, byte* message, unsigned int length);
String mins_to_time(int t);
void publishProgress(void *parameter);
void CreatePublishTask();

void setup() {
  Serial.begin(9600);
  
  wifi.connect();
  broker.begin();
  button.begin();
  klok.setup();
  stepMotor.setup();
  
  sync();
}

void loop() {
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
  broker.publish("progress/get", String(stepMotor.stepsTaken/950));
  broker.publish("mode/circadian", String(circadianMode));
  broker.publish("mode/schedule", String(scheduleMode));
  broker.publish("sunrise", mins_to_time(klok.sunrise));
  broker.publish("sunset" , mins_to_time(klok.sunset));
}

// This function is executed when some device publishes a message to a topic that the ESP32 is subscribed to
void callback(String topic, byte* message, unsigned int length) {
  String msg;
  
  for (int i = 0; i < length; i++)  
    msg += (char)message[i];

  if(topic == "infob3it/student033/gordijn"){
    if(msg == "start")   stepMotor.start();
    if(msg == "reverse") stepMotor.reverse();
    if(msg == "up")      stepMotor.roll(UP);
    if(msg == "down")    stepMotor.roll(DOWN);
  }
  if(topic == "infob3it/student033/mode/circadian") circadianMode = msg.toInt();  
  if(topic == "infob3it/student033/mode/schedule")  scheduleMode = msg.toInt();
  if(topic == "infob3it/student033/schedule/up")    timeUp = schedule(msg);
  if(topic == "infob3it/student033/schedule/down")  timeDown = schedule(msg);
  if(topic == "infob3it/student033/progress/set")   open_curtain_partly(msg);
  if(topic == "infob3it/student033/status/sync")    sync();
}

int schedule(String messageTemp) {
  int h, m, s, timeMin;
  sscanf(messageTemp.c_str(), "%d:%d:%d", &h, &m, &s);

  timeMin = h*60 + m;
  return timeMin;
}

void check_schedule(){
  if(klok.check(timeUp))
    stepMotor.roll(UP);
  if(klok.check(timeDown))
    stepMotor.roll(DOWN);
}

void check_sunTimes(){
  if(klok.check(klok.sunrise))
    stepMotor.roll(UP);
  if(klok.check(klok.sunset))
    stepMotor.roll(DOWN);
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

void open_curtain_partly(String messageTemp){
  int progress;
  sscanf(messageTemp.c_str(), "%d", &progress);
  stepMotor.open_partially(progress);
}

void publishProgress(void *parameter) {
  while (true) 
  {
    const int steps = stepMotor.stepsTaken;
    if(steps % 950 == 0){
      broker.publish("progress/get", String(steps/950));
      vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    if (stepMotor.idle())
      vTaskDelete(NULL);
  }
}

void CreatePublishTask() {
  xTaskCreatePinnedToCore(
    publishProgress,       // Function to run
    "PublishTask",         // Task name
    5000,                  // Stack size in bytes
    NULL,                  // Parameter to pass
    1,                     // Priority
    NULL,                  // Task handle for external control
    0                      // Core ID (0 = Core 0)
  );
}

