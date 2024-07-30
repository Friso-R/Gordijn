#include "esp32-hal-gpio.h"

#ifndef STEPMOTOR_H
#define STEPMOTOR_H

#define DIR_PIN     5  
#define STEP_PIN    23 
#define ATTACH_PIN  4  
#define BUTTON_PIN  16
#define LED_PIN     27

#define STEP_SIZE   200

class StepMotor{

private:
  int numSteps = 95000;
  int stepsTaken = 0;

public:
  bool up        = false;
  bool paused    = true;
  bool direction = true; 
  bool running   = false; 

  void setup() {
    pinMode(DIR_PIN, OUTPUT);
    pinMode(STEP_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(ATTACH_PIN, OUTPUT);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(ATTACH_PIN, LOW); // Enable the driver by default
  }

  void loop() {
    if (running && !paused) {
      step();
      completed();
    }
  }

  void start(){
    digitalWrite(LED_PIN, HIGH); // Turn on LED when button is pressed

    if (!running) {
      paused = false; // Start the motor
      stepsTaken = 0; // Reset steps counter
      running = true; // Update motor state
      Serial.println("Motor started");
    } 
    else {
      paused = !paused; 
      if (paused) digitalWrite(LED_PIN, LOW);
    }
  }
  
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
      Serial.println("Motor finished");
      digitalWrite(LED_PIN, LOW);
      paused = true; 
      running = false;
      direction = !direction;
      up = !up;
    }
  }

  void up_down(){ !running ? start() : close_when_paused(); }
  
  void close_when_paused(){
    direction = !direction;
    stepsTaken = numSteps - stepsTaken;
    paused = false;    
  }

  void roll_up(){
    if(!up){
      direction = true;
      start();
    }
  }

  void roll_down(){
    if(up){
      direction = false;
      start();
    }
  }
};

#endif
