#ifndef broker_h
#define broker_h

#include <PubSubClient.h>

WiFiClient    espClient;
PubSubClient  client(espClient);

extern void callback(String topic, byte* message, unsigned int length);

class Broker
{
private:

const char* MQTT_username = "leendertr"; 
const char* MQTT_password = "Halt2001"; 
const char* MQTT_server   = "server-cam.duckdns.org";

  void subscriptions(){
    client.subscribe("gordijn/#");
  }

  void connect() {
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");

      client.connect("ESP32GordijnClient", MQTT_username, MQTT_password);
      delay(2000);
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

  void update(){ 
    check_connection();
    client.loop(); 
  }

  void publish(String topic, String message, bool retain = false) {
    check_connection();
    topic = "gordijn/" + topic;
    client.publish(topic.c_str(), message.c_str(), retain);
  }

  void check_connection(){
    if (!client.connected()) {
      WiFi.reconnect();
      connect();
    }
  }
};

#endif