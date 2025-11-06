#include <EasyButton.h>

#ifndef STEPMOTOR_H
#define STEPMOTOR_H

#define UP    0
#define DOWN  1

#define ATTACH_PIN  4  
#define DIR_PIN     19  
#define STEP_PIN    18 
#define BUTTON_PIN  32
#define LED_PIN     25
//#define MOSFET_PIN  27

#define SLEEP_PIN  17
#define RESET_PIN  16


extern void CreatePublishTask();

EasyButton button(BUTTON_PIN);

class StepMotor{
private:

  int numSteps = 95000;
  int progress = -1;

  bool active    = false; 
  bool paused    = false;
  bool position  = LOW;
  bool direction = false; 
  
public:

  int stepsTaken = numSteps;

  void setup() {
    pinMode(DIR_PIN   , OUTPUT);
    pinMode(STEP_PIN  , OUTPUT);
    pinMode(LED_PIN   , OUTPUT);
    pinMode(ATTACH_PIN, OUTPUT);
    pinMode(SLEEP_PIN , OUTPUT);
    pinMode(RESET_PIN , OUTPUT);

    digitalWrite(SLEEP_PIN,  HIGH);
    digitalWrite(RESET_PIN,  HIGH);

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
    bool base = (position == pos);

    active ? reverse_or_continue_from(base) : leaveFrom(base); 
  }

  void reverse_or_continue_from(bool fromBase) { fromBase ? reverse() : unpause(); }
  void leaveFrom(bool atBase) { if (atBase) start_motor(); }

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
/*
  void retained_state(int prog){
    progress = prog;
    if (prog > 0){
      
       
      //position  = HIGH;
      //direction = true; 
      
      stepsTaken = numSteps/100 * progress;
    }
    if (prog == 100){
      position  = LOW;
      direction = false;
      stepsTaken = numSteps;
    }
  }


*/

void state_active(){
  active    = true; 
  paused    = true;

}
private:

  void start_motor() {
    driver_on();

    active = true;
    position  = !position;

    CreatePublishTask();
  }

  void step() {
    digitalWrite(DIR_PIN, direction ? HIGH : LOW);

    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(200);

    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(50);

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
    //vTaskDelete(progressTaskHandle);
    //progressTaskHandle = NULL;
  }
  void unpause() {
    paused = false;
    driver_on();
    CreatePublishTask();
  }

  void driver_on(){
    //digitalWrite(MOSFET_PIN,  LOW);
    digitalWrite(ATTACH_PIN,  LOW);
    digitalWrite(LED_PIN   , HIGH);
  }
  void driver_off(){
    digitalWrite(ATTACH_PIN, HIGH);
    digitalWrite(LED_PIN   ,  LOW);
    //digitalWrite(MOSFET_PIN, HIGH);
  }

};

#endif
