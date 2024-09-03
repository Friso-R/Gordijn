#include <Arduino.h>
#include <BlockNot.h>
#include <time.h>

#include "WiFiSetup.h"
#include "broker.h"
#include "suntime.h"
#include "StepMotor.h"

WiFiSetup wifi;
Broker    broker;
SunTime   sunTime;
StepMotor stepMotor;

BlockNot   t5(5, SECONDS);
BlockNot   t60(60, SECONDS);

bool circadianMode = 1;
bool scheduleMode;
int timeUp, timeDown;
int sunrise, sunset;

void open_curtain_partly(String messageTemp);
int schedule(String messageTemp);
void check_schedule();
void check_sunTimes();
void sunLoop();
void callback(String topic, byte* message, unsigned int length);

void setup() {
  Serial.begin(9600);
  
  wifi.connect();
  broker.begin();
  button.begin();
  sunTime.setup();
  stepMotor.setup();

  broker.publish("status", "online");
}

void loop() {
  broker.update();

  if (stepMotor.idle()){
    if(t60.TRIGGERED){
      if (scheduleMode) check_schedule();
      if (circadianMode) sunLoop();
    }
  }
  else {
    stepMotor.update();
    int steps = stepMotor.stepsTaken;
    if (steps % 950 == 0)
      broker.publish("progress/get", String(steps/950));
  }
}

// This function is executed when some device publishes a message to a topic that the ESP32 is subscribed to
void callback(String topic, byte* message, unsigned int length) {
  String messageTemp;
  
  for (int i = 0; i < length; i++)  
    messageTemp += (char)message[i];

  if(topic == "infob3it/student033/gordijn"){
    if(messageTemp == "start")   stepMotor.start();
    if(messageTemp == "reverse") stepMotor.reverse();
    if(messageTemp == "up")      stepMotor.roll(LOW);
    if(messageTemp == "down")    stepMotor.roll(HIGH);
  }
  if(topic == "infob3it/student033/mode/circadian") circadianMode = messageTemp.toInt();  
  if(topic == "infob3it/student033/mode/schedule")  scheduleMode = messageTemp.toInt();
  if(topic == "infob3it/student033/schedule/up")    timeUp = schedule(messageTemp);
  if(topic == "infob3it/student033/schedule/down")  timeUp = schedule(messageTemp);
  if(topic == "infob3it/student033/progress/set")   open_curtain_partly(messageTemp);
}

int schedule(String messageTemp) {
  int h, m, timeMin;
  sscanf(messageTemp.c_str(), "%d:%d:%d", &h, &m);

  timeMin = h*60 + m;
  return timeMin;
}

void check_schedule(){
  if(sunTime.check(timeUp))
    stepMotor.roll(LOW);
  if(sunTime.check(timeDown))
    stepMotor.roll(HIGH);
}

void check_sunTimes(){
  if(sunTime.check(sunTime.sunrise))
    stepMotor.roll(LOW);
  if(sunTime.check(sunTime.sunset))
    stepMotor.roll(HIGH);
}

void sunLoop(){
  sunTime.loop();
  check_sunTimes();

    if (sunrise != sunTime.sunrise){
      sunrise = sunTime.sunrise;
      broker.publish("sunrise", String(sunrise));
    }
    if (sunset != sunTime.sunset){
      sunset = sunTime.sunset;
      broker.publish("sunset", String(sunset));
    }
}

void open_curtain_partly(String messageTemp){
  int progress;
  sscanf(messageTemp.c_str(), "%d", &progress);
  stepMotor.open_partially(progress);
}
