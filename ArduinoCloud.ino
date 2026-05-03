/*
=====================================================
Arduino IoT Cloud + 4 Relay + DHT22
Support:
1. ESP8266 (NodeMCU)
2. ESP32

Fungsi:
- Monitoring suhu & kelembaban DHT22
- Kontrol 4 Relay dari Arduino IoT Cloud

Catatan:
Pastikan pada Arduino IoT Cloud sudah dibuat:
1. temperature  -> float (Read Only)
2. humidity     -> float (Read Only)
3. relay1       -> bool
4. relay2       -> bool
5. relay3       -> bool
6. relay4       -> bool

dan file thingProperties.h sudah sesuai
=====================================================
*/

#include <DHT.h>
#include "thingProperties.h"   // File dari Arduino IoT Cloud

/*
=====================================================
PILIH BOARD
=====================================================
Gunakan salah satu:

#define USE_ESP8266
atau
#define USE_ESP32
=====================================================
*/

#define USE_ESP8266
// #define USE_ESP32


/*
=====================================================
PIN SETTING
=====================================================
*/

#ifdef USE_ESP8266
  // NodeMCU ESP8266
  #define RELAY1 16   // D0
  #define RELAY2 5    // D1
  #define RELAY3 4    // D2
  #define RELAY4 0    // D3

  #define DHTPIN 12   // D6
#endif

#ifdef USE_ESP32
  // ESP32
  #define RELAY1 23
  #define RELAY2 22
  #define RELAY3 21
  #define RELAY4 19

  #define DHTPIN 4
#endif

#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);


/*
=====================================================
SENSOR INTERVAL
=====================================================
*/

unsigned long lastSensorReadMillis = 0;
const long SENSOR_READ_INTERVAL = 5000; // 5 detik


/*
=====================================================
SETUP
=====================================================
*/

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("System Starting...");

  /*
  =====================================
  Inisialisasi Relay
  =====================================
  */

  pinMode(RELAY1, OUTPUT);
  digitalWrite(RELAY1, LOW);

  pinMode(RELAY2, OUTPUT);
  digitalWrite(RELAY2, LOW);

  pinMode(RELAY3, OUTPUT);
  digitalWrite(RELAY3, LOW);

  pinMode(RELAY4, OUTPUT);
  digitalWrite(RELAY4, LOW);

  /*
  =====================================
  Start DHT
  =====================================
  */

  dht.begin();

  /*
  =====================================
  Arduino IoT Cloud
  =====================================
  */

  initProperties();

  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  Serial.println("System Ready");
}


/*
=====================================================
LOOP
=====================================================
*/

void loop() {
  // Wajib dipanggil terus
  ArduinoCloud.update();

  // Update sensor tiap 5 detik
  if (millis() - lastSensorReadMillis >= SENSOR_READ_INTERVAL) {
    updateSensorData();
    lastSensorReadMillis = millis();
  }
}


/*
=====================================================
UPDATE SENSOR DHT22
=====================================================
*/

void updateSensorData() {
  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  // Temperatur
  if (!isnan(temp)) {
    temperature = temp;

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
  } else {
    Serial.println("Gagal membaca suhu dari DHT22");
  }

  // Humidity
  if (!isnan(hum)) {
    humidity = hum;

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  } else {
    Serial.println("Gagal membaca kelembaban dari DHT22");
  }
}


/*
=====================================================
CALLBACK RELAY
Dipanggil otomatis saat nilai cloud berubah
=====================================================
*/

void onRelay1Change() {
  digitalWrite(RELAY1, relay1 ? HIGH : LOW);

  Serial.print("Relay 1: ");
  Serial.println(relay1 ? "ON" : "OFF");
}

void onRelay2Change() {
  digitalWrite(RELAY2, relay2 ? HIGH : LOW);

  Serial.print("Relay 2: ");
  Serial.println(relay2 ? "ON" : "OFF");
}

void onRelay3Change() {
  digitalWrite(RELAY3, relay3 ? HIGH : LOW);

  Serial.print("Relay 3: ");
  Serial.println(relay3 ? "ON" : "OFF");
}

void onRelay4Change() {
  digitalWrite(RELAY4, relay4 ? HIGH : LOW);

  Serial.print("Relay 4: ");
  Serial.println(relay4 ? "ON" : "OFF");
}
