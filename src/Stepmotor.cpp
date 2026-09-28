#include "StepMotor.h"

StepMotor*& StepMotor::getInstance() {
  static StepMotor* instance = nullptr;
  return instance;
}

void IRAM_ATTR StepMotor::onTimer() {
  if (getInstance() != nullptr) {
    getInstance()->handleInterrupt();
  }
}

void IRAM_ATTR StepMotor::handleInterrupt() {
  portENTER_CRITICAL_ISR(&timerMux);
  
  if (active && !paused) {
    if (!step_state) {
      digitalWrite(STEP_PIN, HIGH);
      step_state = true;
      timerAlarmWrite(motorTimer, 200, true); // 200 µs HIGH pulse
    } else {
      digitalWrite(STEP_PIN, LOW);
      step_state = false;
      timerAlarmWrite(motorTimer, 50, true);  // 50 µs LOW pause
      
      if (direction) {
        stepsTaken--;
      } else {
        stepsTaken++;
      }

      // FIX: Hardware-level bounds checking prevents network-blocking overshoot
      if ((direction && stepsTaken <= targetStep) || (!direction && stepsTaken >= targetStep)) {
        stepsTaken = targetStep;         // Clamp exact position
        active = false;                  // Cut off motor logic immediately
        movementComplete = true;         // Flag the main loop to run cleanup
        timerAlarmDisable(motorTimer);   // Kill the interrupt timer
      }
    }
  }
  
  portEXIT_CRITICAL_ISR(&timerMux);
}