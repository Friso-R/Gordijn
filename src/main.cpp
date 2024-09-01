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

int TimeUp, TimeDown;

void schedule(String messageTemp);
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
      sunTime.loop();
      Serial.println(sunTime.currentTimeMinutes);
      if(sunTime.check(sunTime.sunrise))
        stepMotor.roll_up();
      if(sunTime.check(sunTime.sunset))
        stepMotor.roll_down();
      if(sunTime.check(TimeUp))
        stepMotor.roll_up();
      if(sunTime.check(TimeDown))
        stepMotor.roll_down();
    }
  }
  else {
    stepMotor.update();
    int steps = stepMotor.stepsTaken ;
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
    if(messageTemp == "start")
      stepMotor.start();
    if(messageTemp == "reverse")
      stepMotor.reverse();
    if(messageTemp == "up")
      stepMotor.roll_up();
    if(messageTemp == "down")
      stepMotor.roll_down();
  }
  if(topic == "infob3it/student033/schedule")
    schedule(messageTemp);
  if(topic == "infob3it/student033/progress/set") {
    int progress;
    sscanf(messageTemp.c_str(), "%d", &progress);
    stepMotor.open_partially(progress);
  }
}

void schedule(String messageTemp) {
  int h, m;
  char direction[10];
  sscanf(messageTemp.c_str(), "%d:%d %s", &h, &m, &direction);
  String dir = String(direction);

  if (dir == "up")
    TimeUp = h*60 + m;
  if (dir == "down")
    TimeDown = h*60 + m;
}
