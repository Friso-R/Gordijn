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

      client.connect("ESP32Client", MQTT_username, MQTT_password) ? 
      connected_succesfully() : retry_connection();
    }
  }

  void connected_succesfully(){
    Serial.println("connected");  
    subscriptions();
  }

  void retry_connection(){
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    delay(5000);
  }

public: 
  void begin(){
    client.setCallback(callback); 
    client.setServer(MQTT_server, 1883);
    subscriptions();
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