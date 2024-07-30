#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define BOT_TOKEN "7488293420:AAGXOYiaEx1iXSWArrqnEaA9v83VOnSv2nE"  // your Bot Token (Get from Botfather)
#define CHAT_ID   "1373871797"

#define WIFI_SSID "A-je-to! 2.4"
#define WIFI_PASSWORD "HoldTheDoor!187"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done

class Telegram{
public:
  void handleNewMessages(int numNewMessages)
  {
    for (int i = 0; i < numNewMessages; i++)
    {
      bot.sendMessage(bot.messages[i].chat_id, bot.messages[i].text, "");
    }
  }

  void send(String message)
  {
    bot.sendMessage(CHAT_ID, message, "");
  }

  void begin()
  {
    configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
    time_t now = time(nullptr);
    Serial.println(now);
    
    secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  }

  void update()
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages){
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
  }
};
