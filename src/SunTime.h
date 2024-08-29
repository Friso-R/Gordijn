#include <TimeLib.h>
#include <sunset.h>

#define LATITUDE        52.39200088742884
#define LONGITUDE       4.6145287343396255
#define DST_OFFSET      1

const uint8_t _usDSTStart[22] = { 8,14,13,12,10, 9, 8,14,12,11,10, 9,14,13,12,11, 9 };
const uint8_t _usDSTEnd[22]   = { 1, 7, 6, 5, 3, 2, 1, 7, 5, 4, 3, 2, 7, 6, 5, 4, 2 };

static const char ntpServerName[] = "nl.pool.ntp.org";
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

static WiFiUDP Udp; // Make Udp static
unsigned int localPort = 8888;  // local port to listen for UDP packets

SunSet sun;

class SunTime {
public:

  int sunrise;
  int sunset;
  int currentTimeMinutes;

  static time_t getNtpTime() {
    IPAddress ntpServerIP; // NTP server's ip address

    //while (Udp.parsePacket() > 0); // discard any previously received packets
    //Serial.println("Transmit NTP Request");
    // get a random server from the pool
    WiFi.hostByName(ntpServerName, ntpServerIP);
    sendNTPpacket(ntpServerIP);

    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
      int size = Udp.parsePacket();
      if (size >= NTP_PACKET_SIZE) {
        //Serial.println("Receive NTP Response");
        Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
        unsigned long secsSince1900;
        // convert four bytes starting at location 40 to a long integer
        secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
        secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
        secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
        secsSince1900 |= (unsigned long)packetBuffer[43];
        return secsSince1900 - 2208988800UL + (DST_OFFSET * SECS_PER_HOUR);
      }
    }
    //Serial.println("No NTP Response :-(");
    return 0; // return 0 if unable to get the time
  }

  static void sendNTPpacket(IPAddress &address) {
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket(address, 123); //NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
  }

  void setup() {
    sun.setPosition(LATITUDE, LONGITUDE, DST_OFFSET);
    sun.setTZOffset(DST_OFFSET);
    setSyncProvider(getNtpTime);
    setSyncInterval(60*60*12);
    loop();
  }

  void loop() {
    int currentDay = day();
    int currentHour = hour();
    static int lastUpdateDay = 0; // Variable to track the last update day
    static int lastUpdateHour = 0;

    // Check if the day has changed
    if (currentDay != lastUpdateDay) {
      // Update the sunrise and sunset times
      sun.setCurrentDate(year(), month(), currentDay);
      sunrise = static_cast<int>(sun.calcSunrise());
      sunset  = static_cast<int>(sun.calcSunset());
  
      //sunTG.send(String(sunrise));
      //sunTG.send(String(sunset));

      // Update the last update day
      lastUpdateDay = currentDay;
      lastUpdateHour = currentHour;
    }
    currentTimeMinutes = hour() * 60 + minute();
    //Serial.printf("%d-%02d-%02d %02d:%02d:%02d\n", year(), month(), day(), hour(), minute(), second());
  }

  bool check(int setMinutes){
    if (currentTimeMinutes == setMinutes) 
      return true;
    return false;
  }
};
