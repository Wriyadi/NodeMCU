// ==========================================
// DETEKSI BOARD OTOMATIS (ESP8266 / ESP32)
// ==========================================
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#endif

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>

// ==========================================
// BAGIAN 1: KONFIGURASI WIFI & BROKER MQTT
// ==========================================
const char* ssid = "NAMA_WIFI_ANDA";        
const char* password = "PASSWORD_WIFI";     

// Ganti URL ini sesuai dengan Cluster EMQX Cloud atau HiveMQ Cloud
const char* mqtt_server = "id-cluster-anda.s1.eu.hivemq.cloud"; 

// Port SSL/TLS (HiveMQ Cloud dan EMQX Cloud menggunakan 8883)
const int mqtt_port = 8883; 

// Username & Password dari Access Management Broker
const char* mqtt_user = "username_mqtt"; 
const char* mqtt_pass = "password_mqtt";

// ==========================================
// BAGIAN 2: KONFIGURASI PIN RELAY & DHT22
// ==========================================
// Menggunakan nomor GPIO agar kompatibel di kedua board
#define RELAY1_PIN 5  // ESP8266: D1 | ESP32: GPIO 5
#define RELAY2_PIN 4  // ESP8266: D2 | ESP32: GPIO 4
#define RELAY3_PIN 14 // ESP8266: D5 | ESP32: GPIO 14
#define RELAY4_PIN 12 // ESP8266: D6 | ESP32: GPIO 12

#define DHTPIN 13     // ESP8266: D7 | ESP32: GPIO 13
#define DHTTYPE DHT22 // Tipe Sensor DHT

DHT dht(DHTPIN, DHTTYPE);

// ==========================================
// BAGIAN 3: TOPIK MQTT
// ==========================================
const char* topic_relay1 = "kontrol/lampu1";
const char* topic_relay2 = "kontrol/lampu2";
const char* topic_relay3 = "kontrol/lampu3";
const char* topic_relay4 = "kontrol/lampu4";
const char* topic_suhu = "sensor/suhu";
const char* topic_kelembaban = "sensor/kelembaban";

WiFiClientSecure espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0; // Timer untuk DHT22

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Setup Pin Relay
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);

  // Matikan Relay saat awal (Logic Active LOW: HIGH = Mati)
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);
  digitalWrite(RELAY3_PIN, HIGH);
  digitalWrite(RELAY4_PIN, HIGH);

  setup_wifi();

  // Mode Insecure: Bypass pengecekan sertifikat SSL
  espClient.setInsecure(); 

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Terhubung!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Fungsi saat pesan diterima dari MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Pesan diterima di topik [");
  Serial.print(topic);
  Serial.print("]: ");

  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  Serial.println();

  // Fungsi Helper untuk kontrol relay
  kontrolRelay(topic, messageTemp, topic_relay1, RELAY1_PIN, 1);
  kontrolRelay(topic, messageTemp, topic_relay2, RELAY2_PIN, 2);
  kontrolRelay(topic, messageTemp, topic_relay3, RELAY3_PIN, 3);
  kontrolRelay(topic, messageTemp, topic_relay4, RELAY4_PIN, 4);
}

// Fungsi agar kode tidak berulang (DRY)
void kontrolRelay(String reqTopic, String msg, const char* targetTopic, int pin, int relayNum) {
  if (reqTopic == targetTopic) {
    if(msg == "ON" || msg == "1") {
      digitalWrite(pin, LOW); // Nyala
      Serial.printf("-> Relay %d ON\n", relayNum);
    }
    else if(msg == "OFF" || msg == "0") {
      digitalWrite(pin, HIGH); // Mati
      Serial.printf("-> Relay %d OFF\n", relayNum);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT Broker...");
    
    // Buat Client ID Unik berdasarkan Board
    #if defined(ESP8266)
      String clientId = "ESP8266-";
    #elif defined(ESP32)
      String clientId = "ESP32-";
    #endif
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("BERHASIL TERHUBUNG!");
      
      // Subscribe topik
      client.subscribe(topic_relay1);
      client.subscribe(topic_relay2);
      client.subscribe(topic_relay3);
      client.subscribe(topic_relay4);
    } else {
      Serial.print("Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi 5 detik...");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Pengiriman data DHT22 setiap 5 detik (Tanpa delay/blocking)
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (isnan(t) || isnan(h)) {
      Serial.println("Gagal membaca dari sensor DHT22!");
      return;
    }

    Serial.printf("Suhu: %.2f °C | Kelembaban: %.2f %%\n", t, h);
    
    // Publish ke broker MQTT
    client.publish(topic_suhu, String(t).c_str());
    client.publish(topic_kelembaban, String(h).c_str());
  }
}
