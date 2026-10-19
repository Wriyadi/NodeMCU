/*
 * ============================================================================
 * PROJECT : SinricPro Smart Relay + DHT22
 * BOARD   : ESP8266 NodeMCU / ESP32
 * FEATURES:
 *   - 2 relay controlled by SinricPro
 *   - DHT22 temperature and humidity monitoring
 *   - Wi-Fi reconnection attempts
 *   - Periodic sensor acquisition
 *   - Change-based sensor reporting with rate limiting
 *   - Configurable active-LOW / active-HIGH relay logic
 *
 * LIBRARIES:
 *   - SinricPro
 *   - Adafruit DHT sensor library
 *   - Adafruit Unified Sensor
 * ============================================================================
 */

#include <Arduino.h>
#include <math.h>
#include <DHT.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#else
  #error "Board tidak didukung. Gunakan ESP8266 atau ESP32."
#endif

#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <SinricProTemperaturesensor.h>

// ============================================================================
// 1. CREDENTIALS
// ============================================================================

// Ganti dengan kredensial Wi-Fi Anda.
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASS     "YOUR_WIFI_PASSWORD"

// Gunakan kredensial baru dari dashboard SinricPro.
#define APP_KEY       "YOUR_NEW_APP_KEY"
#define APP_SECRET    "YOUR_NEW_APP_SECRET"

// Buat atau pilih perangkat Temperature Sensor di SinricPro,
// lalu masukkan Device ID yang sesuai.
#define TEMP_SENSOR_ID "YOUR_TEMPERATURE_SENSOR_DEVICE_ID"
#define Relay_ID1 "YOUR_RELAY_DEVICE_ID"
#define Relay_ID2 "YOUR_RELAY_DEVICE_ID"

// ============================================================================
// 2. HARDWARE CONFIGURATION
// ============================================================================

#if defined(ESP8266)

  // Konfigurasi mengikuti pin pada kode awal:
  // D16 = GPIO16, D5 = GPIO14, D4 = GPIO2.
  constexpr uint8_t RELAY_PIN_1 = 16;
  constexpr uint8_t RELAY_PIN_2 = 14;
  constexpr uint8_t DHT_PIN     = 2;

#elif defined(ESP32)

  // Konfigurasi mengikuti pin pada kode awal.
  constexpr uint8_t RELAY_PIN_1 = 16;
  constexpr uint8_t RELAY_PIN_2 = 17;
  constexpr uint8_t DHT_PIN     = 5;

#endif

constexpr uint8_t DHT_TYPE = DHT22;

// Atur true untuk modul relay aktif-LOW.
// Atur false untuk modul relay aktif-HIGH.
constexpr bool RELAY_ACTIVE_LOW = true;

constexpr uint8_t RELAY_ON_LEVEL =
    RELAY_ACTIVE_LOW ? LOW : HIGH;

constexpr uint8_t RELAY_OFF_LEVEL =
    RELAY_ACTIVE_LOW ? HIGH : LOW;

// ============================================================================
// 3. TIMING CONFIGURATION
// ============================================================================

constexpr unsigned long SENSOR_READ_INTERVAL_MS = 5000UL;
constexpr unsigned long WIFI_RETRY_INTERVAL_MS  = 15000UL;

// Pengiriman dibatasi minimal satu kali per 60 detik.
// Data tetap dibaca setiap 5 detik.
constexpr unsigned long MIN_EVENT_INTERVAL_MS = 60000UL;

// Kirim ulang data secara berkala walaupun nilainya tidak berubah.
constexpr unsigned long SENSOR_HEARTBEAT_MS = 300000UL;

// Ambang perubahan agar fluktuasi kecil tidak terus memicu event.
constexpr float TEMP_CHANGE_THRESHOLD_C = 0.3f;
constexpr float HUM_CHANGE_THRESHOLD_PCT = 1.0f;

// ============================================================================
// 4. RELAY DEVICE CONFIGURATION
// ============================================================================

struct RelayInfo {
  const char* deviceId;
  const char* name;
  uint8_t pin;
};

