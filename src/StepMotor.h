#include "esp32-hal-gpio.h"
#include <EasyButton.h>

#ifndef STEPMOTOR_H
#define STEPMOTOR_H

#define ATTACH_PIN  4  
#define DIR_PIN     5  
#define STEP_PIN    23 
#define BUTTON_PIN  16
#define LED_PIN     27
#define MOSFET_PIN  17

#define STEP_SIZE   200

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
    pinMode(DIR_PIN   , OUTPUT);
    pinMode(STEP_PIN  , OUTPUT);
    pinMode(LED_PIN   , OUTPUT);
    pinMode(ATTACH_PIN, OUTPUT);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    button.onPressedFor(1000, [this]() { reverse(); });
    button.onPressed   (      [this]() { start();   });
  }

  void update() {
    button.read();
    step();
    partly_open(); 
    completed();
  }

  bool idle() {
    button.read();
    
    if (!active || paused)
      return true;
    return false;
  }

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
  }

  void step() {
    digitalWrite(DIR_PIN, direction ? HIGH : LOW);

    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(STEP_SIZE);

    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(100);

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
  }

  void driver_on(){
    digitalWrite(MOSFET_PIN,  LOW);
    digitalWrite(ATTACH_PIN,  LOW);
    digitalWrite(LED_PIN   , HIGH);
    
  }

  void driver_off(){
    digitalWrite(ATTACH_PIN, HIGH);
    digitalWrite(LED_PIN   ,  LOW);
    digitalWrite(MOSFET_PIN, HIGH);
  }
};

#endif
