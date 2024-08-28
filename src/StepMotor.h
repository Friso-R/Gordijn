#include "esp32-hal-gpio.h"
#include <EasyButton.h>

#ifndef STEPMOTOR_H
#define STEPMOTOR_H

#define DIR_PIN     5  
#define STEP_PIN    23 
#define ATTACH_PIN  4  
#define BUTTON_PIN  16
#define LED_PIN     27

#define STEP_SIZE   200

EasyButton button(BUTTON_PIN);

class StepMotor{
private:

  int numSteps = 95000; 
  int stepsTaken = 0;

  bool raised    = true;
  bool paused    = false;
  bool direction = true; 
  bool active    = false; 
  
public:

  void setup() {
    pinMode(DIR_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(ATTACH_PIN, OUTPUT);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(ATTACH_PIN, LOW); // Enable the driver by default

    button.onPressedFor(1500, [this]() { revert(); });
    button.onPressed   (      [this]() { start();  });
  }

  void update() {
    button.read();
    step();
    completed();
  }

  bool idle(){
    button.read();
    
    if (!active || paused)
      return true;
    return false;
  }

  void start(){
    if (!active) { //Start motor
      active = true;
      digitalWrite(LED_PIN, HIGH);
    }
    else toggle_pause();
  }

  void up_down(){ !active ? start() : revert(); }
  
  void revert(){
    if (active){
      direction = !direction;
      raised = !raised;
      stepsTaken = numSteps - stepsTaken;
      paused = false; 
    }   
  }

  void roll_up(){
    direction = false;
    if(!raised){
      start();
    }
  }

  void roll_down(){
    direction = true;
    if(raised){
      start();
    }
  }
  
private:
  void step(){
    digitalWrite(DIR_PIN, direction ? HIGH : LOW);

    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(STEP_SIZE);

    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(STEP_SIZE);

    stepsTaken++;
  }

  void completed(){
    if (stepsTaken >= numSteps || stepsTaken <= 0) {
      digitalWrite(LED_PIN, LOW);
      paused = false; 
      active = false;
      direction = !direction;
      raised = !raised;
      stepsTaken = 0; 
    }
  }

  

  void toggle_pause(){  paused ? unpause() : pause();  }
  
  void pause(){
    paused = true;
    digitalWrite(LED_PIN, LOW);
  }
  void unpause(){
    paused = false;
    digitalWrite(LED_PIN, HIGH);
  }

};

#endif
