#pragma once

#include <sunset.h>
#include <time.h>

#define LATITUDE        52.39200088742884
#define LONGITUDE       4.6145287343396255

SunSet sun;

class LocalTime {
private:
  int lastTriggeredMin = -1;
  int lastCalcDay = -1; // Tracks the last day we calculated sun times

public:
  int sunrise;
  int sunset;
  int nowTimeMins;
  struct tm now;

  void setup() {
    sun.setPosition(LATITUDE, LONGITUDE, 0); // Offset is injected dynamically later
  }
  
  void update() {
    if (!getLocalTime(&now)) {
      return; // Do nothing if time isn't synced via NTP yet
    }

    nowTimeMins = now.tm_hour * 60 + now.tm_min;

    // Recalculate sun times only when the day changes (or on first boot)
    if (now.tm_mday != lastCalcDay) {
      calcSunTimes();
      lastCalcDay = now.tm_mday;
    }
  }

  void calcSunTimes() {
    // FIX 1: Correct the ESP32 time structure to match what Sunset library expects
    // tm_year is years since 1900, tm_mon is 0-indexed (Jan = 0)
    sun.setCurrentDate(now.tm_year + 1900, now.tm_mon + 1, now.tm_mday);
    
    // FIX 2: Dynamic Daylight Saving Time for the Sunset library
    // now.tm_isdst is > 0 during summer time. NL is UTC+1 (Winter) or UTC+2 (Summer).
    int currentOffset = 1 + (now.tm_isdst > 0 ? 1 : 0);
    sun.setTZOffset(currentOffset);

    sunrise = static_cast<int>(sun.calcSunrise());
    sunset  = static_cast<int>(sun.calcSunset());

    Serial.printf("Sun times updated: Sunrise at %02d:%02d, Sunset at %02d:%02d\n", 
                  sunrise / 60, sunrise % 60, sunset / 60, sunset % 60);
  }

  bool check(int setMinutes) {
    static int lastMinuteChecked = -1;
    
    // Reset the trigger lock when a new minute starts
    if (nowTimeMins != lastMinuteChecked) {
      lastTriggeredMin = -1;
      lastMinuteChecked = nowTimeMins;
    }

    if (nowTimeMins == setMinutes && lastTriggeredMin != setMinutes) {
      lastTriggeredMin = setMinutes;
      return true;
    }
    return false;
  }
};