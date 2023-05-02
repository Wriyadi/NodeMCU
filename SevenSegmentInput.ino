#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<meta name="viewport" content="width=device-width, initial-scale=1" />
<body>
<h1>Nodemcu ESP8266 & IC4017 </h1>
<h2>HTML Form Seven Segment Input</h2>

<form action="/action_page">
  Input Number 0 - 9:<br>
  <input type="number" name="input" min="0" max="9">
  <br><br>
  <input type="submit" value="Submit">
</form> 

</body>
</html>
)=====";

const char* ssid = "SSID";
const char* password = "PASSWORD";
//Seven segment pins attachecd with nodemcu pins  
const int A = 16;  //Gpio-16 with a of 7 segment display   
const int B = 5;  //Gpio-5 with b of 7 segment display    
const int C = 4;  //Gpio-4 with c of 7 segment display  
const int D = 0;  //Gpio-0 with d of 7 segment display  

ESP8266WebServer server(80); //Server on port 80

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
  String s = MAIN_page; //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}
//===============================================================
// This routine is executed when you press submit
//===============================================================
void handleForm() {
 String input = server.arg("input"); 

 Serial.print("Input:");
 Serial.println(input);

 String s = "<a href='/'> <h2>Go Back</h2> </a>";
 server.send(200, "text/html", s); //Send web page
}
 
void setup() {
  Serial.begin(115200);
  delay(10);
  //Declaring Seven segment pins as Output
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
 
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address on serial monitor
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");    //URL IP to be typed in mobile/desktop browser
  Serial.print(WiFi.localIP());
  Serial.println("/");

  server.on("/", handleRoot);      //Which routine to handle at root location
  server.on("/action_page", handleForm); //form action is handled here

  server.begin();                  //Start server
  Serial.println("HTTP server started");
 
}
 
void loop() {
server.handleClient();          //Handle client requests

if (server.arg("input") == "0")  {
  nol();
}
if (server.arg("input") == "1")  {
  satu();
}
if (server.arg("input") == "2")  {  
  dua();
}
if (server.arg("input") == "3")  { 
  tiga();
}
if (server.arg("input") == "4")  {  
  empat();
}
if (server.arg("input") == "5")  { 
  lima();
}
if (server.arg("input") == "6")  {  
  enam();
}
if (server.arg("input") == "7")  {  
  tujuh();
}
if (server.arg("input") == "8")  { 
  delapan();
}
if (server.arg("input") == "9")  {  
  sembilan();
}  

}

void nol(){
  digitalWrite(A, LOW);
  digitalWrite(B, LOW);
  digitalWrite(C, LOW);
  digitalWrite(D, LOW);  
}

void satu(){
  digitalWrite(A, HIGH);
  digitalWrite(B, LOW);
  digitalWrite(C, LOW);
  digitalWrite(D, LOW);  
}

void dua(){
  digitalWrite(A, LOW);
  digitalWrite(B, HIGH);
  digitalWrite(C, LOW);
  digitalWrite(D, LOW);  
}

void tiga(){
  digitalWrite(A, HIGH);
  digitalWrite(B, HIGH);
  digitalWrite(C, LOW);
  digitalWrite(D, LOW);  
}

void empat(){
  digitalWrite(A, LOW);
  digitalWrite(B, LOW);
  digitalWrite(C, HIGH);
  digitalWrite(D, LOW);  
}

void lima(){
  digitalWrite(A, HIGH);
  digitalWrite(B, LOW);
  digitalWrite(C, HIGH);
  digitalWrite(D, LOW);  
}

void enam(){
  digitalWrite(A, LOW);
  digitalWrite(B, HIGH);
  digitalWrite(C, HIGH);
  digitalWrite(D, LOW);  
}

void tujuh(){
  digitalWrite(A, HIGH);
  digitalWrite(B, HIGH);
  digitalWrite(C, HIGH);
  digitalWrite(D, LOW);  
}

void delapan(){
  digitalWrite(A, LOW);
  digitalWrite(B, LOW);
  digitalWrite(C, LOW);
  digitalWrite(D, HIGH);  
}

void sembilan(){
  digitalWrite(A, HIGH);
  digitalWrite(B, LOW);
  digitalWrite(C, LOW);
  digitalWrite(D, HIGH);  
}
