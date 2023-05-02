#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
 
const char* ssid = "SSID";
const char* password = "PASSWORD";
 
FirebaseData firebaseData;
 
#define LED1 D1
#define LED2 D2
 
void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED1, LOW); //nilai awal LED1 akan off
  digitalWrite(LED2, LOW); //nilai awal LED2 akan off
  
  ConnectWifi();
  Firebase.begin("https://esp8266-61098-default-rtdb.asia-southeast1.firebasedatabase.app/", "AIzaSyDBLmz76dnI7");
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
}
