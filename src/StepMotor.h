#include "esp32-hal-gpio.h"
#include <EasyButton.h>

#ifndef STEPMOTOR_H
#define STEPMOTOR_H

#define ATTACH_PIN  4  
#define DIR_PIN     5  
#define STEP_PIN    23 
#define BUTTON_PIN  16
#define LED_PIN     27

#define STEP_SIZE   200

#define UPWARD   false
#define DOWNWARD true

EasyButton button(BUTTON_PIN);

class StepMotor{
private:

  int numSteps = 95000; 
  int progress = -1;

  bool active    = false; 
  bool paused    = false;
  bool position  = HIGH;
  bool direction = DOWNWARD; 
  
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
    partly_open(); 
    step();
    completed();
  }

  bool idle() {
    button.read();
    
    if (!active || paused)
      return true;
    return false;
  }

  void roll_up() {
    direction = UPWARD;
    if(position == LOW)
      start();
  }

  void roll_down() {
    direction = DOWNWARD;
    if(position == HIGH)
      start();
  }

  void start() { !active ? start_motor() : toggle_pause(); }
  
  void reverse() {
    if (active){
      direction = !direction;
      position  = !position;
      paused = false; 
    }   
  }

  void open_partially(int p) {
    progress = p;
    start();
  }

private:

  void start_motor() {
    digitalWrite(ATTACH_PIN, LOW);
    active = true;
    digitalWrite(LED_PIN, HIGH);
  }

  void step() {
    digitalWrite(DIR_PIN, direction ? HIGH : LOW);

    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(STEP_SIZE);

    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(STEP_SIZE);

    direction ? stepsTaken++ : stepsTaken--;
  }

  void completed() {
    if (stepsTaken >= numSteps || stepsTaken <= 0) {
      digitalWrite(LED_PIN, LOW);
      paused = false; 
      active = false;
      direction = !direction;
      position = !position;
      if (position == HIGH);{
        digitalWrite(ATTACH_PIN, HIGH);
        
      }
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
    digitalWrite(LED_PIN, LOW);
  }
  void unpause() {
    paused = false;
    digitalWrite(LED_PIN, HIGH);
  }

};

#endif
