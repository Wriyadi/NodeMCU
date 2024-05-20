#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

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
      welcome += "/r1_on to turn GPIO ON \n";
      welcome += "/r1_off to turn GPIO OFF \n";
      welcome += "/r2_on to turn GPIO ON \n";
      welcome += "/r2_off to turn GPIO OFF \n";
      welcome += "/r3_on to turn GPIO ON \n";
      welcome += "/r3_off to turn GPIO OFF \n";
      welcome += "/temp to get temperature data \n";
      welcome += "/hum to get humidity data \n";
      welcome += "/state to request current GPIO state \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/r1_on") {
      bot.sendMessage(chat_id, "Relay 1 set to ON", "");
      digitalWrite(16, LOW);
    }
    
    if (text == "/r1_off") {
      bot.sendMessage(chat_id, "Relay 1 set to OFF", "");
      digitalWrite(16, HIGH);
    }
    
    if (text == "/r2_on") {
      bot.sendMessage(chat_id, "Relay 2 set to ON", "");
      digitalWrite(5, LOW);
    }
    
    if (text == "/r2_off") {
      bot.sendMessage(chat_id, "Relay 2 set to OFF", "");
      digitalWrite(5, HIGH);
    }

    if (text == "/r3_on") {
      bot.sendMessage(chat_id, "Relay 3 set to ON", "");
      digitalWrite(4, LOW);
    }
    
    if (text == "/r3_off") {
      bot.sendMessage(chat_id, "Relay 3 set to OFF", "");
      digitalWrite(4, HIGH);
    }

    if (text == "/temp") {
      float temperature = dht.readTemperature();
      if (!isnan(temperature)) {
        bot.sendMessage(chat_id, "Temperature: " + String(temperature) + " °C", "");
      } else {
        bot.sendMessage(chat_id, "Failed to read temperature data", "");
      }
    }

    if (text == "/hum") {
      float humidity = dht.readHumidity();
      if (!isnan(humidity)) {
        bot.sendMessage(chat_id, "Humidity: " + String(humidity) + " %", "");
      } else {
        bot.sendMessage(chat_id, "Failed to read humidity data", "");
      }
    }
    
    if (text == "/state") {
      if (digitalRead(16)==LOW){
        bot.sendMessage(chat_id, "RELAY 1 is ON", "");
      }
      else {
        bot.sendMessage(chat_id, "RELAY 1 is OFF", "");
      }
      if (digitalRead(5)==LOW){
        bot.sendMessage(chat_id, "RELAY 2 is ON", "");
      }
      else {
        bot.sendMessage(chat_id, "RELAY 2 is OFF", "");
      }
      if (digitalRead(4)==LOW){
        bot.sendMessage(chat_id, "RELAY 3 is ON", "");
      }
      else {
        bot.sendMessage(chat_id, "RELAY 3 is OFF", "");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org

  pinMode(16, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(2, INPUT);
  
  digitalWrite(16, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(4, HIGH);

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
