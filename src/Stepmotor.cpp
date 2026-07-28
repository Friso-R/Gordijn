#include "StepMotor.h"

// Hier maken we de globale knop daadwerkelijk aan in het geheugen
EasyButton button(BUTTON_PIN);

// 1. Definitie van de veilige statische pointer
StepMotor*& StepMotor::getInstance() {
  static StepMotor* instance = nullptr;
  return instance;
}

// 2. Definitie van de statische wrapper
void IRAM_ATTR StepMotor::onTimer() {
  if (getInstance() != nullptr) {
    getInstance()->handleInterrupt();
  }
}

// 3. Definitie van de daadwerkelijke Interrupt Logica
void IRAM_ATTR StepMotor::handleInterrupt() {
  portENTER_CRITICAL_ISR(&timerMux);
  
  if (active && !paused) {
    if (!step_state) {
      digitalWrite(STEP_PIN, HIGH);
      step_state = true;
      timerAlarmWrite(motorTimer, 200, true); // 200 microseconden HIGH puls
    } else {
      digitalWrite(STEP_PIN, LOW);
      step_state = false;
      timerAlarmWrite(motorTimer, 50, true);  // 50 microseconden LOW pauze
      
      direction ? stepsTaken++ : stepsTaken--;
    }
  }
  
  portEXIT_CRITICAL_ISR(&timerMux);
}