RelayInfo relays[] = {
  {
    Relay_ID1,
    "Relay 1",
    RELAY_PIN_1
  },
  {
    Relay_ID2,
    "Relay 2",
    RELAY_PIN_2
  }
};

constexpr size_t RELAY_COUNT =
    sizeof(relays) / sizeof(relays[0]);

// ============================================================================
// 5. GLOBAL OBJECTS AND STATE
// ============================================================================

DHT dht(DHT_PIN, DHT_TYPE);

bool sinricStarted = false;

unsigned long lastWiFiAttemptMs = 0;
unsigned long lastSensorReadMs = 0;
unsigned long lastEventAttemptMs = 0;
unsigned long lastSuccessfulEventMs = 0;

bool firstSensorRead = true;
bool hasEventAttempt = false;
bool hasPublishedData = false;

// Mendorong pengiriman data setelah koneksi SinricPro berhasil.
bool forceSensorPublish = true;

float lastSentTemperature = 0.0f;
float lastSentHumidity = 0.0f;

// ============================================================================
// 6. FUNCTION DECLARATIONS
// ============================================================================

void initializeRelays();
void setRelayState(const RelayInfo& relay, bool state);
bool onPowerState(const String& deviceId, bool& state);

void startWiFi();
void setupSinricPro();

void readAndReportSensor();
void reportSensorEvent(float temperature, float humidity);

// ============================================================================
// 7. SETUP
// ============================================================================

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println();
  Serial.println(F("===================================="));
  Serial.println(F(" SinricPro Smart Relay + DHT22"));
  Serial.println(F("===================================="));

#if defined(ESP8266)
  Serial.println(F("[SYSTEM] Board: ESP8266"));
#elif defined(ESP32)
  Serial.println(F("[SYSTEM] Board: ESP32"));
#endif

  initializeRelays();

  dht.begin();
  Serial.println(F("[SENSOR] DHT22 initialized."));

  startWiFi();

  Serial.println(F("[SYSTEM] Initialization completed."));
}

// ============================================================================
// 8. MAIN LOOP
// ============================================================================

void loop() {
  const unsigned long now = millis();

  // Check Wi-Fi state and retry periodically if disconnected.
  if (WiFi.status() == WL_CONNECTED) {
    if (!sinricStarted) {
      setupSinricPro();
    }
  } else if (now - lastWiFiAttemptMs >= WIFI_RETRY_INTERVAL_MS) {
    lastWiFiAttemptMs = now;

    Serial.println(F("[WiFi] Connection lost/not ready. Retrying..."));
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  }

  // SinricPro must be serviced continuously once initialized.
  if (sinricStarted) {
    SinricPro.handle();
  }

  // Read the sensor immediately, then periodically.
  if (firstSensorRead ||
      now - lastSensorReadMs >= SENSOR_READ_INTERVAL_MS) {

    firstSensorRead = false;
    lastSensorReadMs = now;

    readAndReportSensor();
  }
}

// ============================================================================
// 9. RELAY INITIALIZATION
// ============================================================================

void initializeRelays() {
  for (size_t i = 0; i < RELAY_COUNT; i++) {
    // Set the OFF level before enabling output mode.
    digitalWrite(relays[i].pin, RELAY_OFF_LEVEL);
    pinMode(relays[i].pin, OUTPUT);
  }

  Serial.println(F("[RELAY] All relays initialized to OFF."));
}

// ============================================================================
// 10. CENTRALIZED RELAY CONTROL
// ============================================================================

void setRelayState(const RelayInfo& relay, bool state) {
  digitalWrite(
    relay.pin,
    state ? RELAY_ON_LEVEL : RELAY_OFF_LEVEL
  );

  Serial.printf(
    "[RELAY] %s -> %s\r\n",
    relay.name,
    state ? "ON" : "OFF"
  );
}

// ============================================================================
// 11. SINRICPRO SWITCH CALLBACK
// ============================================================================

bool onPowerState(const String& deviceId, bool& state) {
  for (size_t i = 0; i < RELAY_COUNT; i++) {
    if (deviceId.equals(relays[i].deviceId)) {
      setRelayState(relays[i], state);
      return true;
    }
  }

  Serial.println(F("[RELAY][ERROR] Unknown device ID."));
  return false;
}

