#ifndef STEPMOTOR_H
#define STEPMOTOR_H

#include <Arduino.h>
#include <EasyButton.h>

#define CURTAIN_OPEN  45000
#define CURTAIN_CLOSE 0

#define ATTACH_PIN  4  
#define DIR_PIN     19  
#define STEP_PIN    18 
#define BUTTON_PIN  32
#define LED_PIN     25
#define SLEEP_PIN   17
#define RESET_PIN   16

class StepMotor {
private:
  volatile bool active    = false; 
  volatile bool paused    = false;
  volatile bool direction = true; 
  
  // NEW: Add a flag for main loop cleanup and ensure target is volatile
  volatile bool movementComplete = false;
  volatile int targetStep = 0;

  hw_timer_t * motorTimer = NULL;
  portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
  volatile bool step_state = false;
  
  static StepMotor*& getInstance();
  static void IRAM_ATTR onTimer();
  void IRAM_ATTR handleInterrupt();

public:
  int numSteps = 45000;
  volatile int stepsTaken = CURTAIN_OPEN;

  void setup() {
    getInstance() = this;

    pinMode(DIR_PIN   , OUTPUT);
    pinMode(STEP_PIN  , OUTPUT);
    pinMode(LED_PIN   , OUTPUT);
    pinMode(ATTACH_PIN, OUTPUT);
    pinMode(SLEEP_PIN , OUTPUT);
    pinMode(RESET_PIN , OUTPUT);

    digitalWrite(SLEEP_PIN,  HIGH);
    digitalWrite(RESET_PIN,  HIGH);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    driver_off();
    step_state = false;
    
    motorTimer = timerBegin(0, 80, true);
    timerAttachInterrupt(motorTimer, &StepMotor::onTimer, true);
  }

  void update() {
    // Look for the completion flag set by the ISR
    if (movementComplete) {
      portENTER_CRITICAL(&timerMux);
      movementComplete = false;
      portEXIT_CRITICAL(&timerMux);
      
      stop_motor_cleanup();
    }
  }

  bool idle() { return (!active || paused); }

  void moveTo(int target) {
    target = constrain(target, 0, numSteps);
    
    if (target == stepsTaken) {
      Serial.print("Command ignored: Already at target step ");
      Serial.println(target);
      return;
    }

    // Lock the state update so the ISR doesn't read partial data
    portENTER_CRITICAL(&timerMux);
    targetStep = target;
    direction = (targetStep < stepsTaken); 
    active = true;
    paused = false;
    movementComplete = false;
    portEXIT_CRITICAL(&timerMux);

    digitalWrite(DIR_PIN, direction);

    Serial.print("Moving from step ");
    Serial.print(stepsTaken);
    Serial.print(" -> to step ");
    Serial.println(targetStep);

    driver_on();

    timerAlarmWrite(motorTimer, 200, true);
    timerAlarmEnable(motorTimer);
  }
  
  void roll(int targetPosition) {
    moveTo(targetPosition);
  }

  void open_partially(int p) {
    moveTo(p);
  }

  void start() {
    if (active && !paused) {
      pause();
    } else if (paused) {
      unpause();
    } else {
      if (stepsTaken >= (numSteps / 2)) {
        moveTo(0);
      } else {
        moveTo(numSteps);
      }
    }
  }

  void reverse() {
    if (!active) return;
    int newTarget = direction ? numSteps : 0;
    moveTo(newTarget);
  }

private:
  void pause() {
    paused = true;
    timerAlarmDisable(motorTimer);
    digitalWrite(STEP_PIN, LOW);
    driver_off();
    Serial.println("Motor paused.");
  }
  
  void unpause() {
    paused = false;
    digitalWrite(DIR_PIN, direction);
    driver_on();
    timerAlarmEnable(motorTimer);
    Serial.println("Motor unpaused.");
  }

  void stop_motor_cleanup() {
    // ISR has already disabled the timer and stopped pulses. 
    // This just executes the slow serial and pin states safely.
    digitalWrite(STEP_PIN, LOW);
    driver_off();
    Serial.print("Movement complete. Current step: ");
    Serial.println(stepsTaken);
  }

  void driver_on() {
    digitalWrite(ATTACH_PIN, LOW);
    digitalWrite(LED_PIN   , HIGH);
  }
  
  void driver_off() {
    digitalWrite(ATTACH_PIN, HIGH);
    digitalWrite(LED_PIN   , LOW);
  }
};

#endif