/*
  ESP8266 / ESP32 Web Server
  Kontrol 4 Relay + Monitoring DHT22

  Fitur:
  - Web control 4 relay
  - Monitoring suhu dan kelembapan DHT22
  - Status relay melalui HTTP API
  - Mendukung ESP8266 dan ESP32
*/

#include <DHT.h>
#include <math.h>

// ==========================================
// 1. PILIH BOARD OTOMATIS
// ==========================================

#if defined(ESP8266)

  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>

  ESP8266WebServer server(80);

  // NodeMCU ESP8266: D1, D2, D3, D4
  const uint8_t relayPins[4] = {5, 4, 0, 2};

  // D5
  const uint8_t DHT_PIN = 14;

#elif defined(ESP32)

  #include <WiFi.h>
  #include <WebServer.h>

  WebServer server(80);

  const uint8_t relayPins[4] = {16, 17, 18, 19};
  const uint8_t DHT_PIN = 4;

#else
  #error "Pilih board ESP8266 atau ESP32 di Arduino IDE"
#endif

// ==========================================
// 2. KONFIGURASI WIFI
// ==========================================

const char* ssid     = "hotspot@unama";
const char* password = "";

// ==========================================
// 3. KONFIGURASI RELAY
// ==========================================

// true  = relay aktif ketika pin LOW
// false = relay aktif ketika pin HIGH
const bool RELAY_ACTIVE_LOW = true;

const uint8_t RELAY_COUNT = 4;
bool relayState[RELAY_COUNT] = {false, false, false, false};

// ==========================================
// 4. KONFIGURASI DHT22
// ==========================================

#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

float temperatureC = NAN;
float humidity = NAN;

unsigned long lastDhtRead = 0;
const unsigned long DHT_INTERVAL_MS = 2500;
bool dhtReadOnce = false;

// ==========================================
// 5. STATUS WIFI DAN SERVER
// ==========================================

bool serverStarted = false;
unsigned long lastWiFiRetry = 0;

// ==========================================
// 6. HALAMAN WEB
// ==========================================

const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
  <meta charset="UTF-8">
  <meta name="viewport"
        content="width=device-width, initial-scale=1.0">

  <title>ESP IoT Control Panel</title>

  <style>
    * {
      box-sizing: border-box;
    }

    body {
      font-family: Arial, sans-serif;
      background: #f1f5f9;
      color: #1e293b;
      margin: 0;
      padding: 20px;
    }

    .container {
      max-width: 850px;
      margin: auto;
    }

    h1 {
      text-align: center;
      color: #0f172a;
    }

    .subtitle {
      text-align: center;
      color: #64748b;
      margin-bottom: 25px;
    }

    .sensor-grid,
    .relay-grid {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 15px;
    }

    .card {
      background: white;
      padding: 20px;
      border-radius: 14px;
      box-shadow: 0 3px 12px rgba(0,0,0,0.07);
      margin-bottom: 15px;
    }

    .sensor-label {
      color: #64748b;
      font-size: 14px;
    }

    .sensor-value {
      font-size: 30px;
      font-weight: bold;
      margin-top: 10px;
    }

    .unit {
      font-size: 16px;
      color: #64748b;
    }

    h2 {
      font-size: 20px;
      margin-top: 0;
    }

    .relay-status {
      font-weight: bold;
      margin: 12px 0;
    }

    .on {
      color: #16a34a;
    }

    .off {
      color: #dc2626;
    }

    button {
      border: none;
      border-radius: 8px;
      padding: 11px 20px;
      margin: 4px;
      font-size: 14px;
      font-weight: bold;
      cursor: pointer;
      color: white;
    }

    .btn-on {
      background: #16a34a;
    }

    .btn-off {
      background: #dc2626;
    }

    button:disabled {
      opacity: 0.5;
      cursor: wait;
    }

    .footer {
      text-align: center;
      color: #64748b;
      font-size: 12px;
      margin-top: 20px;
    }

    @media (max-width: 480px) {
      body {
        padding: 12px;
      }

      .sensor-grid,
      .relay-grid {
        grid-template-columns: 1fr;
      }
    }
  </style>
</head>

