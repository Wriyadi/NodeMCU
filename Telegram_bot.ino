/*
  ESP8266 / ESP32
  Telegram Bot + 4 Relay + DHT22
  Versi: Optimized Long Polling

  Perintah:
    /start
    /help
    /r1_on    /r1_off
    /r2_on    /r2_off
    /r3_on    /r3_off
    /r4_on    /r4_off
    /all_on   /all_off
    /temp
    /hum
    /status
*/

// =====================================================
// 1. LIBRARY
// =====================================================

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#else
  #error "Pilih board ESP8266 atau ESP32"
#endif

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <time.h>
#include <math.h>

// =====================================================
// 2. KONFIGURASI PIN
// =====================================================

#if defined(ESP8266)

  // NodeMCU: D1, D2, D3, D4
  const uint8_t RELAY_PINS[4] = {
    5, 4, 0, 2
  };

  // NodeMCU D5
  const uint8_t DHT_PIN = 14;

#elif defined(ESP32)

  const uint8_t RELAY_PINS[4] = {
    26, 27, 32, 33
  };

  const uint8_t DHT_PIN = 4;

#endif

// =====================================================
// 3. KONFIGURASI WIFI DAN TELEGRAM
// =====================================================

const char* WIFI_SSID = "hotspot@unama";
const char* WIFI_PASSWORD = "";

const char* BOT_TOKEN = "6219478371:xxxxx";
const char* CHAT_ID = "8755xxxxx";

// =====================================================
// 4. KONFIGURASI RELAY DAN SENSOR
// =====================================================

const uint8_t RELAY_COUNT = 4;

// true = relay ON saat GPIO LOW
// false = relay ON saat GPIO HIGH
const bool RELAY_ACTIVE_LOW = true;

#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

bool relayState[RELAY_COUNT] = {
  false, false, false, false
};

float temperatureC = NAN;
float humidity = NAN;

const unsigned long DHT_INTERVAL_MS = 2500;

unsigned long lastDHTRead = 0;
unsigned long deviceStartedAt = 0;

bool sensorHasBeenRead = false;

// =====================================================
// 5. OBJEK TELEGRAM DAN SSL
// =====================================================

WiFiClientSecure telegramClient;
UniversalTelegramBot bot(BOT_TOKEN, telegramClient);

#if defined(ESP8266)
  X509List telegramCert(TELEGRAM_CERTIFICATE_ROOT);
#endif

bool tlsConfigured = false;
bool clockSynced = false;

unsigned long lastWiFiRetry = 0;
unsigned long lastClockRetry = 0;

const unsigned long WIFI_RETRY_MS = 10000;
const unsigned long CLOCK_RETRY_MS = 30000;

// Long polling: Telegram menunggu update hingga 5 detik.
// Update baru akan membuat permintaan selesai lebih awal.
const int TELEGRAM_LONG_POLL_SECONDS = 5;

// =====================================================
// 6. INISIALISASI SSL DAN WAKTU NTP
// =====================================================

void configureTelegramTLS() {

#if defined(ESP8266)

  telegramClient.setTrustAnchors(&telegramCert);

#elif defined(ESP32)

  telegramClient.setCACert(TELEGRAM_CERTIFICATE_ROOT);

#endif

  // Waktu sistem diperlukan untuk validasi sertifikat TLS.
  configTime(
    0,
    0,
    "pool.ntp.org",
    "time.google.com"
  );

  tlsConfigured = true;
  lastClockRetry = millis();

  Serial.println(
    "Sertifikat Telegram dikonfigurasi; sinkronisasi NTP dimulai."
  );
}

void updateClockStatus() {

  if (!tlsConfigured) {
    configureTelegramTLS();
  }

  time_t now = time(nullptr);

  // Epoch valid setelah tahun 2023.
  clockSynced = (now > 1700000000);

  if (!clockSynced &&
      millis() - lastClockRetry >= CLOCK_RETRY_MS) {

    lastClockRetry = millis();

    configTime(
      0,
      0,
      "pool.ntp.org",
      "time.google.com"
    );

    Serial.println("Mencoba sinkronisasi waktu NTP...");
  }
}

