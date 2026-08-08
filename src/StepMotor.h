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
  volatile bool direction = true; // true = moving towards 0 (DICHT), false = moving towards 45000 (OPEN)
  
  int targetStep = 0;

  // Hardware timer variables
  hw_timer_t * motorTimer = NULL;
  portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
  volatile bool step_state = false;
  
  static StepMotor*& getInstance();
  static void IRAM_ATTR onTimer();
  void IRAM_ATTR handleInterrupt();

public:
int numSteps = 45000;
  volatile int stepsTaken = CURTAIN_OPEN; // Boots in the OPEN position

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
    if (!active || paused) return;

    bool done = false;
    portENTER_CRITICAL(&timerMux);
    if (direction && stepsTaken <= targetStep) {
      done = true;
      stepsTaken = targetStep; // Clamp exact position
    } else if (!direction && stepsTaken >= targetStep) {
      done = true;
      stepsTaken = targetStep;
    }
    portEXIT_CRITICAL(&timerMux);

    if (done) {
      stop_motor();
    }
  }

  bool idle() { return (!active || paused); }

  // Universal movement command
  void moveTo(int target) {
    target = constrain(target, 0, numSteps);
    
    if (target == stepsTaken) {
      Serial.print("Command ignored: Already at target step ");
      Serial.println(target);
      return;
    }

    targetStep = target;
    direction = (targetStep < stepsTaken); // true = moving towards 0 (DICHT/Close)
    digitalWrite(DIR_PIN, direction);

    Serial.print("Moving from step ");
    Serial.print(stepsTaken);
    Serial.print(" -> to step ");
    Serial.println(targetStep);

    active = true;
    paused = false;
    driver_on();

    timerAlarmWrite(motorTimer, 200, true);
    timerAlarmEnable(motorTimer);
  }

  
  //ROLL FUNCTION: Accepts explicit target positions instead of true/false
  void roll(int targetPosition) {
    moveTo(targetPosition);
  }

  void open_partially(int p) {
    moveTo(p);
  }

  // Handle physical button or MQTT "start"
  void start() {
    if (active && !paused) {
      pause();
    } else if (paused) {
      unpause();
    } else {
      // If idle at or near open (45000), close it; otherwise open it
      if (stepsTaken >= (numSteps / 2)) {
        moveTo(0);
      } else {
        moveTo(numSteps);
      }
    }
  }

  // Handle MQTT "reverse"
  void reverse() {
    if (!active) return;
    
    // Flip destination to opposite end immediately
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

  void stop_motor() {
    active = false;
    paused = false;
    timerAlarmDisable(motorTimer);
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