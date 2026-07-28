#pragma once

#include <PubSubClient.h>
#include <BlockNot.h> // Toegevoegd voor de timer

WiFiClient    espClient;
PubSubClient  client(espClient);

extern void callback(String topic, byte* message, unsigned int length);

class Broker
{
private:
  const char* MQTT_username = "Gordijnclient"; 
  const char* MQTT_password = "Halt2001"; 
  const char* MQTT_server   = "192.168.1.150";
  
  // Timer: Probeer elke 10 seconden opnieuw te verbinden met MQTT
  BlockNot mqttReconnectTimer = BlockNot(10, SECONDS); 

  void subscriptions(){
    client.subscribe("gordijn/#");
  }

public: 
  void begin(){
    client.setCallback(callback); 
    client.setServer(MQTT_server, 1883);
    // Let op: we blokkeren hier niet meer bij het opstarten!
  }

  void update(){ 
    if (client.connected()) {
      client.loop(); 
    }
  }

  // NIEUW: Deze functie controleert op de achtergrond de status
  void handleConnection() {
    // Alleen proberen als WiFi werkt, en MQTT niet verbonden is
    if (WiFi.status() == WL_CONNECTED && !client.connected()) {
      
      // Probeer het maximaal 1x per 10 seconden
      if (mqttReconnectTimer.TRIGGERED) {
        Serial.print("Attempting MQTT connection...");
        
        if (client.connect("ESP32GordijnClient", MQTT_username, MQTT_password)) {
          Serial.println("connected");
          subscriptions();
        } else {
          Serial.print("failed, rc=");
          Serial.println(client.state()); // Print foutcode en ga direct door
        }
      }
    }
  }

  void publish(String topic, String message) {
    // Alleen publiceren als we online zijn, geen delays of reconnects forceren!
    if (client.connected()) {
      topic = "gordijn/" + topic;
      client.publish(topic.c_str(), message.c_str());
    }
  }
};