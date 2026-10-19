/*
 * ============================================================================
 * PROJECT : Arduino IoT Cloud - Smart Relay & Environment Monitor
 * PLATFORM: ESP8266 NodeMCU / ESP32
 * SENSOR  : DHT22
 * OUTPUT  : 4-Channel Active-LOW Relay
 *
 * FEATURES:
 * 1. Remote relay control through Arduino IoT Cloud.
 * 2. Temperature and humidity monitoring.
 * 3. Non-blocking periodic sensor acquisition.
 * 4. Sensor read-error handling.
 * 5. Safe relay initialization.
 * 6. Structured serial diagnostics.
 *
 * CLOUD VARIABLES:
 *   temperature : float, Read Only
 *   humidity    : float, Read Only
 *   relay1       : bool, Read & Write
 *   relay2       : bool, Read & Write
 *   relay3       : bool, Read & Write
 *   relay4       : bool, Read & Write
 *
 * DEPENDENCIES:
 *   - Arduino IoT Cloud
 *   - DHT sensor library
 *
 * ============================================================================
 */

#include <DHT.h>
#include "thingProperties.h"

// ============================================================================
// 1. BOARD SELECTION
// ============================================================================

// Aktifkan tepat satu pilihan berikut.
#define USE_ESP8266
// #define USE_ESP32

#if defined(USE_ESP8266) && defined(USE_ESP32)
  #error "Pilih hanya satu board: USE_ESP8266 atau USE_ESP32."
#elif !defined(USE_ESP8266) && !defined(USE_ESP32)
  #error "Board belum dipilih. Aktifkan USE_ESP8266 atau USE_ESP32."
#endif

// ============================================================================
// 2. HARDWARE CONFIGURATION
// ============================================================================

#if defined(USE_ESP8266)

  // NodeMCU ESP8266: menggunakan nomor GPIO.
  constexpr uint8_t RELAY_PIN_1 = 16;  // D0 / GPIO16
  constexpr uint8_t RELAY_PIN_2 = 5;   // D1 / GPIO5
  constexpr uint8_t RELAY_PIN_3 = 4;   // D2 / GPIO4
  constexpr uint8_t RELAY_PIN_4 = 14;  // D5 / GPIO14
  constexpr uint8_t DHT_PIN     = 12;  // D6 / GPIO12

#elif defined(USE_ESP32)

  // ESP32 DevKit.
  constexpr uint8_t RELAY_PIN_1 = 26;
  constexpr uint8_t RELAY_PIN_2 = 27;
  constexpr uint8_t RELAY_PIN_3 = 32;
  constexpr uint8_t RELAY_PIN_4 = 33;
  constexpr uint8_t DHT_PIN     = 4;

#endif

constexpr uint8_t DHT_TYPE = DHT22;

constexpr uint8_t RELAY_PINS[] = {
  RELAY_PIN_1,
  RELAY_PIN_2,
  RELAY_PIN_3,
  RELAY_PIN_4
};

constexpr uint8_t RELAY_COUNT =
    sizeof(RELAY_PINS) / sizeof(RELAY_PINS[0]);

// ============================================================================
// 3. RELAY LOGIC
// ============================================================================

// Untuk modul relay Active-LOW:
// ON  = LOW
// OFF = HIGH

constexpr uint8_t RELAY_ON_LEVEL  = LOW;
constexpr uint8_t RELAY_OFF_LEVEL = HIGH;

// ============================================================================
// 4. SENSOR CONFIGURATION
// ============================================================================

constexpr unsigned long SENSOR_INTERVAL_MS = 5000UL;

DHT dht(DHT_PIN, DHT_TYPE);

unsigned long lastSensorReadMs = 0;
bool sensorReadRequested = true;

// ============================================================================
// 5. FUNCTION DECLARATIONS
// ============================================================================

void initializeRelays();
void setRelayState(uint8_t relayNumber, bool state);
void updateSensorData();
void printStartupInformation();

// ============================================================================
// 6. SETUP
// ============================================================================

void setup() {
  Serial.begin(115200);
  delay(1000);

  printStartupInformation();

  // Initialize hardware outputs in the OFF state.
  initializeRelays();

  // Initialize DHT22.
  dht.begin();
  Serial.println(F("[SENSOR] DHT22 initialized."));

  // Initialize Arduino IoT Cloud properties and connection.
  initProperties();

  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  // Cloud diagnostics: 0-4, where higher levels are more detailed.
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  Serial.println(F("[SYSTEM] Initialization completed."));
  Serial.println(F("[SYSTEM] Waiting for Cloud synchronization."));
}

