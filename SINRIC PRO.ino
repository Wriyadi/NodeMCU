#ifdef ENABLE_DEBUG
  #define DEBUG_ESP_PORT Serial
  #define NODEBUG_WEBSOCKETS
  #define NDEBUG
#endif

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <SinricPro.h>
#include <SinricProTemperaturesensor.h">
#include <SinricProSwitch.h>
#define DHT_PIN 2
#define RELAYPIN_1 16
#define RELAYPIN_2 5
#define RELAYPIN_3 4
#define TEMP_SENSOR_ID "643667f39xxxx"
#define EVENT_WAIT_TIME 10000  // send event every 10 seconds

float temperature;
float humidity;
float lastTemperature;                        // last known temperature (for compare)
float lastHumidity;                           // last known humidity (for compare)
DHT dht;

/*****************
 * Configuration *
 *****************/

struct RelayInfo {
  String deviceId;
  String name;
  int pin;
};

std::vector<RelayInfo> relays = {
    {"643667f3918a3c91cxxx", "Relay 1", RELAYPIN_1},
    {"643668a3312d40edcxxx", "Relay 2", RELAYPIN_2},
    {"643668cd918a3c911xxx", "Relay 3", RELAYPIN_3},
};
/*   ^^^^^^^^^^^^^^^^^^^^^^^^^   ^^^^^^^^^  ^^^
 *              |                     |      |
 *              |                     |      +---> digital PIN or GPIO number (see Note below!)
 *              |                     +----------> Name that will be printed to serial monitor
 *              +--------------------------------> deviceId
 * 
 *  In the vector above, you can add as many relays you want to have
 *  This is only limited to:
 *    - the number of SinricPro devices you have available
 *    - the number of pins / GPIOs your board have
 *
 *  Note: Some GPIO's are set to specific level when the board boots up
 *        This might result in strange behavior if there are relays connected to those pins
 *        Check your board documentation!
 */

#define WIFI_SSID  "SSID"
#define WIFI_PASS  "PASSWORD"
#define APP_KEY    "ae5f7d54-9a3c-4fea-8f64-xxx"    // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET "21f85ec4-f0d4-4fd7-b7fd-xxx" // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"

#define BAUD_RATE  115200              // Change baudrate to your need

void handleTemperaturesensor() {
  if (SinricPro.isConnected() == false) {
    Serial.printf("Not connected to Sinric Pro...!\r\n");
    return; 
  }

  static unsigned long last_millis;
  unsigned long        current_millis = millis();
  if (last_millis && current_millis - last_millis < EVENT_WAIT_TIME) return;
  last_millis = current_millis;
  
  temperature = dht.getTemperature();          // get actual temperature in °C
//  temperature = dht.getTemperature() * 1.8f + 32;  // get actual temperature in °F
  humidity = dht.getHumidity();                // get actual humidity

  if (isnan(temperature) || isnan(humidity)) { // reading failed... 
    Serial.printf("DHT reading failed!\r\n");  // print error message
    return;                                    // try again next time
  } 

  Serial.printf("Temperature: %2.1f Celsius\tHumidity: %2.1f%%\r\n", temperature, humidity);

  if (temperature == lastTemperature && humidity == lastHumidity) {
    Serial.printf("Temperature did not changed. do nothing...!\r\n");
    return; 
  }

  SinricProTemperaturesensor &mySensor = SinricPro[TEMP_SENSOR_ID];  // get temperaturesensor device
  bool success = mySensor.sendTemperatureEvent(temperature, humidity); // send event
  if (success) {  
    Serial.printf("Sent!\r\n");
  } else {
    Serial.printf("Something went wrong...could not send Event to server!\r\n"); // Enable ENABLE_DEBUG to see why
  }

  lastTemperature = temperature;  // save actual temperature for next compare
  lastHumidity = humidity;        // save actual humidity for next compare 
}

void setupRelayPins() {
  for (auto &relay : relays) {    // for each relay configuration
    pinMode(relay.pin, OUTPUT);     // set pinMode to OUTPUT
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
  for (auto &relay : relays) {                                                            // for each relay configuration
    if (deviceId == relay.deviceId) {                                                       // check if deviceId matches
      Serial.printf("Device %s turned %s\r\n", relay.name.c_str(), state ? "on" : "off");     // print relay name and state to serial
      digitalWrite(relay.pin, state);                                                         // set state to digital pin / gpio
      return true;                                                                            // return with success true
    }
  }
  return false; // if no relay configuration was found, return false
}

void setupSinricPro() {
  for (auto &relay : relays) {                             // for each relay configuration
    SinricProSwitch &mySwitch = SinricPro[relay.deviceId];   // create a new device with deviceId from relay configuration
    mySwitch.onPowerState(onPowerState);                     // attach onPowerState callback to the new device
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
  dht.setup(DHT_PIN);
}

void loop() {
  SinricPro.handle();
  handleTemperaturesensor();   
}
