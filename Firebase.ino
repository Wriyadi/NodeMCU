#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
 
const char* ssid = "SSID";
const char* password = "PASSWORD";
 
FirebaseData firebaseData;
 
#define LED1 D1
#define LED2 D2
#define LED3 D3
 
void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  digitalWrite(LED1, LOW); //nilai awal LED1 akan off
  digitalWrite(LED2, LOW); //nilai awal LED2 akan off
  digitalWrite(LED3, LOW); //nilai awal LED3 akan off
  
  ConnectWifi();
  Firebase.begin("https://esp8266-60018-default-rtdb.asia-southeast1.firebasedatabase.app/", "AIzaSyDBLxxx");
  //Firebase.begin("firebase host", "firebase auth database");
}
 
void ConnectWifi() {
  WiFi.begin(ssid, password);
  //memulai menghubungkan ke wifi router
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); //status saat mengkoneksikan
  }
  Serial.println("Sukses terkoneksi Wifi!");
  Serial.println("IP Address:"); //alamat ip lokal
  Serial.println(WiFi.localIP());
}
 
void loop() {
  if (Firebase.getString(firebaseData, "/Led1")) { //misal database diberikan nama LED1
    if  (firebaseData.dataType() == "string") 
    {
      String FBStatus = firebaseData.stringData();
      if (FBStatus == "ON") {                                                         
      Serial.println("LED1 ON");                         
      digitalWrite(LED1, HIGH); }
        else if (FBStatus == "OFF") {                                                  
        Serial.println("LED1 OFF");
        digitalWrite(LED1, LOW);                                                
          }
      else {Serial.println("Salah kode! isi dengan data ON/OFF");}
    }
  }
  if (Firebase.getString(firebaseData, "/Led2")) { //misal database diberikan nama LED2
    if  (firebaseData.dataType() == "string") 
    {
      String FBStatus = firebaseData.stringData();
      if (FBStatus == "ON") {                                                         
      Serial.println("LED2 ON");                         
      digitalWrite(LED2, HIGH); }
        else if (FBStatus == "OFF") {                                                  
        Serial.println("LED2 OFF");
        digitalWrite(LED2, LOW);                                                
          }
      else {Serial.println("Salah kode! isi dengan data ON/OFF");}
    }
  }
 if (Firebase.getString(firebaseData, "/Led3")) { //misal database diberikan nama LED3
    if  (firebaseData.dataType() == "string") 
    {
      String FBStatus = firebaseData.stringData();
      if (FBStatus == "ON") {                                                         
      Serial.println("LED3 ON");                         
      digitalWrite(LED3, HIGH); }
        else if (FBStatus == "OFF") {                                                  
        Serial.println("LED3 OFF");
        digitalWrite(LED3, LOW);                                                
          }
      else {Serial.println("Salah kode! isi dengan data ON/OFF");}
    }
  }
}