void waitForClockSync(unsigned long timeoutMs) {

  unsigned long start = millis();

  while (millis() - start < timeoutMs) {

    if (time(nullptr) > 1700000000) {
      clockSynced = true;
      Serial.println("Waktu NTP berhasil disinkronkan.");
      return;
    }

    delay(100);
  }

  clockSynced = (time(nullptr) > 1700000000);

  if (!clockSynced) {
    Serial.println(
      "NTP belum siap. ESP akan mencoba kembali."
    );
  }
}

// =====================================================
// 7. KONTROL RELAY
// =====================================================

void setRelay(uint8_t index, bool turnOn) {

  if (index >= RELAY_COUNT) {
    return;
  }

  uint8_t outputLevel;

  if (RELAY_ACTIVE_LOW) {
    outputLevel = turnOn ? LOW : HIGH;
  } else {
    outputLevel = turnOn ? HIGH : LOW;
  }

  digitalWrite(RELAY_PINS[index], outputLevel);

  // Simpan status logis untuk pelaporan yang konsisten.
  relayState[index] = turnOn;

  Serial.print("Relay ");
  Serial.print(index + 1);
  Serial.print(" -> ");
  Serial.println(turnOn ? "ON" : "OFF");
}

void setAllRelays(bool turnOn) {

  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    setRelay(i, turnOn);
  }
}

// =====================================================
// 8. PEMBACAAN DHT22 DENGAN CACHE
// =====================================================

void updateDHTIfDue() {

  unsigned long now = millis();

  // Hindari membaca DHT22 terlalu sering.
  if (!sensorHasBeenRead &&
      now - deviceStartedAt < 2000) {
    return;
  }

  if (sensorHasBeenRead &&
      now - lastDHTRead < DHT_INTERVAL_MS) {
    return;
  }

  lastDHTRead = now;
  sensorHasBeenRead = true;

  float newHumidity = dht.readHumidity();
  float newTemperature = dht.readTemperature();

  if (isnan(newHumidity) || isnan(newTemperature)) {

    temperatureC = NAN;
    humidity = NAN;

    Serial.println("ERROR: Pembacaan DHT22 gagal.");

  } else {

    temperatureC = newTemperature;
    humidity = newHumidity;

    Serial.print("Suhu: ");
    Serial.print(temperatureC, 1);
    Serial.print(" °C | Kelembapan: ");
    Serial.print(humidity, 1);
    Serial.println(" %");
  }
}

// =====================================================
// 9. FORMAT STATUS
// =====================================================

String getSensorStatus() {

  updateDHTIfDue();

  if (isnan(temperatureC) || isnan(humidity)) {
    return
      "Data DHT22 belum tersedia.\n"
      "Periksa koneksi sensor dan coba lagi.";
  }

  String message;
  message.reserve(100);

  message += "Monitoring DHT22\n";
  message += "Suhu: ";
  message += String(temperatureC, 1);
  message += " °C\n";

  message += "Kelembapan: ";
  message += String(humidity, 1);
  message += " %";

  return message;
}

String getSystemStatus() {

  updateDHTIfDue();

  String message;
  message.reserve(220);

  message += "STATUS PERANGKAT\n";
  message += "--------------------\n";

  for (uint8_t i = 0; i < RELAY_COUNT; i++) {

    message += "Relay ";
    message += String(i + 1);
    message += ": ";
    message += relayState[i] ? "ON" : "OFF";
    message += "\n";
  }

  message += "\n";

  if (isnan(temperatureC) || isnan(humidity)) {

    message += "Suhu: belum tersedia\n";
    message += "Kelembapan: belum tersedia";

  } else {

    message += "Suhu: ";
    message += String(temperatureC, 1);
    message += " °C\n";

    message += "Kelembapan: ";
    message += String(humidity, 1);
    message += " %";
  }

  return message;
}

