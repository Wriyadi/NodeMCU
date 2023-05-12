#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "SSID";
const char* password = "Password";

// Initialize Telegram BOT
#define BOTtoken "621947xxx:AAF0UP4GDzzclhDtFtP20UWXIa06_xxx"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "87558xxx"

X509List cert(TELEGRAM_CERTIFICATE_ROOT);

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 0,5 second.
int botRequestDelay = 500;
unsigned long lastTimeBotRan;

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following commands to control your outputs.\n\n";
      welcome += "/led1_on to turn GPIO ON \n";
      welcome += "/led1_off to turn GPIO OFF \n";
      welcome += "/state to request current GPIO state \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/led1_on") {
      bot.sendMessage(chat_id, "LED1 state set to ON", "");
      digitalWrite(D1, HIGH);
    }
    
    if (text == "/led1_off") {
      bot.sendMessage(chat_id, "LED1 state set to OFF", "");
      digitalWrite(D1, LOW);
    }
    
    if (text == "/led2_on") {
      bot.sendMessage(chat_id, "LED2 state set to ON", "");
      digitalWrite(D2, HIGH);
    }
    
    if (text == "/led2_off") {
      bot.sendMessage(chat_id, "LED2 state set to OFF", "");
      digitalWrite(D2, LOW);
    }

    if (text == "/led3_on") {
      bot.sendMessage(chat_id, "LED3 state set to ON", "");
      digitalWrite(D3, HIGH);
    }
    
    if (text == "/led3_off") {
      bot.sendMessage(chat_id, "LED3 state set to OFF", "");
      digitalWrite(D3, LOW);
    }
    if (text == "/state") {
      if (digitalRead(D1==1)){
        bot.sendMessage(chat_id, "LED1 is ON", "");
      }
      else {
        bot.sendMessage(chat_id, "LED1 is OFF", "");
      }
      if (digitalRead(D2==1)){
        bot.sendMessage(chat_id, "LED2 is ON", "");
      }
      else {
        bot.sendMessage(chat_id, "LED2 is OFF", "");
      }
      if (digitalRead(D3==1)){
        bot.sendMessage(chat_id, "LED3 is ON", "");
      }
      else {
        bot.sendMessage(chat_id, "LED3 is OFF", "");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org

  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