// ============================================================================
// 7. MAIN LOOP
// ============================================================================

void loop() {
  // Keep Cloud communication and property synchronization running.
  ArduinoCloud.update();

  // Periodic sensor acquisition without a recurring delay().
  const unsigned long currentMs = millis();

  if (sensorReadRequested ||
      (currentMs - lastSensorReadMs >= SENSOR_INTERVAL_MS)) {

    sensorReadRequested = false;
    lastSensorReadMs = currentMs;

    updateSensorData();
  }
}

// ============================================================================
// 8. INITIALIZE RELAYS
// ============================================================================

void initializeRelays() {
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {

    // Set the OFF output level before enabling output mode.
    digitalWrite(RELAY_PINS[i], RELAY_OFF_LEVEL);
    pinMode(RELAY_PINS[i], OUTPUT);
  }

  Serial.println(F("[RELAY] All relays initialized to OFF."));
}

// ============================================================================
// 9. CENTRALIZED RELAY CONTROL
// ============================================================================

void setRelayState(uint8_t relayNumber, bool state) {

  // Validate relay index.
  if (relayNumber < 1 || relayNumber > RELAY_COUNT) {
    Serial.println(F("[RELAY][ERROR] Invalid relay number."));
    return;
  }

  const uint8_t pin = RELAY_PINS[relayNumber - 1];

  digitalWrite(
    pin,
    state ? RELAY_ON_LEVEL : RELAY_OFF_LEVEL
  );

  Serial.print(F("[RELAY] Relay "));
  Serial.print(relayNumber);
  Serial.print(F(" -> "));
  Serial.println(state ? F("ON") : F("OFF"));
}

// ============================================================================
// 10. READ AND PUBLISH DHT22 DATA
// ============================================================================

void updateSensorData() {

  const float newTemperature = dht.readTemperature();
  const float newHumidity = dht.readHumidity();

  const bool temperatureValid = !isnan(newTemperature);
  const bool humidityValid = !isnan(newHumidity);

  // Update temperature only when the reading is valid.
  if (temperatureValid) {
    temperature = newTemperature;
  } else {
    Serial.println(
      F("[SENSOR][ERROR] Failed to read temperature.")
    );
  }

  // Update humidity only when the reading is valid.
  if (humidityValid) {
    humidity = newHumidity;
  } else {
    Serial.println(
      F("[SENSOR][ERROR] Failed to read humidity.")
    );
  }

  // Print successfully acquired measurements.
  if (temperatureValid) {
    Serial.print(F("[SENSOR] Temperature: "));
    Serial.print(newTemperature, 1);
    Serial.println(F(" C"));
  }

  if (humidityValid) {
    Serial.print(F("[SENSOR] Humidity: "));
    Serial.print(newHumidity, 1);
    Serial.println(F(" %"));
  }
}

// ============================================================================
// 11. ARDUINO IOT CLOUD CALLBACKS
// ============================================================================

void onRelay1Change() {
  setRelayState(1, relay1);
}

void onRelay2Change() {
  setRelayState(2, relay2);
}

void onRelay3Change() {
  setRelayState(3, relay3);
}

void onRelay4Change() {
  setRelayState(4, relay4);
}

// ============================================================================
// 12. STARTUP INFORMATION
// ============================================================================

void printStartupInformation() {
  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("  Arduino IoT Cloud Smart Relay System"));
  Serial.println(F("========================================"));

#if defined(USE_ESP8266)
  Serial.println(F("[SYSTEM] Board: ESP8266 NodeMCU"));
#elif defined(USE_ESP32)
  Serial.println(F("[SYSTEM] Board: ESP32"));
#endif

  Serial.println(F("[SYSTEM] Sensor: DHT22"));
  Serial.println(F("[SYSTEM] Relay count: 4"));
  Serial.println(F("[SYSTEM] Relay logic: Active-LOW"));
  Serial.println(F("[SYSTEM] Sensor interval: 5000 ms"));
  Serial.println(F("========================================"));
  Serial.println();
}
