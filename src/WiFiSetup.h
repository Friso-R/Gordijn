#ifndef SETUP_WIFI_H
#define SETUP_WIFI_H

#include <WiFi.h>
#include "time.h"

class WiFiSetup {
private:
  const char* ssid = "Appels";
  const char* password = "R!kwjH0acuEP8JE";
//const char* ssid = "A-je-to! 2.4";
//const char* password = "HoldTheDoor!187";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 0;

public:

// This function connects ESP32 to router
void connect() {
  int tries = 3;
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    //Serial.println(".");
    uint8_t status = WiFi.waitForConnectResult();
    tries--;
    if (tries == 0) ESP.restart();
  }
  Serial.println("WiFi connected");

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}
};

#endif
