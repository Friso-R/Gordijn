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
  bool timeConfigured = false;

  void setup() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    // Zet de ESP32 in Station modus en start WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    // Laat de ESP32 op de achtergrond zelf reconnecten als hij wegvalt!
    WiFi.setAutoReconnect(true); 
  }

  // Deze functie roepen we straks in de main loop aan
  void handleTime() {
    // Haal pas de tijd op als de WiFi verbonden is, en doe dit maar 1 keer
    if (WiFi.status() == WL_CONNECTED && !timeConfigured) {
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      timeConfigured = true;
      Serial.println("\nWiFi Connected! Network Time Configured.");
    }
  }

int nowTimeMin()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return -1;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S"); 
  return timeinfo.tm_hour * 60 + timeinfo.tm_min;
}
};

#endif
