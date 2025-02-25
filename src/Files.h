#include <Arduino.h>
#include <BlockNot.h>
#include <time.h>

#include "WiFiSetup.h"
#include "broker.h"
#include "LocalTime.h"
#include "StepMotor.h"

void monitor();
int  schedule(String messageTemp);
void sync();
void open_curtain_partly(String messageTemp);
void check_schedule();
void check_sunTimes();
void sunLoop();
void callback(String topic, byte* message, unsigned int length);
String mins_to_time(int t);
void publishProgress(void *parameter);
void CreatePublishTask();
