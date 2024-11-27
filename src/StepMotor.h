#include <EasyButton.h>
#include "Switch.h"

#ifndef STEPMOTOR_H
#define STEPMOTOR_H

#define BUTTON_PIN  25
#define STEP_SIZE   200

#define UP    0
#define DOWN  1

Switch ATTACH (4);
Switch DIR    (5);
Switch MOSFET (17);
Switch STEP   (23);
Switch LED    (27);

extern void CreatePublishTask();

EasyButton button(BUTTON_PIN);

class StepMotor{
private:

  int numSteps = 95000;
  int progress = -1;

  bool active    = false; 
  bool paused    = false;
  bool position  = HIGH;
  bool direction = true; 
  
public:

  int stepsTaken = 0;

  void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    button.onPressedFor(1000, [this]() { reverse(); });
    button.onPressed   (      [this]() { start();   });
  }

  void update() {
    step();
    partly_open(); 
    completed();
  }

  bool idle() { return (!active || paused); }

  void roll(bool pos) {
    if (!active){
      if (position == pos) start_motor();
    }
    else 
    { position == pos ? reverse() : unpause(); }
  }

  void start() { !active ? start_motor() : toggle_pause(); }
  
  void reverse() {
    if (active){
      direction = !direction;
      position  = !position;
      unpause();
    }   
  }

  void open_partially(int p) {
    progress = p;
    start();
  }

private:

  void start_motor() {
    driver_on();
    
    active = true;
    position  = !position;

    CreatePublishTask();
  }

  void step() {
    direction ? DIR.on() : DIR.off();

    STEP.on();
    delayMicroseconds(150);

    STEP.off();
    delayMicroseconds(80);

    direction ? stepsTaken++ : stepsTaken--;
  }

  void completed() {
    if (stepsTaken >= numSteps || stepsTaken <= 0) {
      paused = false; 
      active = false;
      direction = !direction;

      driver_off();
    }
  }

  void partly_open(){
    if (stepsTaken == progress*950){
      pause();
      progress = -1;
    }
  }

  void toggle_pause() {  paused ? unpause() : pause();  }
  
  void pause() {
    paused = true;
    driver_off();
  }
  void unpause() {
    paused = false;
    driver_on();
    CreatePublishTask();
  }

  void driver_on(){
    MOSFET.off();
    ATTACH.off();
    LED.on();
  }
  void driver_off(){
    ATTACH.on();
    LED.off();
    MOSFET.on();
  }
};

#endif
