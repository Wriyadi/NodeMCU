#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

// Replace with your network credentials
const char* ssid     = "SSID";
const char* password = "PASSWORD";

// Set web server port number to 80
ESP8266WebServer server(80);

// DHT Sensor setup
#define DHTPIN 2     // D4 pin
#define DHTTYPE DHT22   // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);

float temperature;
float humidity;

// Variable to store the HTML web page
String webpage;

#define LED1 16  // D0
#define LED2 5   // D1
#define LED3 4   // D2
#define LED4 0   // D3
#define LED5 2   // D4
#define LED6 14  // D5
#define LED7 12  // D6
#define LED8 13  // D7
#define LED9 15  // D8

void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  pinMode(LED6, OUTPUT);
  pinMode(LED7, OUTPUT);
  pinMode(LED8, OUTPUT);
  pinMode(LED9, OUTPUT);

  dht.begin();

  // Connect to Wi-Fi
  Serial.println();
  Serial.print("Configuring access point...");

  // Set up Wi-Fi
  WiFi.mode(WIFI_AP);
  WiFi.begin(ssid, password);

  // Print status
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Set up webpage content
  webpage += "<h1>Web Control LED ESP8266</h1>";
  webpage += "<p>LED 1: <a href=\"LED1ON\"><button>ON</button></a><a href=\"LED1OFF\"><button>OFF</button></a></p>";
  webpage += "<p>LED 2: <a href=\"LED2ON\"><button>ON</button></a><a href=\"LED2OFF\"><button>OFF</button></a></p>";
  webpage += "<p>LED 3: <a href=\"LED3ON\"><button>ON</button></a><a href=\"LED3OFF\"><button>OFF</button></a></p>";
  webpage += "<p>LED 4: <a href=\"LED4ON\"><button>ON</button></a><a href=\"LED4OFF\"><button>OFF</button></a></p>";
  webpage += "<p>LED 5: <a href=\"LED5ON\"><button>ON</button></a><a href=\"LED5OFF\"><button>OFF</button></a></p>";
  webpage += "<p>LED 6: <a href=\"LED6ON\"><button>ON</button></a><a href=\"LED6OFF\"><button>OFF</button></a></p>";
  webpage += "<p>LED 7: <a href=\"LED7ON\"><button>ON</button></a><a href=\"LED7OFF\"><button>OFF</button></a></p>";
  webpage += "<p>LED 8: <a href=\"LED8ON\"><button>ON</button></a><a href=\"LED8OFF\"><button>OFF</button></a></p>";
  webpage += "<p>LED 9: <a href=\"LED9ON\"><button>ON</button></a><a href=\"LED9OFF\"><button>OFF</button></a></p>";
  webpage += "<p>Temperature: <span id=\"temperature\">--</span> &deg;C</p>";
  webpage += "<p>Humidity: <span id=\"humidity\">--</span> %</p>";
  webpage += "<script>setInterval(function() {fetch('/data').then(response => response.json()).then(data => {document.getElementById('temperature').innerText = data.temperature;document.getElementById('humidity').innerText = data.humidity;});}, 2000);</script>";

  // Configure web server
  server.on("/", []() {
    server.send(200, "text/html", webpage);
  });

  server.on("/data", []() {
    String data = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
    server.send(200, "application/json", data);
  });

  // LED control handlers
  server.on("/LED1ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED1, HIGH);
  });
  server.on("/LED1OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED1, LOW);
  });
  server.on("/LED2ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED2, HIGH);
  });
  server.on("/LED2OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED2, LOW);
  });
  server.on("/LED3ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED3, HIGH);
  });
  server.on("/LED3OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED3, LOW);
  });
  server.on("/LED4ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED4, HIGH);
  });
  server.on("/LED4OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED4, LOW);
  });
  server.on("/LED5ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED5, HIGH);
  });
  server.on("/LED5OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED5, LOW);
  });
  server.on("/LED6ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED6, HIGH);
  });
  server.on("/LED6OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED6, LOW);
  });
  server.on("/LED7ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED7, HIGH);
  });
  server.on("/LED7OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED7, LOW);
  });
  server.on("/LED8ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED8, HIGH);
  });
  server.on("/LED8OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED8, LOW);
  });
  server.on("/LED9ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED9, HIGH);
  });
  server.on("/LED9OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED9, LOW);
  });

  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();

  // Read the temperature and humidity
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // Print the sensor data to the Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" *C ");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}
