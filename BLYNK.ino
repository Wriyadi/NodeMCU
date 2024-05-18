#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6MVxxxx"
#define BLYNK_TEMPLATE_NAME "Project WillyR"
#define BLYNK_AUTH_TOKEN "UUvGgg3kxxxxxx"
#include <ESP8266WiFi.h>
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

void setup()
{
  // Debug console
  Serial.begin(115200);
  pinMode(16, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

}

void loop()
{
  Blynk.run();
}
