#ifndef SETUP_WIFI_H
#define SETUP_WIFI_H

#include <WiFi.h>

class WiFiSetup {
private:
  const char* ssid = "A-je-to! 2.4";
  const char* password = "HoldTheDoor!187";
  
  // Static function for handling the WiFi disconnection event
  static void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.println("Disconnected from WiFi access point");
    Serial.print("WiFi lost connection. Reason: ");
    Serial.println(info.wifi_sta_disconnected.reason);
    Serial.println("Trying to Reconnect");
    WiFi.reconnect();
  }

public:
  // Constructor to set up the WiFi event handler
  WiFiSetup() {
    WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  }

  // This function connects ESP32 to router
  void connect() {
    int tries = 3;
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      Serial.println(".");
      delay(3000);
      tries--;
      if (tries == 0) ESP.restart();
    }
    Serial.println("WiFi connected");
  }
};

#endif