<body>
<div class="container">

  <h1>ESP IoT Control Panel</h1>
  <p class="subtitle">
    Kontrol 4 Relay dan Monitoring DHT22
  </p>

  <h2>Monitoring Sensor</h2>

  <div class="sensor-grid">
    <div class="card">
      <div class="sensor-label">Suhu</div>
      <div class="sensor-value">
        <span id="temperature">--</span>
        <span class="unit">°C</span>
      </div>
    </div>

    <div class="card">
      <div class="sensor-label">Kelembapan</div>
      <div class="sensor-value">
        <span id="humidity">--</span>
        <span class="unit">%</span>
      </div>
    </div>
  </div>

  <h2>Kontrol Relay</h2>

  <div class="relay-grid">

    <div class="card">
      <h2>Relay 1</h2>
      <div class="relay-status off" id="status1">OFF</div>
      <button class="btn-on" onclick="setRelay(1,1)">ON</button>
      <button class="btn-off" onclick="setRelay(1,0)">OFF</button>
    </div>

    <div class="card">
      <h2>Relay 2</h2>
      <div class="relay-status off" id="status2">OFF</div>
      <button class="btn-on" onclick="setRelay(2,1)">ON</button>
      <button class="btn-off" onclick="setRelay(2,0)">OFF</button>
    </div>

    <div class="card">
      <h2>Relay 3</h2>
      <div class="relay-status off" id="status3">OFF</div>
      <button class="btn-on" onclick="setRelay(3,1)">ON</button>
      <button class="btn-off" onclick="setRelay(3,0)">OFF</button>
    </div>

    <div class="card">
      <h2>Relay 4</h2>
      <div class="relay-status off" id="status4">OFF</div>
      <button class="btn-on" onclick="setRelay(4,1)">ON</button>
      <button class="btn-off" onclick="setRelay(4,0)">OFF</button>
    </div>

  </div>

  <div class="footer">
    ESP8266 / ESP32 Web Server | Pembaruan otomatis
  </div>

</div>

<script>
  async function updateStatus() {
    try {
      const response = await fetch('/api/status', {
        cache: 'no-store'
      });

      if (!response.ok) {
        throw new Error('Gagal membaca status');
      }

      const data = await response.json();

      document.getElementById('temperature').textContent =
        data.temperature === null
          ? '--'
          : Number(data.temperature).toFixed(1);

      document.getElementById('humidity').textContent =
        data.humidity === null
          ? '--'
          : Number(data.humidity).toFixed(1);

      data.relays.forEach((isOn, index) => {
        const statusElement =
          document.getElementById('status' + (index + 1));

        statusElement.textContent = isOn ? 'ON' : 'OFF';
        statusElement.className =
          'relay-status ' + (isOn ? 'on' : 'off');
      });

    } catch (error) {
      console.error('Status error:', error);
    }
  }

  async function setRelay(channel, state) {
    try {
      const response = await fetch(
        '/relay?ch=' + channel + '&state=' + state,
        { cache: 'no-store' }
      );

      if (!response.ok) {
        throw new Error('Perintah relay gagal');
      }

      const data = await response.json();

      data.relays.forEach((isOn, index) => {
        const statusElement =
          document.getElementById('status' + (index + 1));

        statusElement.textContent = isOn ? 'ON' : 'OFF';
        statusElement.className =
          'relay-status ' + (isOn ? 'on' : 'off');
      });

      document.getElementById('temperature').textContent =
        data.temperature === null
          ? '--'
          : Number(data.temperature).toFixed(1);

      document.getElementById('humidity').textContent =
        data.humidity === null
          ? '--'
          : Number(data.humidity).toFixed(1);

    } catch (error) {
      alert('Gagal mengirim perintah. Periksa koneksi Wi-Fi.');
      console.error(error);
    }
  }

  updateStatus();
  setInterval(updateStatus, 2000);
</script>
</body>
</html>
)rawliteral";

// ==========================================
// 7. FUNGSI KONTROL RELAY
// ==========================================

void writeRelay(uint8_t index, bool turnOn) {
  if (index >= RELAY_COUNT) {
    return;
  }

  relayState[index] = turnOn;

  uint8_t outputLevel;

  if (RELAY_ACTIVE_LOW) {
    outputLevel = turnOn ? LOW : HIGH;
  } else {
    outputLevel = turnOn ? HIGH : LOW;
  }

  digitalWrite(relayPins[index], outputLevel);
}

// ==========================================
// 8. MEMBACA SENSOR DHT22
// ==========================================

void updateDHT() {
  unsigned long now = millis();

  if (!dhtReadOnce ||
      now - lastDhtRead >= DHT_INTERVAL_MS) {

    lastDhtRead = now;
    dhtReadOnce = true;

    float newHumidity = dht.readHumidity();
    float newTemperature = dht.readTemperature();

    if (isnan(newHumidity) || isnan(newTemperature)) {
      temperatureC = NAN;
      humidity = NAN;

      Serial.println("Gagal membaca sensor DHT22.");
    } else {
      temperatureC = newTemperature;
      humidity = newHumidity;

      Serial.print("Suhu: ");
      Serial.print(temperatureC);
      Serial.print(" C | Kelembapan: ");
      Serial.print(humidity);
      Serial.println(" %");
    }
  }
}

