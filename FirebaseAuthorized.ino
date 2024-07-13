#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "Password"

// Insert Firebase project API Key
#define API_KEY "AIzaSyCUxxxx"

// Insert Authorized Username and Corresponding Password
#define USER_EMAIL "xxx@gmail.com"
#define USER_PASSWORD "PASSWORD"

// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://willyxxx.asia-southeast1.firebasedatabase.app/"

// Define Firebase objects
FirebaseData stream;
FirebaseAuth auth;
FirebaseConfig config;

// Variables to save database paths
String listenerPath = "/Output";
String tempPath = "/TempHum/temp";
String humPath = "/TempHum/hum";
String chaser1Path = "/Output/Chaser1";
String chaser2Path = "/Output/Chaser2";

// Declare outputs
const int Relay1 = 16;
const int Relay2 = 5;
const int Relay3 = 4;
const int Relay4 = 0;
const int Relay5 = 2;
const int Relay6 = 14;

// DHT22
#define DHTPIN 12
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Variables to store the last sent temperature and humidity values
float lastTemperature = NAN;
float lastHumidity = NAN;

// Flag to control the LED chaser
bool chaser1Running = false;
bool chaser2Running = false;

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
}

// Callback function that runs on database changes
void streamCallback(FirebaseStream data){
  Serial.printf("stream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); //see addons/RTDBHelper.h
  Serial.println();

  // Get the path that triggered the function
  String streamPath = String(data.dataPath());

  // if the data returned is an integer, there was a change on the GPIO state on the following path /{gpio_number}
  if (data.dataTypeEnum() == fb_esp_rtdb_data_type_integer){
    String gpio = streamPath.substring(1);
    int state = data.intData();
    Serial.print("GPIO: ");
    Serial.println(gpio);
    Serial.print("STATE: ");
    Serial.println(state);
    digitalWrite(gpio.toInt(), state);
  }

  // Handle Chaser1 logic
  if (streamPath == "/Chaser1") {
    int chaser1State = data.intData();
    if (chaser1State == 1) {
      chaser1Running = true;
      Serial.println("Chaser1 started.");
    } else {
      chaser1Running = false;
      Serial.println("Chaser1 stopped.");
      // Turn off all relays
      digitalWrite(Relay1, LOW);
      digitalWrite(Relay2, LOW);
      digitalWrite(Relay3, LOW);
      digitalWrite(Relay4, LOW);
      digitalWrite(Relay5, LOW);
      digitalWrite(Relay6, LOW);
    }
  }

  // Handle Chaser1 logic
  if (streamPath == "/Chaser2") {
    int chaser2State = data.intData();
    if (chaser2State == 1) {
      chaser2Running = true;
      Serial.println("Chaser2 started.");
    } else {
      chaser2Running = false;
      Serial.println("Chaser2 stopped.");
      // Turn off all relays
      digitalWrite(Relay1, LOW);
      digitalWrite(Relay2, LOW);
      digitalWrite(Relay3, LOW);
      digitalWrite(Relay4, LOW);
      digitalWrite(Relay5, LOW);
      digitalWrite(Relay6, LOW);
    }
  }

  /* When it first runs, it is triggered on the root (/) path and returns a JSON with all keys
  and values of that path. So, we can get all values from the database and updated the GPIO states*/
  if (data.dataTypeEnum() == fb_esp_rtdb_data_type_json){
    FirebaseJson json = data.to<FirebaseJson>();

    // To iterate all values in Json object
    size_t count = json.iteratorBegin();
    Serial.println("\n---------");
    for (size_t i = 0; i < count; i++){
        FirebaseJson::IteratorValue value = json.valueAt(i);
        int gpio = value.key.toInt();
        int state = value.value.toInt();
        Serial.print("STATE: ");
        Serial.println(state);
        Serial.print("GPIO:");
        Serial.println(gpio);
        digitalWrite(gpio, state);
        Serial.printf("Name: %s, Value: %s, Type: %s\n", value.key.c_str(), value.value.c_str(), value.type == FirebaseJson::JSON_OBJECT ? "object" : "array");
    }
    Serial.println();
    json.iteratorEnd(); // required for free the used memory in iteration (node data collection)
  }
  
  //This is the size of stream payload received (current and max value)
  //Max payload size is the payload size under the stream path since the stream connected
  //and read once and will not update until stream reconnection takes place.
  //This max value will be zero as no payload received in case of ESP8266 which
  //BearSSL reserved Rx buffer size is less than the actual stream payload.
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());
}

