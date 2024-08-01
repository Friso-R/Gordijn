#include <Arduino.h>
#include <EasyButton.h>
#include <BlockNot.h>
#include <time.h>

#include "WiFiSetup.h"
#include "broker.h"
#include "Telegram.h"
#include "suntime.h"
#include "StepMotor.h"

WiFiSetup wifi;
Broker    broker;
Telegram  tg;
SunTime   sunTime(tg);
StepMotor stepMotor;

EasyButton button(16);
BlockNot   t5(5, SECONDS);
BlockNot   t900(900, SECONDS);

void sunTimeUpdate();
void callback(String topic, byte* message, unsigned int length);

void setup() {
  Serial.begin(9600);

  wifi.connect();
  broker.begin();
  tg.begin();
  sunTime.setup();
  stepMotor.setup();
  button.begin();

  button.onPressedFor (1500, []() {  stepMotor.close_when_paused ();  });
  button.onPressed    ([]()       {  stepMotor.start             ();  });
}

void loop() {
  button.read();

  stepMotor.loop();
  sunTimeUpdate();
  broker.update();
  tg.update();

  if(t900.TRIGGERED){ 
    tg.send(String(sunTime.currentTimeMinutes));
  }
}

void sunTimeUpdate(){
  if (!stepMotor.running || stepMotor.paused){
    
    if(t5.TRIGGERED){
      sunTime.loop();
      tg.update();
      broker.publish("status", "up");
    }
    if(sunTime.Sunrise()){
      stepMotor.roll_up();
      tg.send("sunrise - up");
    }
    if(sunTime.Sunset()){
      stepMotor.roll_down();
      tg.send("sunset - down");
    }
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
      stepMotor.close_when_paused();
    if(messageTemp == "updown")
      stepMotor.up_down();
    if(messageTemp == "up")
      stepMotor.roll_up();
    if(messageTemp == "down")
      stepMotor.roll_down();
  }
}