// =====================================================
// 10. BANTUAN BOT
// =====================================================

String getHelpMessage() {

  String message;
  message.reserve(420);

  message += "ESP IoT Control System\n\n";

  message += "KONTROL RELAY\n";
  message += "/r1_on  - Relay 1 ON\n";
  message += "/r1_off - Relay 1 OFF\n";
  message += "/r2_on  - Relay 2 ON\n";
  message += "/r2_off - Relay 2 OFF\n";
  message += "/r3_on  - Relay 3 ON\n";
  message += "/r3_off - Relay 3 OFF\n";
  message += "/r4_on  - Relay 4 ON\n";
  message += "/r4_off - Relay 4 OFF\n\n";

  message += "KONTROL KELOMPOK\n";
  message += "/all_on  - Semua relay ON\n";
  message += "/all_off - Semua relay OFF\n\n";

  message += "MONITORING\n";
  message += "/temp   - Cek suhu\n";
  message += "/hum    - Cek kelembapan\n";
  message += "/status - Status perangkat\n";
  message += "/help   - Bantuan perintah";

  return message;
}

// =====================================================
// 11. PEMROSESAN PERINTAH
// =====================================================

void handleCommand(
  const String& chatId,
  String command,
  const String& fromName
) {

  command.trim();
  command.toLowerCase();

  // Mendukung format /command@NamaBot pada grup Telegram.
  int atPosition = command.indexOf('@');

  if (atPosition >= 0) {
    command = command.substring(0, atPosition);
  }

  Serial.print("Perintah: ");
  Serial.println(command);

  if (command == "/start" || command == "/help") {

    String welcome = "Halo ";

    if (fromName.length() > 0) {
      welcome += fromName;
    } else {
      welcome += "pengguna";
    }

    welcome += "!\n\n";
    welcome += getHelpMessage();

    bot.sendMessage(chatId, welcome, "");
    return;
  }

  // Kontrol individual dan kelompok relay.
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {

    String prefix = "/r";
    prefix += String(i + 1);
    prefix += "_";

    if (command == prefix + "on") {

      setRelay(i, true);

      String reply = "Relay ";
      reply += String(i + 1);
      reply += " berhasil diatur ke ON.";

      bot.sendMessage(chatId, reply, "");
      return;
    }

    if (command == prefix + "off") {

      setRelay(i, false);

      String reply = "Relay ";
      reply += String(i + 1);
      reply += " berhasil diatur ke OFF.";

      bot.sendMessage(chatId, reply, "");
      return;
    }
  }

  if (command == "/all_on") {

    setAllRelays(true);

    bot.sendMessage(
      chatId,
      "Semua relay telah diatur ke ON.",
      ""
    );

    return;
  }

  if (command == "/all_off") {

    setAllRelays(false);

    bot.sendMessage(
      chatId,
      "Semua relay telah diatur ke OFF.",
      ""
    );

    return;
  }

  if (command == "/temp") {

    updateDHTIfDue();

    if (isnan(temperatureC)) {
      bot.sendMessage(
        chatId,
        "Data suhu tidak tersedia. Periksa sensor DHT22.",
        ""
      );
    } else {

      String reply = "Suhu saat ini: ";
      reply += String(temperatureC, 1);
      reply += " °C";

      bot.sendMessage(chatId, reply, "");
    }

    return;
  }

  if (command == "/hum") {

    updateDHTIfDue();

    if (isnan(humidity)) {
      bot.sendMessage(
        chatId,
        "Data kelembapan tidak tersedia. Periksa sensor DHT22.",
        ""
      );
    } else {

      String reply = "Kelembapan saat ini: ";
      reply += String(humidity, 1);
      reply += " %";

      bot.sendMessage(chatId, reply, "");
    }

    return;
  }

  if (command == "/status" || command == "/state") {

    bot.sendMessage(
      chatId,
      getSystemStatus(),
      ""
    );

    return;
  }

  bot.sendMessage(
    chatId,
    "Perintah tidak dikenali. Kirim /help untuk daftar perintah.",
    ""
  );
}