// ==========================================
// 9. MEMBUAT JSON STATUS
// ==========================================

String makeStatusJSON() {
  updateDHT();

  String tempValue = isnan(temperatureC)
                       ? "null"
                       : String(temperatureC, 1);

  String humidityValue = isnan(humidity)
                           ? "null"
                           : String(humidity, 1);

  String json = "{\"temperature\":";
  json += tempValue;
  json += ",\"humidity\":";
  json += humidityValue;
  json += ",\"relays\":[";

  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    if (i > 0) {
      json += ",";
    }

    json += relayState[i] ? "true" : "false";
  }

  json += "]}";

  return json;
}

// ==========================================
// 10. HANDLER HALAMAN WEB
// ==========================================

void handleRoot() {
  server.send_P(200, "text/html", PAGE);
}

// ==========================================
// 11. API STATUS SENSOR DAN RELAY
// ==========================================

void handleStatus() {
  server.send(
    200,
    "application/json",
    makeStatusJSON()
  );
}

// ==========================================
// 12. API KONTROL RELAY
// URL contoh:
// /relay?ch=1&state=1  -> Relay 1 ON
// /relay?ch=1&state=0  -> Relay 1 OFF
// ==========================================

void handleRelay() {
  if (!server.hasArg("ch") ||
      !server.hasArg("state")) {

    server.send(
      400,
      "text/plain",
      "Parameter ch dan state wajib diisi"
    );

    return;
  }

  int channel = server.arg("ch").toInt();
  String state = server.arg("state");

  if (channel < 1 || channel > RELAY_COUNT ||
      (state != "0" && state != "1")) {

    server.send(
      400,
      "text/plain",
      "Parameter tidak valid"
    );

    return;
  }

  bool turnOn = (state == "1");

  writeRelay(channel - 1, turnOn);

  Serial.print("Relay ");
  Serial.print(channel);
  Serial.println(turnOn ? " ON" : " OFF");

  server.send(
    200,
    "application/json",
    makeStatusJSON()
  );
}

// ==========================================
// 13. MEMULAI WEB SERVER
// ==========================================

void startWebServer() {
  if (serverStarted) {
    return;
  }

  server.begin();
  serverStarted = true;

  Serial.println();
  Serial.println("Web server aktif.");
  Serial.print("Alamat web: http://");
  Serial.println(WiFi.localIP());
}

// ==========================================
// 14. SETUP
// ==========================================

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println();
  Serial.println("Memulai ESP IoT Web Server...");

  // Siapkan seluruh relay dalam kondisi OFF.
  // Atur level keluaran sebelum mengaktifkan OUTPUT
  // untuk mengurangi kemungkinan relay aktif sesaat.
  for (uint8_t i = 0; i < RELAY_COUNT; i++) {
    uint8_t offLevel =
      RELAY_ACTIVE_LOW ? HIGH : LOW;

    digitalWrite(relayPins[i], offLevel);
    pinMode(relayPins[i], OUTPUT);

    relayState[i] = false;
  }

  // Inisialisasi DHT22.
  dht.begin();

  // Daftarkan URL web server.
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/relay", HTTP_GET, handleRelay);

  server.onNotFound([]() {
    server.send(404, "text/plain", "Halaman tidak ditemukan");
  });

  // Hubungkan ESP ke Wi-Fi.
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  lastWiFiRetry = millis();

  Serial.print("Menghubungkan ke Wi-Fi");

  // Tidak menunggu tanpa batas jika Wi-Fi gagal.
  unsigned long startAttempt = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startAttempt < 15000) {

    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("Wi-Fi terhubung.");

    startWebServer();
  } else {
    Serial.println();
    Serial.println("Wi-Fi belum terhubung.");
    Serial.println("ESP akan mencoba kembali.");
  }
}

// ==========================================
// 15. LOOP
// ==========================================

void loop() {
  if (WiFi.status() == WL_CONNECTED) {

    if (!serverStarted) {
      startWebServer();
    }

    server.handleClient();

  } else {
    // Coba menghubungkan kembali setiap 10 detik.
    if (millis() - lastWiFiRetry >= 10000) {
      lastWiFiRetry = millis();

      Serial.println("Mencoba menghubungkan Wi-Fi...");
      WiFi.reconnect();
    }
  }
}