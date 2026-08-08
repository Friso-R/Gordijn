#ifndef SETUP_WIFI_H
#define SETUP_WIFI_H

#include <WiFi.h>
#include "time.h"

class WiFiSetup {
private:
  const char* ssid = "Appels";
  const char* password = "R!kwjH0acuEP8JE";
  const char* ntpServer = "pool.ntp.org";

  // POSIX Timezone string for Europe/Amsterdam (Netherlands)
  // Handles CET (UTC+1) and CEST (UTC+2) automatically!
  const char* tzInfo = "CET-1CEST,M3.5.0,M10.5.0/3"; 

public:
  bool timeConfigured = false;

  void setup() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    WiFi.setAutoReconnect(true); 
  }

  void handleTime() {
    if (WiFi.status() == WL_CONNECTED && !timeConfigured) {
      // Use configTzTime for automatic Daylight Saving adjustments
      configTzTime(tzInfo, ntpServer);
      timeConfigured = true;
      Serial.println("\nWiFi Connected! Network Time (with Auto-DST) Configured.");
    }
  }

  int nowTimeMin() {
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
      return -1;
    }
    return timeinfo.tm_hour * 60 + timeinfo.tm_min;
  }
};

#endif