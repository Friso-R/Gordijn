#pragma once

#include <WiFi.h>
#include "time.h"

class WiFiSetup {
private:
  const char* ssid = "Appels";
  const char* password = "R!kwjH0acuEP8JE";
//const char* ssid = "A-je-to! 2.4";
//const char* password = "HoldTheDoor!187";

  const char* ntpServer = "pool.ntp.org";
  
  // POSIX Timezone string for Europe/Amsterdam (Netherlands)
  // Handles CET (UTC+1) and CEST (UTC+2) automatically
  const char* tzInfo = "CET-1CEST,M3.5.0,M10.5.0/3"; 

public:
  // This function connects ESP32 to router
  bool timeConfigured = false;

  void setup() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    // Zet de ESP32 in Station modus en start WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    // Removed WiFi.setAutoReconnect(true) to prevent race conditions
    // with the manual reconnect timer in Broker.h
  }

  // Deze functie roepen we straks in de main loop aan
  void handleTime() {
    // Haal pas de tijd op als de WiFi verbonden is, en doe dit maar 1 keer
    if (WiFi.status() == WL_CONNECTED && !timeConfigured) {
      configTzTime(tzInfo, ntpServer);
      timeConfigured = true;
      Serial.println("\nWiFi Connected! Network Time Configured with Auto-DST.");
    }
  }

  int nowTimeMin() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
      return -1;
    }
    
    // Note: You can comment this print out if you don't want it 
    // spamming your serial monitor every 5 seconds.
    // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S"); 
    
    return timeinfo.tm_hour * 60 + timeinfo.tm_min;
  }
};