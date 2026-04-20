/*********************************************************
 * ESP8266 / ESP32 - Local WebServer 3 Relay + DHT22
 * NPN Transistor Relay (ACTIVE HIGH)
 * Fully Cross-Platform Compatible
 *********************************************************/

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <WiFi.h>
#else
  #error "Unsupported board"
#endif

#include <DHT.h> // Tambahkan library DHT

/******************** WIFI CONFIG ********************/
const char* ssid     = "NAMA_WIFI_ANDA";
const char* password = "PASSWORD_WIFI";

/******************** WEB SERVER *********************/
WiFiServer server(80);
String header;

/******************** PIN CONFIG *********************/
// Konfigurasi Pin Relay
#if defined(ESP8266)
const uint8_t RELAY1 = 5;   // D1
const uint8_t RELAY2 = 4;   // D2
const uint8_t RELAY3 = 14;  // D5
#elif defined(ESP32)
const uint8_t RELAY1 = 16;
const uint8_t RELAY2 = 5;
const uint8_t RELAY3 = 4;
#endif

// Konfigurasi Pin DHT22 (Aman untuk ESP8266 dan ESP32)
#define DHTPIN 2 
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

/******************** STATE & TIMEOUT ****************/
String relay1State = "OFF";
String relay2State = "OFF";
String relay3State = "OFF";

unsigned long previousTime = 0;
const unsigned long timeoutTime = 2000;

/******************** HTML + CSS *********************/
// Menambahkan Card untuk Suhu dan Kelembaban
const char html_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP Control Panel</title>
<link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.15.4/css/all.css">
<style>
html{font-family:Arial;text-align:center;background:#f1f1f1}
body{margin:0;padding-bottom:60px}
.topnav{background:#049faa;color:#fff;padding:15px}
.card-grid{max-width:800px;margin:20px auto;display:grid;grid-gap:20px;
grid-template-columns:repeat(auto-fit,minmax(200px,1fr))}
.card{background:#fff;padding:20px;border-radius:10px;
box-shadow:0 4px 8px rgba(0,0,0,.2)}
.sensor-card{background:#e8f4f8; border-left: 5px solid #049faa;} /* Gaya khusus untuk sensor */
button{border:none;padding:10px 20px;font-size:16px;color:#fff;
border-radius:5px;cursor:pointer}
.on{background:#034078}
.off{background:#858585}
.state{margin-top:10px;font-weight:bold;color:#1282A2}
.sensor-value{font-size: 28px; font-weight: bold; color:#034078; margin: 10px 0;}
.footer{position:fixed;bottom:0;width:100%;background:#049faa;
color:#fff;padding:10px;font-size:14px}
</style>
</head>
<body>
<div class="topnav">
<h2>ESP Web Control <i class="fas fa-microchip"></i></h2>
</div>

<div class="card-grid">
  <div class="card sensor-card">
    <h3><i class="fas fa-thermometer-half"></i> Temperature</h3>
    <p class="sensor-value">%TEMP% &deg;C</p>
  </div>
  
  <div class="card sensor-card">
    <h3><i class="fas fa-tint"></i> Humidity</h3>
    <p class="sensor-value">%HUM% &percnt;</p>
  </div>

  <div class="card">
    <h3><i class="fas fa-lightbulb"></i> Relay 1</h3>
    <a href="/r1/on"><button class="on">ON</button></a>
    <a href="/r1/off"><button class="off">OFF</button></a>
    <p class="state">State: %R1%</p>
  </div>

  <div class="card">
    <h3><i class="fas fa-fan"></i> Relay 2</h3>
    <a href="/r2/on"><button class="on">ON</button></a>
    <a href="/r2/off"><button class="off">OFF</button></a>
    <p class="state">State: %R2%</p>
  </div>

  <div class="card">
    <h3><i class="fas fa-plug"></i> Relay 3</h3>
    <a href="/r3/on"><button class="on">ON</button></a>
    <a href="/r3/off"><button class="off">OFF</button></a>
    <p class="state">State: %R3%</p>
  </div>
</div>

<div class="footer">
&copy; Willy Riyadi 2025 - ESP8266 / ESP32
</div>
</body>
</html>
)rawliteral";

/******************** SETUP *********************/
void setup() {
  Serial.begin(115200);
  header.reserve(1024); 

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);

  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY3, LOW);

  dht.begin(); // Inisialisasi sensor DHT

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());

  server.begin();
}

/******************** LOOP *********************/
void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  previousTime = millis();
  String currentLine = "";

  while (client.connected() && millis() - previousTime < timeoutTime) {
    if (client.available()) {
      char c = client.read();
      header += c;

      if (c == '\n') {
        if (currentLine.length() == 0) {

          // Cek request endpoint relay
          if (header.indexOf("GET /r1/on") >= 0) {
            digitalWrite(RELAY1, HIGH); relay1State = "ON";
          } else if (header.indexOf("GET /r1/off") >= 0) {
            digitalWrite(RELAY1, LOW); relay1State = "OFF";
          } else if (header.indexOf("GET /r2/on") >= 0) {
            digitalWrite(RELAY2, HIGH); relay2State = "ON";
          } else if (header.indexOf("GET /r2/off") >= 0) {
            digitalWrite(RELAY2, LOW); relay2State = "OFF";
          } else if (header.indexOf("GET /r3/on") >= 0) {
            digitalWrite(RELAY3, HIGH); relay3State = "ON";
          } else if (header.indexOf("GET /r3/off") >= 0) {
            digitalWrite(RELAY3, LOW); relay3State = "OFF";
          }

          // Baca data dari DHT22
          float t = dht.readTemperature();
          float h = dht.readHumidity();

          // Validasi pembacaan, jika gagal tampilkan "--"
          String tempString = isnan(t) ? "--" : String(t, 1);
          String humString  = isnan(h) ? "--" : String(h, 1);

          // Render HTML dan Replace variabel
          String page = FPSTR(html_page);
          page.replace("%R1%", relay1State);
          page.replace("%R2%", relay2State);
          page.replace("%R3%", relay3State);
          page.replace("%TEMP%", tempString);
          page.replace("%HUM%", humString);

          // Kirim HTTP Response
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println();
          client.print(page);
          client.println();
          break;
        } else {
          currentLine = "";
        }
      } else if (c != '\r') {
        currentLine += c;
      }
    }
    yield();
  }
  header = "";
  client.stop();
}
