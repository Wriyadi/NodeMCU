#ifdef ENABLE_DEBUG
  #define DEBUG_ESP_PORT Serial
  #define NODEBUG_WEBSOCKETS
  #define NDEBUG
#endif

#include <Arduino.h>

// --- DETEKSI OTOMATIS ARSITEKTUR BOARD ---
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif
// -----------------------------------------

#include <DHT.h>
#include <SinricPro.h>
#include <SinricProThermostat.h> 
#include <SinricProSwitch.h>

// Catatan Pin: Pastikan pin ini aman digunakan untuk board spesifik Anda
#define DHTPIN 2
#define DHTTYPE DHT22

#define RELAYPIN_1 16
#define RELAYPIN_2 5
#define RELAYPIN_3 4
#define EVENT_WAIT_TIME 10000  // Kirim event setiap 10 detik

float temperature;
float humidity;
float lastTemperature;                        
float lastHumidity;                           
DHT dht(DHTPIN, DHTTYPE);

/*****************
 * Configuration *
 *****************/

struct RelayInfo {
  String deviceId;
  String name;
  int pin;
};

std::vector<RelayInfo> relays = {
    {"ID_THERMOSTAT_BARU_ANDA", "Relay 1 & Suhu", RELAYPIN_1}, 
    {"643668a3312d40edcxxx", "Relay 2", RELAYPIN_2},
    {"643668cd918a3c911xxx", "Relay 3", RELAYPIN_3},
};

#define WIFI_SSID  "SSID"
#define WIFI_PASS  "PASSWORD"
#define APP_KEY    "ae5f7d54-9a3c-4fea-8f64-xxx"    
#define APP_SECRET "21f85ec4-f0d4-4fd7-b7fd-xxx" 

#define BAUD_RATE  115200              

void handleTemperaturesensor() {
  if (!SinricPro.isConnected()) {
    return; 
  }

  static unsigned long last_millis;
  unsigned long current_millis = millis();
  if (last_millis && current_millis - last_millis < EVENT_WAIT_TIME) return;
  last_millis = current_millis;
  
  temperature = dht.readTemperature();          
  humidity = dht.readHumidity();                

  if (isnan(temperature) || isnan(humidity)) { 
    Serial.printf("DHT reading failed!\r\n");  
    return;                                    
  } 

  if (temperature == lastTemperature && humidity == lastHumidity) {
    return; 
  }

  Serial.printf("Temperature: %2.1f Celsius\tHumidity: %2.1f%%\r\n", temperature, humidity);

  SinricProThermostat &myThermostat = SinricPro[relays[0].deviceId];  
  bool success = myThermostat.sendTemperatureEvent(temperature, humidity); 
  
  if (success) {  
    Serial.printf("Sent!\r\n");
  } else {
    Serial.printf("Something went wrong...could not send Event to server!\r\n"); 
  }

  lastTemperature = temperature;  
  lastHumidity = humidity;        
}

void setupRelayPins() {
  for (auto &relay : relays) {    
    pinMode(relay.pin, OUTPUT);     
  }
}

void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

bool onPowerState(const String &deviceId, bool &state) {
  for (auto &relay : relays) {                                                        
    if (deviceId == relay.deviceId) {                                                 
      Serial.printf("Device %s turned %s\r\n", relay.name.c_str(), state ? "on" : "off");     
      digitalWrite(relay.pin, state);                                                   
      return true;                                                                    
    }
  }
  return false; 
}

void setupSinricPro() {
  // 1. Setup Relay 1 sebagai Thermostat (Suhu + Switch)
  SinricProThermostat &thermostat = SinricPro[relays[0].deviceId];
  thermostat.onPowerState(onPowerState); 

  // 2. Setup Relay 2 dan Relay 3 sebagai Switch standar
  for (size_t i = 1; i < relays.size(); i++) {                             
    SinricProSwitch &mySwitch = SinricPro[relays[i].deviceId];   
    mySwitch.onPowerState(onPowerState);                     
  }

  SinricPro.onConnected([]() { Serial.printf("Connected to SinricPro\r\n"); });
  SinricPro.onDisconnected([]() { Serial.printf("Disconnected from SinricPro\r\n"); });

  SinricPro.begin(APP_KEY, APP_SECRET);
}

void setup() {
  Serial.begin(BAUD_RATE);
  setupRelayPins();
  setupWiFi();
  setupSinricPro();
  dht.begin(); 
}

void loop() {
  SinricPro.handle();
  handleTemperaturesensor();   
}
