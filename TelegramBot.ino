#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>

// --- KONFIGURASI PIN UNIVERSAL ---
// Menggunakan pin yang aman untuk masing-masing board
#if defined(ESP8266)
  #define DHTPIN 2
  #define RELAY1_PIN 16
  #define RELAY2_PIN 5
  #define RELAY3_PIN 4
  #define RELAY4_PIN 0
#elif defined(ESP32)
  #define DHTPIN 4
  #define RELAY1_PIN 26
  #define RELAY2_PIN 27
  #define RELAY3_PIN 32
  #define RELAY4_PIN 33
#endif

#define DHTTYPE DHT22

// --- KONFIGURASI JARINGAN & BOT ---
const char* ssid = "SSID";
const char* password = "Password";
#define BOTtoken "6219478371:AAF0UP4GDxxxxx" 
#define CHAT_ID "875xxxxx"

// --- INISIALISASI OBJEK ---
DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Sertifikat SSL Telegram untuk ESP8266
#if defined(ESP8266)
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

// Variabel delay dan timer
int botRequestDelay = 500;
unsigned long lastTimeBotRan;

// --- FUNGSI HANDLE PESAN ---
void handleNewMessages(int numNewMessages) {
  Serial.print("Menangani pesan baru: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    
    // Validasi User
    if (chat_id != CHAT_ID) {
      bot.sendMessage(chat_id, "Akses Ditolak. Anda bukan pengguna yang sah.", "");
      continue;
    }

    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    Serial.println("Pesan diterima: " + text);

    if (text == "/start") {
      String welcome = "Halo, " + from_name + ".\n";
      welcome += "Gunakan perintah berikut untuk kontrol:\n\n";
      welcome += "/r1_on - Nyalakan Relay 1\n";
      welcome += "/r1_off - Matikan Relay 1\n";
      welcome += "/r2_on - Nyalakan Relay 2\n";
      welcome += "/r2_off - Matikan Relay 2\n";
      welcome += "/r3_on - Nyalakan Relay 3\n";
      welcome += "/r3_off - Matikan Relay 3\n";
      welcome += "/r4_on - Nyalakan Relay 4\n";
      welcome += "/r4_off - Matikan Relay 4\n";
      welcome += "/temp - Cek Suhu\n";
      welcome += "/hum - Cek Kelembaban\n";
      welcome += "/state - Cek Status Relay\n";
      bot.sendMessage(chat_id, welcome, "");
    }

    // Kontrol Relay 1
    else if (text == "/r1_on") {
      digitalWrite(RELAY1_PIN, LOW);
      bot.sendMessage(chat_id, "Relay 1 diatur ke ON", "");
    } else if (text == "/r1_off") {
      digitalWrite(RELAY1_PIN, HIGH);
      bot.sendMessage(chat_id, "Relay 1 diatur ke OFF", "");
    }

    // Kontrol Relay 2
    else if (text == "/r2_on") {
      digitalWrite(RELAY2_PIN, LOW);
      bot.sendMessage(chat_id, "Relay 2 diatur ke ON", "");
    } else if (text == "/r2_off") {
      digitalWrite(RELAY2_PIN, HIGH);
      bot.sendMessage(chat_id, "Relay 2 diatur ke OFF", "");
    }

    // Kontrol Relay 3
    else if (text == "/r3_on") {
      digitalWrite(RELAY3_PIN, LOW);
      bot.sendMessage(chat_id, "Relay 3 diatur ke ON", "");
    } else if (text == "/r3_off") {
      digitalWrite(RELAY3_PIN, HIGH);
      bot.sendMessage(chat_id, "Relay 3 diatur ke OFF", "");
    }

    // Kontrol Relay 4
    else if (text == "/r4_on") {
      digitalWrite(RELAY4_PIN, LOW);
      bot.sendMessage(chat_id, "Relay 4 diatur ke ON", "");
    } else if (text == "/r4_off") {
      digitalWrite(RELAY4_PIN, HIGH);
      bot.sendMessage(chat_id, "Relay 4 diatur ke OFF", "");
    }

    // Sensor DHT
    else if (text == "/temp") {
      float temperature = dht.readTemperature();
      if (!isnan(temperature)) {
        bot.sendMessage(chat_id, "Suhu saat ini: " + String(temperature) + " °C", "");
      } else {
        bot.sendMessage(chat_id, "Gagal membaca data suhu dari sensor.", "");
      }
    } else if (text == "/hum") {
      float humidity = dht.readHumidity();
      if (!isnan(humidity)) {
        bot.sendMessage(chat_id, "Kelembaban saat ini: " + String(humidity) + " %", "");
      } else {
        bot.sendMessage(chat_id, "Gagal membaca data kelembaban dari sensor.", "");
      }
    }

    // Cek Status Keseluruhan
    else if (text == "/state") {
      String statusMsg = "Status Perangkat Saat Ini:\n\n";
      statusMsg += "Relay 1: " + String(digitalRead(RELAY1_PIN) == LOW ? "ON" : "OFF") + "\n";
      statusMsg += "Relay 2: " + String(digitalRead(RELAY2_PIN) == LOW ? "ON" : "OFF") + "\n";
      statusMsg += "Relay 3: " + String(digitalRead(RELAY3_PIN) == LOW ? "ON" : "OFF") + "\n";
      statusMsg += "Relay 4: " + String(digitalRead(RELAY4_PIN) == LOW ? "ON" : "OFF");
      
      bot.sendMessage(chat_id, statusMsg, "");
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Inisialisasi Sensor DHT
  dht.begin();

  // Konfigurasi SSL Universal untuk Telegram
  #if defined(ESP8266)
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    client.setTrustAnchors(&cert);         // Add root certificate for ESP8266
  #elif defined(ESP32)
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for ESP32
  #endif

  // Inisialisasi Pin
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  
  // Set kondisi awal relay NPN (Active LOW berarti HIGH adalah kondisi mati/awal)
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(RELAY3_PIN, HIGH);
  digitalWrite(RELAY4_PIN, HIGH);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Terhubung!");
  
  #if defined(ESP8266)
    Serial.print("ESP8266 Local IP Address: ");
  #elif defined(ESP32)
    Serial.print("ESP32 Local IP Address: ");
  #endif
  
  Serial.println(WiFi.localIP());
}

void loop() {
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
