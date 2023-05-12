#define BLYNK_AUTH_TOKEN            "HOC1HLpxxx"
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "SSID";
char pass[] = "PASSWORD";

// This function is called every time the Virtual Pin 1 state changes
BLYNK_WRITE(V1) // Executes when the value of virtual pin 1 changes
{
  if(param.asInt() == 1)
  {
    // execute this code if the switch widget is now ON
    digitalWrite(D1,HIGH);  // Set digital pin 2 HIGH
  }
  else
  {
    // execute this code if the switch widget is now OFF
    digitalWrite(D1,LOW);  // Set digital pin 2 LOW    
  }
}

BLYNK_WRITE(V2) // Executes when the value of virtual pin 2 changes
{
  if(param.asInt() == 1)
  {
    // execute this code if the switch widget is now ON
    digitalWrite(D2,HIGH);  // Set digital pin 2 HIGH
  }
  else
  {
    // execute this code if the switch widget is now OFF
    digitalWrite(D2,LOW);  // Set digital pin 2 LOW    
  }
}

BLYNK_WRITE(V3) // Executes when the value of virtual pin 3 changes
{
  if(param.asInt() == 1)
  {
    // execute this code if the switch widget is now ON
    digitalWrite(D3,HIGH);  // Set digital pin 2 HIGH
  }
  else
  {
    // execute this code if the switch widget is now OFF
    digitalWrite(D3,LOW);  // Set digital pin 2 LOW    
  }
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
}

void setup()
{
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  }

void loop()
{
  Blynk.run();
}
