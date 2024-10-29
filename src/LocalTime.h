#include <sunset.h>

#define LATITUDE        52.39200088742884
#define LONGITUDE       4.6145287343396255
#define DST_OFFSET      1

SunSet sun;

class LocalTime {
public:

  int sunrise;
  int sunset;
  int nowTimeMins;
  struct tm now;

  void setup() {
    sun.setPosition(LATITUDE, LONGITUDE, DST_OFFSET);
    sun.setTZOffset(DST_OFFSET);
    update();
    calcSunTimes();
  }
  
  void update() {
    if (!getLocalTime(&now)) {
      Serial.println("Failed to obtain time");
    }
    //Serial.println(&now, "%A, %B %d %Y %H:%M:%S");

    nowTimeMins = now.tm_hour * 60 + now.tm_min;
    if (nowTimeMins == 1)
      calcSunTimes();
  }

  void calcSunTimes() {
    sun.setCurrentDate(now.tm_year, now.tm_mon, now.tm_mday);
    sunrise = static_cast<int>(sun.calcSunrise());
    sunset  = static_cast<int>(sun.calcSunset());
  }

  bool check(int setMinutes){
    return (nowTimeMins == setMinutes);
  }
};