// ============================================================================
// 12. WI-FI INITIALIZATION
// ============================================================================

void startWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

#if defined(ESP8266)
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
#elif defined(ESP32)
  WiFi.setSleep(false);
#endif

  Serial.printf("[WiFi] Connecting to: %s\r\n", WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  lastWiFiAttemptMs = millis();
}

// ============================================================================
// 13. SINRICPRO INITIALIZATION
// ============================================================================

void setupSinricPro() {
  // Register each relay device and its callback.
  for (size_t i = 0; i < RELAY_COUNT; i++) {
    SinricProSwitch& mySwitch = SinricPro[relays[i].deviceId];
    mySwitch.onPowerState(onPowerState);
  }

  // Register the temperature sensor device.
  SinricProTemperaturesensor& mySensor =
      SinricPro[TEMP_SENSOR_ID];

  // Suppress an unused-variable warning; device registration occurs
  // through the SinricPro device accessor above.
  (void)mySensor;

  SinricPro.onConnected([]() {
    Serial.println(F("[SinricPro] Connected."));
    forceSensorPublish = true;
    hasEventAttempt = false;
  });

  SinricPro.onDisconnected([]() {
    Serial.println(F("[SinricPro] Disconnected."));
  });

  // Start SinricPro exactly once.
  SinricPro.begin(APP_KEY, APP_SECRET);
  sinricStarted = true;

  Serial.println(F("[SinricPro] Client initialized."));
}

// ============================================================================
// 14. DHT22 SENSOR ACQUISITION
// ============================================================================

void readAndReportSensor() {
  const float temperature = dht.readTemperature();
  const float humidity = dht.readHumidity();

  // Reject invalid or implausible readings.
  if (isnan(temperature) ||
      isnan(humidity) ||
      temperature < -40.0f ||
      temperature > 80.0f ||
      humidity < 0.0f ||
      humidity > 100.0f) {

    Serial.println(F("[SENSOR][ERROR] Invalid DHT22 reading."));
    return;
  }

  Serial.printf(
    "[SENSOR] Temperature: %.1f C | Humidity: %.1f%%\r\n",
    temperature,
    humidity
  );

  if (!sinricStarted) {
    Serial.println(F("[SinricPro] Waiting for Wi-Fi connection."));
    return;
  }

  reportSensorEvent(temperature, humidity);
}

// ============================================================================
// 15. CHANGE-BASED SENSOR REPORTING
// ============================================================================

void reportSensorEvent(float temperature, float humidity) {
  const unsigned long now = millis();

  const bool temperatureChanged =
      !hasPublishedData ||
      fabsf(temperature - lastSentTemperature) >=
          TEMP_CHANGE_THRESHOLD_C;

  const bool humidityChanged =
      !hasPublishedData ||
      fabsf(humidity - lastSentHumidity) >=
          HUM_CHANGE_THRESHOLD_PCT;

  const bool heartbeatDue =
      hasPublishedData &&
      (now - lastSuccessfulEventMs >= SENSOR_HEARTBEAT_MS);

  const bool eventRequired =
      forceSensorPublish ||
      temperatureChanged ||
      humidityChanged ||
      heartbeatDue;

  if (!eventRequired) {
    return;
  }

  // Prevent excessive event attempts.
  if (hasEventAttempt &&
      now - lastEventAttemptMs < MIN_EVENT_INTERVAL_MS) {
    return;
  }

  lastEventAttemptMs = now;
  hasEventAttempt = true;

  SinricProTemperaturesensor& mySensor =
      SinricPro[TEMP_SENSOR_ID];

  const bool success =
      mySensor.sendTemperatureEvent(temperature, humidity);

  if (success) {
    lastSentTemperature = temperature;
    lastSentHumidity = humidity;

    lastSuccessfulEventMs = now;
    hasPublishedData = true;
    forceSensorPublish = false;

    Serial.println(F("[SinricPro] Sensor event sent successfully."));
  } else {
    Serial.println(
      F("[SinricPro][ERROR] Failed to send sensor event.")
    );

    // Do not update the last successful values.
    // A later attempt can retry the latest measurement.
  }
}
