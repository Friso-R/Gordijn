#ifndef SWITCH_H
#define SWITCH_H

class Switch{

  int SWITCH_PIN;

public:

  Switch(int pin){
    SWITCH_PIN = pin;
    pinMode(pin, OUTPUT);
  }

  void on() { digitalWrite(SWITCH_PIN, HIGH); Serial.println("Relay " + String(SWITCH_PIN) + " ON!"); }
  void off(){ digitalWrite(SWITCH_PIN, LOW);  Serial.println("Relay " + String(SWITCH_PIN) + " OFF!");}
};

#endif





