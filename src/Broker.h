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
  /*
  const char* MQTT_username = "leendertr"; 
  const char* MQTT_password = "Halt2001"; 
  const char* MQTT_server   = "192.168.1.213";
  */
  void subscriptions(){
    client.subscribe("infob3it/student033/#");
  }

  void connect() {
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");

      client.connect("ESP32GordijnClient", MQTT_username, MQTT_password);
      delay(500);
      subscriptions();
    }
    Serial.println("connected"); 
  }

public: 
  void begin(){
    client.setCallback(callback); 
    client.setServer(MQTT_server, 1883);
    connect();
  }

  void update(){ client.loop(); }

  void publish(String topic, String message) {

    if (!client.connected()) 
      connect();
    
    topic = "infob3it/student033/" + topic;
    client.publish(topic.c_str(),   message.c_str());
  }
};

#endif