// =====================================================
// 12. PEMERIKSAAN PESAN TELEGRAM
// =====================================================

void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String chatId = bot.messages[i].chat_id;

    // Hanya CHAT_ID yang diizinkan boleh mengontrol perangkat.
    // Pengguna lain diabaikan tanpa balasan.
    if (chatId != String(CHAT_ID)) {

      Serial.println("Pesan ditolak: CHAT_ID tidak diizinkan.");
      continue;
    }

    String text = bot.messages[i].text;
    String fromName = bot.messages[i].from_name;

    handleCommand(chatId, text, fromName);
  }
}

// =====================================================
// 13. WIFI: PENYAMBUNGAN ULANG
// =====================================================

void startWiFi() {

  WiFi.mode(WIFI_STA);

#if defined(ESP8266)
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
#elif defined(ESP32)
  WiFi.setSleep(false);
#endif

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  lastWiFiRetry = millis();

  Serial.println("Menghubungkan ke Wi-Fi...");

  unsigned long startAttempt = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startAttempt < 15000) {

    delay(250);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("Wi-Fi terhubung.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

  } else {

    Serial.println(
      "Wi-Fi belum terhubung. Akan mencoba kembali."
    );
  }
}

void maintainWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  if (millis() - lastWiFiRetry >= WIFI_RETRY_MS) {

    lastWiFiRetry = millis();

    Serial.println("Wi-Fi terputus. Mencoba menyambung ulang.");

    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
}

// =====================================================
// 14. SETUP
// =====================================================

void setup() {

  Serial.begin(115200);
  delay(100);

  deviceStartedAt = millis();

  Serial.println();
  Serial.println("==============================");
  Serial.println("ESP Telegram IoT Controller");
  Serial.println("==============================");

  // Atur relay OFF sebelum memulai komunikasi jaringan.
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {

    uint8_t offLevel =
      RELAY_ACTIVE_LOW ? HIGH : LOW;

    digitalWrite(RELAY_PINS[i], offLevel);
    pinMode(RELAY_PINS[i], OUTPUT);

    relayState[i] = false;
  }

  dht.begin();

  startWiFi();

  if (WiFi.status() == WL_CONNECTED) {

    configureTelegramTLS();
    waitForClockSync(10000);
  }

  // Long polling ditangani oleh getUpdates().
  bot.longPoll = TELEGRAM_LONG_POLL_SECONDS;

  Serial.println("Inisialisasi selesai.");
}

// =====================================================
// 15. LOOP UTAMA
// =====================================================

void loop() {

  maintainWiFi();

  if (WiFi.status() != WL_CONNECTED) {
    delay(10);
    return;
  }

  // Pastikan konfigurasi TLS dan sinkronisasi waktu tersedia.
  if (!tlsConfigured) {
    configureTelegramTLS();
  }

  updateClockStatus();

  // Jangan menonaktifkan validasi sertifikat jika waktu NTP
  // belum tersedia. Tunggu sampai jam sistem tersinkronisasi.
  if (!clockSynced) {
    updateDHTIfDue();
    delay(10);
    return;
  }

  updateDHTIfDue();

  // Permintaan akan menunggu update Telegram hingga batas
  // long polling atau sampai update baru datang.
  int numNewMessages =
    bot.getUpdates(bot.last_message_received + 1);

  // Ambil seluruh pesan tertunda sebelum kembali menunggu.
  while (numNewMessages > 0) {

    handleNewMessages(numNewMessages);

    yield();

    numNewMessages =
      bot.getUpdates(bot.last_message_received + 1);
  }

  yield();
}