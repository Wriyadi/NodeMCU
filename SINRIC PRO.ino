#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <SinricPro.h>
#include <SinricProSwitch.h>

#define RELAYPIN_1 D1
#define RELAYPIN_2 D2
#define RELAYPIN_3 D3

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
}

void loop() {
  SinricPro.handle();
}
