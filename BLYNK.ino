// Makro BLYNK WAJIB diletakkan paling atas sebelum library apa pun
#define BLYNK_TEMPLATE_ID "TMPL6MVxxxx"
#define BLYNK_TEMPLATE_NAME "Project WillyR"
#define BLYNK_AUTH_TOKEN "UUvGgg3kxxxxxx"
#define BLYNK_PRINT Serial

// --- PEMILIHAN LIBRARY UNIVERSAL ---
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <BlynkSimpleEsp8266.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <BlynkSimpleEsp32.h>
#endif

#include <DHT.h>
#include <SPI.h>

// --- KONFIGURASI PIN UNIVERSAL ---
#if defined(ESP8266)
  #define DHTPIN 2
  #define RELAY1_PIN 16
  #define RELAY2_PIN 5
  #define RELAY3_PIN 4
  #define RELAY4_PIN 0
#elif defined(ESP32)
  #define DHTPIN 4
  #define RELAY1_PIN 26
  #define RELAY2_PIN 27
  #define RELAY3_PIN 32
  #define RELAY4_PIN 33
#endif

#define DHTTYPE DHT22

// --- KONFIGURASI JARINGAN ---
char ssid[] = "ssid";
char pass[] = "password";

// --- INISIALISASI OBJEK ---
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

// Logika Relay NPN (Ubah ke HIGH/LOW jika menggunakan Relay PNP)
#define RELAY_ON LOW
#define RELAY_OFF HIGH

// --- FUNGSI KONTROL RELAY BLYNK ---
// Menerima nilai 1 (ON) atau 0 (OFF) dari tombol di aplikasi Blynk
BLYNK_WRITE(V0) { digitalWrite(RELAY1_PIN, param.asInt() ? RELAY_ON : RELAY_OFF); }
BLYNK_WRITE(V1) { digitalWrite(RELAY2_PIN, param.asInt() ? RELAY_ON : RELAY_OFF); }
BLYNK_WRITE(V2) { digitalWrite(RELAY3_PIN, param.asInt() ? RELAY_ON : RELAY_OFF); }
BLYNK_WRITE(V3) { digitalWrite(RELAY4_PIN, param.asInt() ? RELAY_ON : RELAY_OFF); }

// Sinkronisasi status terakhir dari server Blynk saat perangkat restart
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
}

// --- FUNGSI SENSOR DHT ---
void sendSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // Celcius

  if (isnan(h) || isnan(t)) {
    Serial.println("Gagal membaca dari sensor DHT!");
    return;
  }
  
  // Mengirim data ke Virtual Pin Blynk
  Blynk.virtualWrite(V4, h); // Kelembaban ke V4
  Blynk.virtualWrite(V5, t); // Suhu ke V5 (DIPERBAIKI: Sebelumnya tabrakan di V3)
}

void setup() {
  Serial.begin(115200);

  // Inisialisasi Sensor DHT (BUG FIXED)
  dht.begin();

  // Inisialisasi Pin Relay
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  
  // Set kondisi awal relay ke posisi mati
  digitalWrite(RELAY1_PIN, RELAY_OFF);
  digitalWrite(RELAY2_PIN, RELAY_OFF);
  digitalWrite(RELAY3_PIN, RELAY_OFF);
  digitalWrite(RELAY4_PIN, RELAY_OFF);

  // Mulai koneksi Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Setup timer untuk membaca sensor setiap 2 detik (2000 ms)
  // DHT22 butuh waktu ideal minimal 2 detik antar pembacaan agar stabil
  timer.setInterval(2000L, sendSensor);
}

void loop() {
  Blynk.run();
  timer.run();
}
