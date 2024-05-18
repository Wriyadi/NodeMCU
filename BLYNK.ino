#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6MVxxxx"
#define BLYNK_TEMPLATE_NAME "Project WillyR"
#define BLYNK_AUTH_TOKEN "UUvGgg3kxxxxxx"
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

#include <ESP8266WiFi.h>
#include <DHT.h>
#include <SPI.h>
#include <BlynkSimpleEsp8266.h>

char ssid[] = "ssid";
char pass[] = "password";

BLYNK_WRITE(V0)
{
  if(param.asInt() == 1)
  {
    digitalWrite(16,HIGH);
  }
  else
  {
    digitalWrite(16,LOW);
  }
}

BLYNK_WRITE(V1)
{
  if(param.asInt() == 1)
  {
    digitalWrite(5,HIGH);
  }
  else
  {
    digitalWrite(5,LOW);
  }
}

BLYNK_WRITE(V2)
{
  if(param.asInt() == 1)
  {
    digitalWrite(4,HIGH);
  }
  else
  {
    digitalWrite(4,LOW);
  }
}

BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
}

void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V4, h);
  Blynk.virtualWrite(V3, t);
}

void setup()
{
  // Debug console
  Serial.begin(115200);
  pinMode(16, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(2, INPUT);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, sendSensor);
}

void loop()
{
  Blynk.run();
  timer.run();
}
