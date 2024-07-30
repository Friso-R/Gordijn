#ifndef broker_h
#define broker_h

#include <PubSubClient.h>

WiFiClient    espClient;
PubSubClient  client(espClient);

extern void callback(String topic, byte* message, unsigned int length);

class Broker
{
private:
  const char* MQTT_username = "student033"; 
  const char* MQTT_password = "U5rlK4N8"; 
  const char* MQTT_server   = "science-vs352.science.uu.nl";

  void connect() {
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");

      if (client.connect("ESP32Client", MQTT_username, MQTT_password)) {
        Serial.println("connected");  
        client.subscribe("infob3it/student033/gordijn");
      } 
      else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
      }
    }
  }

public: 
  void begin(){
    client.setCallback(callback); 
    client.setServer(MQTT_server, 1883);
    client.subscribe("infob3it/student033/gordijn");
    connect();
  }

  void update(){ client.loop(); }

  void publish(String topic, String message) {

    if (!client.connected()) 
      connect();
      
    if(!client.loop())
      client.connect("ESP32Client", MQTT_username, MQTT_password);
    
    topic = "infob3it/student033/" + topic;
    client.publish(topic.c_str(),   String(message).c_str());
  }
};

#endif