#include "Files.h"

WiFiSetup wifi;
Broker    broker;
LocalTime klok;
StepMotor stepMotor;

BlockNot t60 (60, SECONDS);

bool circadianMode;
bool scheduleMode = 1;
int timeUp   = 10 * 60; 
int timeDown = 16 * 60;
int sunrise, sunset;

TaskHandle_t progressTaskHandle = NULL;

void setup() {
  Serial.begin(9600);
  
  wifi.connect();
  broker.begin();
  button.begin();
  klok.setup();
  stepMotor.setup();
  
  sync();
}

void loop() {
  broker.update();
  button.read();
  stepMotor.idle() ? monitor() : stepMotor.update();
}

void monitor() {
  if(t60.TRIGGERED){
    klok.update();
    if (scheduleMode) check_schedule();
    if (circadianMode) sunLoop();
  }
}

void sync(){
  broker.publish("status", "online");
  /*
  broker.publish("progress/get", String(stepMotor.stepsTaken/950));
  broker.publish("mode/circadian", String(circadianMode));
  broker.publish("mode/schedule", String(scheduleMode));
  broker.publish("sunrise", mins_to_time(klok.sunrise));
  broker.publish("sunset" , mins_to_time(klok.sunset));
  */
}

// This function is executed when some device publishes a message to a topic that the ESP32 is subscribed to
void callback(String topic, byte* message, unsigned int length) {
  topic = topic.substring(8);
  String msg;
  
  for (int i = 0; i < length; i++)  
    msg += (char)message[i];

  if(topic == "action"){
    if(msg == "start")   stepMotor.start();
    if(msg == "reverse") stepMotor.reverse();
    if(msg == "up")      stepMotor.roll(UP);
    if(msg == "down")    stepMotor.roll(DOWN);
  } 
  if(topic == "mode/circadian"){ circadianMode = msg.toInt(); sunLoop(); }
  if(topic == "mode/schedule")   scheduleMode = msg.toInt();
  if(topic == "schedule/up")     timeUp = schedule(msg);
  if(topic == "schedule/down")   timeDown = schedule(msg);
  if(topic == "progress/set")    open_curtain_partly(msg);
  //if(topic == "progress/get")   stepMotor.retained_state(msg.toInt());
  if(topic == "status/sync")     sync();
}

int schedule(String messageTemp) {
  int h, m, s, timeMin;
  sscanf(messageTemp.c_str(), "%d:%d:%d", &h, &m, &s);

  timeMin = h*60 + m;
  return timeMin;
}

void check_schedule(){
  if(klok.check(timeUp))
    stepMotor.roll(UP);
  if(klok.check(timeDown))
    stepMotor.roll(DOWN);
}

void check_sunTimes(){
  if(klok.check(klok.sunrise))
    stepMotor.roll(UP);
  if(klok.check(klok.sunset))
    stepMotor.roll(DOWN);
}

String mins_to_time(int t) {
  char timeChars[6];
  sprintf(timeChars, "%d:%02d", t / 60, t % 60);
  return String(timeChars);
}

void sunLoop(){
  check_sunTimes();

  if (sunrise != klok.sunrise){
    sunrise = klok.sunrise;
    broker.publish("sunrise", mins_to_time(sunrise));
  }
  if (sunset != klok.sunset){
    sunset = klok.sunset;
    broker.publish("sunset", mins_to_time(sunset));
  }
}

void open_curtain_partly(String messageTemp){
  int progress;
  sscanf(messageTemp.c_str(), "%d", &progress);
  stepMotor.open_partially(progress);
}

void publishProgress(void *parameter) {
  //unsigned long start = millis();
  int stepSize = 950;
  bool motor_active = true;

  while (motor_active) 
  {
    motor_active = !stepMotor.idle();

    const int steps = stepMotor.stepsTaken;
    if(steps % stepSize == 0)
    {
      //unsigned long publishDuration = millis() - start;
      //Serial.println(publishDuration);
      //start = millis();
      broker.publish("progress/get", String(steps/stepSize));
      
      vTaskDelay(300 / portTICK_PERIOD_MS);
    }
    //taskYIELD();
    
  }
  //Serial.println("Task deleted");
  progressTaskHandle = NULL;
  vTaskDelete(NULL); 
}

void CreatePublishTask() {
  
  //Serial.print("progressTaskHandle: ");
  //Serial.println((uint32_t)progressTaskHandle, HEX);  // Print as a hexadecimal memory address

  if (progressTaskHandle == NULL) {
  xTaskCreatePinnedToCore(
    publishProgress,       // Function to run
    "PublishTask",         // Task name
    2000,                  // Stack size in bytes
    NULL,                  // Parameter to pass
    999,                   // Priority
    &progressTaskHandle,   // Task handle for external control
    0                      // Core ID (0 = Core 0)
  );
}
}

