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

void update();
void callback(String topic, byte* message, unsigned int length);

void setup() {
  wifi.connect();
  broker.begin();
  sunTime.setup();
  stepMotor.setup();
  button.begin();
}

void loop() {
  broker.update();

  if (stepMotor.idle()){
    if(t60.TRIGGERED){
      sunTime.loop();
      if(sunTime.check(sunTime.sunrise))
        stepMotor.roll_up();
      if(sunTime.check(sunTime.sunset))
        stepMotor.roll_down();
    }
  }
  else stepMotor.update();
}

void update(){
  
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
      stepMotor.revert();
    if(messageTemp == "updown")
      stepMotor.up_down();
    if(messageTemp == "up")
      stepMotor.roll_up();
    if(messageTemp == "down")
      stepMotor.roll_down();
  }
}
