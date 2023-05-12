#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Replace with your network credentials
const char* ssid     = "SSID";
const char* password = "PASSWORD";

// Set web server port number to 80
ESP8266WebServer server(80);      //Menyatakan Webserver pada port 80

// Variable to store the HTTP request
String webpage;

#define LED1 16  // Pemilihan Pin D0
#define LED2 5    // Pemilihan Pin D1
#define LED3 4    // Pemilihan Pin D2
#define LED4 0  // Pemilihan Pin D3
#define LED5 2  // Pemilihan Pin D4
#define LED6 14    // Pemilihan Pin D5
#define LED7 12   // Pemilihan Pin D6
#define LED8 13    // Pemilihan Pin D7
#define LED9 15   // Pemilihan Pin D8

void setup() {  //Pengaturan Pin
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

  // Connect ke WiFi
  Serial.println();
  Serial.print("Configuring access point...");

  // Mengatur WiFi 
  WiFi.mode(WIFI_AP);                      // Mode Station
  WiFi.begin(ssid, password);               // Mencocokan SSID dan Password

  // Print status Connect 
  Serial.println("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Isi dari Webpage 
  webpage += "&lt;h1&gt; Web Control LED ESP8266&lt;/h1&gt;";
  webpage += "&lt;wp-p&gt;LED 1 : ";
  webpage += "&lt;a href=\"LED1ON\"\"&gt;&lt;button&gt;ON&lt;/button&gt;&lt;/a&gt;&lt;a href=\"LED1OFF\"\"&gt;&lt;button&gt;OFF&lt;/button&gt;&lt;/a&gt;&lt;/wp-p&gt;&lt;wp-br&gt;";
  webpage += "&lt;wp-p&gt;LED 2 : ";
  webpage += "&lt;a href=\"LED2ON\"\"&gt;&lt;button&gt;ON&lt;/button&gt;&lt;/a&gt;&lt;a href=\"LED2OFF\"\"&gt;&lt;button&gt;OFF&lt;/button&gt;&lt;/a&gt;&lt;/wp-p&gt;&lt;wp-br&gt;";
  webpage += "&lt;wp-p&gt;LED 3 : ";
  webpage += "&lt;a href=\"LED3ON\"\"&gt;&lt;button&gt;ON&lt;/button&gt;&lt;/a&gt;&lt;a href=\"LED3OFF\"\"&gt;&lt;button&gt;OFF&lt;/button&gt;&lt;/a&gt;&lt;/wp-p&gt;&lt;wp-br&gt;";
  webpage += "&lt;wp-p&gt;LED 4 : ";
  webpage += "&lt;a href=\"LED4ON\"\"&gt;&lt;button&gt;ON&lt;/button&gt;&lt;/a&gt;&lt;a href=\"LED4OFF\"\"&gt;&lt;button&gt;OFF&lt;/button&gt;&lt;/a&gt;&lt;/wp-p&gt;&lt;wp-br&gt;";
  webpage += "&lt;wp-p&gt;LED 5 : ";
  webpage += "&lt;a href=\"LED5ON\"\"&gt;&lt;button&gt;ON&lt;/button&gt;&lt;/a&gt;&lt;a href=\"LED5OFF\"\"&gt;&lt;button&gt;OFF&lt;/button&gt;&lt;/a&gt;&lt;/wp-p&gt;&lt;wp-br&gt;";
  webpage += "&lt;wp-p&gt;LED 6 : ";
  webpage += "&lt;a href=\"LED6ON\"\"&gt;&lt;button&gt;ON&lt;/button&gt;&lt;/a&gt;&lt;a href=\"LED6OFF\"\"&gt;&lt;button&gt;OFF&lt;/button&gt;&lt;/a&gt;&lt;/wp-p&gt;&lt;wp-br&gt;";
  webpage += "&lt;wp-p&gt;LED 7 : ";
  webpage += "&lt;a href=\"LED7ON\"\"&gt;&lt;button&gt;ON&lt;/button&gt;&lt;/a&gt;&lt;a href=\"LED7OFF\"\"&gt;&lt;button&gt;OFF&lt;/button&gt;&lt;/a&gt;&lt;/wp-p&gt;&lt;wp-br&gt;";
  webpage += "&lt;wp-p&gt;LED 8 : ";
  webpage += "&lt;a href=\"LED8ON\"\"&gt;&lt;button&gt;ON&lt;/button&gt;&lt;/a&gt;&lt;a href=\"LED8OFF\"\"&gt;&lt;button&gt;OFF&lt;/button&gt;&lt;/a&gt;&lt;/wp-p&gt;&lt;wp-br&gt;";
  webpage += "&lt;wp-p&gt;LED 9 : ";
  webpage += "&lt;a href=\"LED9ON\"\"&gt;&lt;button&gt;ON&lt;/button&gt;&lt;/a&gt;&lt;a href=\"LED9OFF\"\"&gt;&lt;button&gt;OFF&lt;/button&gt;&lt;/a&gt;&lt;/wp-p&gt;";

  // Membuat file webpage 
  server.on("/", []() {
    server.send(200, "text/html", webpage);
  });

  // Bagian ini untuk merespon perintah yang masuk 
  server.on("/LED1ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED1, HIGH);
    delay(500);
  });
  server.on("/LED2ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED2, HIGH);
    delay(500);
  });
  server.on("/LED3ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED3, HIGH);
    delay(500);
  });
    server.on("/LED4ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED4, HIGH);
    delay(500);
  });
  server.on("/LED5ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED5, HIGH);
    delay(500);
  });
  server.on("/LED6ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED6, HIGH);
    delay(500);
  });
    server.on("/LED7ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED7, HIGH);
    delay(500);
  });
  server.on("/LED8ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED8, HIGH);
    delay(500);
  });
  server.on("/LED9ON", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED9, HIGH);
    delay(500);
  });
  server.on("/LED1OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED1, LOW);
    delay(500);
  });
  server.on("/LED2OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED2, LOW);
    delay(500);
  });
  server.on("/LED3OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED3, LOW);
    delay(500);
  });
    server.on("/LED4OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED4, LOW);
    delay(500);
  });
  server.on("/LED5OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED5, LOW);
    delay(500);
  });
  server.on("/LED6OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED6, LOW);
    delay(500);
  });
    server.on("/LED7OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED7, LOW);
    delay(500);
  });
  server.on("/LED8OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED8, LOW);
    delay(500);
  });
  server.on("/LED9OFF", []() {
    server.send(200, "text/html", webpage);
    digitalWrite(LED9, LOW);
    delay(500);
  });

  server.begin();
  Serial.println("Server dijalankan");
}

void loop() {  //Perulangan Program
  server.handleClient();
}
