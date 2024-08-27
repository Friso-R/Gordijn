#include <Arduino.h>
#include <BlockNot.h>
#include <time.h>

#include "WiFiSetup.h"
#include "broker.h"
//#include "Telegram.h"
#include "suntime.h"
#include "StepMotor.h"

WiFiSetup wifi;
Broker    broker;
//Telegram  tg;
SunTime   sunTime;
StepMotor stepMotor;

BlockNot   t5(5, SECONDS);
BlockNot   t60(60, SECONDS);
BlockNot   t900(900, SECONDS);

void update();
void callback(String topic, byte* message, unsigned int length);

void setup() {
  Serial.begin(9600);

  wifi.connect();
  broker.begin();
  //tg.begin();
  sunTime.setup();
  stepMotor.setup();
  button.begin();
}

void loop() {
  broker.update();
  update();

}

void update(){
  if (stepMotor.idle()){
    /*
    tg.update();
    
    if(t900.TRIGGERED){ tg.send(String(sunTime.currentTimeMinutes)); }

    if(t5.TRIGGERED){
      tg.update();
    }
    */
    if(t60.TRIGGERED){
      sunTime.loop();
      if(sunTime.Sunrise()){
        stepMotor.roll_up();
        //tg.send("sunrise - up");
      }
      if(sunTime.Sunset()){
        stepMotor.roll_down();
        //tg.send("sunset - down");
      }
    }
    
  }
  else{
    stepMotor.update();
    //if(t5.TRIGGERED)
    //  tg.send(String(stepMotor.stepsTaken));
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
      stepMotor.revert();
    if(messageTemp == "updown")
      stepMotor.up_down();
    if(messageTemp == "up")
      stepMotor.roll_up();
    if(messageTemp == "down")
      stepMotor.roll_down();
  }
}
