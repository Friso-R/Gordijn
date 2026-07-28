#ifndef STEPMOTOR_H
#define STEPMOTOR_H

#include <Arduino.h>
#include <EasyButton.h>

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

// Door 'extern' te gebruiken weet de compiler dat deze knop ergens anders bestaat.
// Dit voorkomt dubbele declaraties bij het linken van C++ bestanden.
extern EasyButton button;

class StepMotor {
private:
  int numSteps = 55;
  int progress = -1;

  bool active    = false; 
  bool paused    = false;
  bool position  = LOW;
  bool direction = true; 
  
  // --- TIMER VARIABELEN ---
  hw_timer_t * motorTimer = NULL;
  portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
  volatile bool step_state = false;
  
  // 1. Declaratie van de statische pointer
  static StepMotor*& getInstance();

  // 2. Declaratie van de statische wrapper
  static void IRAM_ATTR onTimer();

  // 3. Declaratie van de Interrupt Logica (Hier is de verdwaalde } verwijderd)
  void IRAM_ATTR handleInterrupt();

public:

  volatile int stepsTaken = numSteps;

  void setup() {
    // Sla een referentie naar DIT specifieke motor-object op in de statische pointer
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

    set_direction();

    button.onPressedFor(1000, [this]() { reverse(); });
    button.onPressed   (      [this]() { start();   });

    // Timer 0, Prescaler 80 (80MHz klok / 80 = 1 tick per microseconde)
    motorTimer = timerBegin(0, 80, true);

    // Koppel de timer aan onze static wrapper
    timerAttachInterrupt(motorTimer, &StepMotor::onTimer, true);
  }

  void update() {
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

  void state_active(){
    active    = true; 
    paused    = true;
  }

private:

  void start_motor() {
    driver_on();

    active = true;
    position  = !position;

    // Start de hardware timer
    timerAlarmWrite(motorTimer, 200, true); // Zet eerste trigger
    timerAlarmEnable(motorTimer);           // Zet de timer aan

    CreatePublishTask();
  }

void completed() {
  if ((direction == UP && stepsTaken >= numSteps) || 
      (direction == DOWN && stepsTaken <= 0)) {
    paused = false; 
    active = false;
    direction = !direction;
    timerAlarmDisable(motorTimer); 
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
    timerAlarmDisable(motorTimer); // Stop de timer tijdens pauze
    driver_off();
  }
  
  void unpause() {
    paused = false;
    driver_on();
    timerAlarmEnable(motorTimer); // Hervat de timer
    CreatePublishTask();
  }

  void driver_on(){
    set_direction();
    digitalWrite(ATTACH_PIN,  LOW);
    digitalWrite(LED_PIN   , HIGH);
  }
  
  void driver_off(){
    set_direction();
    digitalWrite(ATTACH_PIN, HIGH);
    digitalWrite(LED_PIN   ,  LOW);
  }

  void set_direction() 
  { digitalWrite(DIR_PIN, direction ? HIGH : LOW); }

};

#endif