void streamTimeoutCallback(bool timeout){
  if (timeout)
    Serial.println("stream timeout, resuming...\n");
  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void setup(){
  Serial.begin(115200);
  initWiFi();

  // Initialize Outputs
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(Relay3, OUTPUT);
  pinMode(Relay4, OUTPUT);
  pinMode(Relay5, OUTPUT);
  pinMode(Relay6, OUTPUT);
  
  // Initialize DHT sensor
  dht.begin();
  
  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Assign the RTDB URL (required)
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);

  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Streaming (whenever data changes on a path)
  // Begin stream on a database path --> board1/outputs/digital
  if (!Firebase.RTDB.beginStream(&stream, listenerPath.c_str()))
    Serial.printf("stream begin error, %s\n\n", stream.errorReason().c_str());

  // Assign a calback function to run when it detects changes on the database
  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);

  delay(2000);
}

void updateTempHum(){
  // Read temperature and humidity
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check if any reads failed and exit early (to try again).
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Update temperature to Firebase and Serial Monitor only if value has changed
  if (temperature != lastTemperature) {
    if (Firebase.RTDB.setFloat(&stream, tempPath.c_str(), temperature)) {
      Serial.println("Temperature data uploaded successfully.");
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" °C");
    } else {
      Serial.println("Failed to upload temperature data.");
      Serial.println(stream.errorReason());
    }
    lastTemperature = temperature;
  }

  // Update humidity to Firebase and Serial Monitor only if value has changed
  if (humidity != lastHumidity) {
    if (Firebase.RTDB.setFloat(&stream, humPath.c_str(), humidity)) {
      Serial.println("Humidity data uploaded successfully.");
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");
    } else {
      Serial.println("Failed to upload humidity data.");
      Serial.println(stream.errorReason());
    }
    lastHumidity = humidity;
  }
}


void loop(){
  updateTempHum();

  if (chaser1Running) {
    // Chaser sequence
    digitalWrite(Relay1, HIGH);
    delay(500);
    digitalWrite(Relay1, LOW);
    digitalWrite(Relay2, HIGH);
    delay(500);
    digitalWrite(Relay2, LOW);
    digitalWrite(Relay3, HIGH);
    delay(500);
    digitalWrite(Relay3, LOW);
    digitalWrite(Relay4, HIGH);
    delay(500);
    digitalWrite(Relay4, LOW);
    digitalWrite(Relay5, HIGH);
    delay(500);
    digitalWrite(Relay5, LOW);
    digitalWrite(Relay6, HIGH);
    delay(500);
    digitalWrite(Relay6, LOW);
  }

  if (chaser2Running) {
    // Chaser sequence
    digitalWrite(Relay1, HIGH);
    digitalWrite(Relay6, HIGH);
    delay(500);
    digitalWrite(Relay1, LOW);
    digitalWrite(Relay6, LOW);
    digitalWrite(Relay2, HIGH);
    digitalWrite(Relay5, HIGH);
    delay(500);
    digitalWrite(Relay2, LOW);
    digitalWrite(Relay5, LOW);
    digitalWrite(Relay3, HIGH);
    digitalWrite(Relay4, HIGH);
    delay(500);
    digitalWrite(Relay3, LOW);
    digitalWrite(Relay4, LOW);
    delay(500);
  }
  
  // Check if token is expired and refresh if necessary
  if (Firebase.isTokenExpired()){
    Firebase.refreshToken(&config);
    Serial.println("Refresh token");
  }
}
