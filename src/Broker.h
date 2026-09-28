#pragma once

#include <PubSubClient.h>
#include <BlockNot.h> // Toegevoegd voor de timer

WiFiClient    espClient;
PubSubClient  client(espClient);

extern void callback(String topic, byte* message, unsigned int length);

class Broker
{
private:
  const char* MQTT_username = "Kasclient"; 
  const char* MQTT_password = "Halt2001"; 
  const char* MQTT_server   = "192.168.1.150";
  
  // 1. Add a timer for Wi-Fi reconnects (e.g., every 15 seconds)
  BlockNot wifiReconnectTimer = BlockNot(15, SECONDS);
  
  // Timer: Probeer elke 10 seconden opnieuw te verbinden met MQTT
  BlockNot mqttReconnectTimer = BlockNot(10, SECONDS); 

  void subscriptions(){
    client.subscribe("gordijn/#");
  }

public: 
  void begin(){
    client.setCallback(callback); 
    client.setServer(MQTT_server, 1883);
    
    // Force a strict keep-alive and socket timeout to detect container resets
    client.setKeepAlive(15);
    client.setSocketTimeout(15);
  }

  void update(){ 
    if (client.connected()) {
      client.loop(); 
    }
  }

  void handleConnection() {
    if (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
      if (wifiReconnectTimer.TRIGGERED) {
        Serial.println("Network dropped or DHCP lost. Forcing reconnect...");
        WiFi.disconnect();
        WiFi.reconnect();
      }
      return; 
    }

    if (!client.connected()) {
      // Probeer het maximaal 1x per 10 seconden
      if (mqttReconnectTimer.TRIGGERED) {
        Serial.print("Attempting MQTT connection...");
        
        // Purge any lingering half-open socket states before reconnecting
        client.disconnect(); 
        
        if (client.connect("ESP32KasGorijnClient", MQTT_username, MQTT_password)) {
          Serial.println("connected");
          subscriptions();
        } else {
          Serial.print("failed, rc=");
          Serial.println(client.state()); 
        }
      }
    }
  }

  void publish(const char* topic, const char* message) {
    if (client.connected()) {
      char fullTopic[64];
      snprintf(fullTopic, sizeof(fullTopic), "gordijn/%s", topic);
      client.publish(fullTopic, message); // Directly use the C-string
    }
  }
};