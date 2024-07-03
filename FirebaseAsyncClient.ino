/**
 * SYNTAX:
 *
 * RealtimeDatabase::get(<AsyncClient>, <path>, <AsyncResultCallback>, <SSE>, <uid>);
 *
 * RealtimeDatabase::get(<AsyncClient>, <path>, <DatabaseOption>, <AsyncResultCallback>, <uid>);
 *
 * <AsyncClient> - The async client.
 * <path> - The node path to get/watch the value.
 * <DatabaseOption> - The database options (DatabaseOptions).
 * <AsyncResultCallback> - The async result callback (AsyncResultCallback).
 * <uid> - The user specified UID of async result (optional).
 * <SSE> - The Server-sent events (HTTP Streaming) mode.
 *
 * The complete usage guidelines, please visit https://github.com/mobizt/FirebaseClient
 */

#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <DHT.h>
#define DHTPIN 12     // DHT PIN 2
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);

#include <FirebaseClient.h>

#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "Password"

// The API key can be obtained from Firebase console > Project Overview > Project settings.
#define API_KEY "AIzaSxxxxx"

// User Email and password that already registerd or added in your project.
#define USER_EMAIL "email@gmail.com"
#define USER_PASSWORD "xxxxxx"
#define DATABASE_URL "https://willyR-xxxxx.asia-southeast1.firebasedatabase.app/"

void asyncCB(AsyncResult &aResult);

void printResult(AsyncResult &aResult);

DefaultNetwork network;

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);

FirebaseApp app;

#if defined(ESP32) || defined(ESP8266)
#include <WiFiClientSecure.h>
WiFiClientSecure ssl_client;
#endif

using AsyncClient = AsyncClientClass;

AsyncClient aClient(ssl_client, getNetwork(network));

RealtimeDatabase Database;

bool taskComplete = false;

void setup()
{
    dht.begin();
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    pinMode(16, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(0, OUTPUT);
    pinMode(2, OUTPUT);
    pinMode(14, OUTPUT);

    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

    Serial.println("Initializing app...");

#if defined(ESP32) || defined(ESP8266)
    ssl_client.setInsecure();
#if defined(ESP8266)
    ssl_client.setBufferSizes(4096, 1024);
#endif
#endif

    initializeApp(aClient, app, getAuth(user_auth), asyncCB, "authTask");

    // Binding the FirebaseApp for authentication handler.
    // To unbind, use Database.resetApp();
    app.getApp<RealtimeDatabase>(Database);

    Database.url(DATABASE_URL);
}

void loop()
{
    // The async task handler should run inside the main loop
    // without blocking delay or bypassing with millis code blocks.

    app.loop();

    Database.loop();

    if (app.ready() && !taskComplete)
    {
        taskComplete = true;
        // Get Data from Firebase
        Database.get(aClient, "/Output/16", asyncCB, false, "16");
        Database.get(aClient, "/Output/5", asyncCB, false, "5");
        Database.get(aClient, "/Output/4", asyncCB, false, "4");
        Database.get(aClient, "/Output/0", asyncCB, false, "0");
        Database.get(aClient, "/Output/2", asyncCB, false, "2");
        Database.get(aClient, "/Output/14", asyncCB, false, "14");
        
        // Read Humidity
        float h = dht.readHumidity();
        // Read temperature as Celsius (the default)
        float t = dht.readTemperature();
        // Push Data to Firebase
        Database.push<number_t>(aClient, "/TempHum/Temp", number_t(t, 2), asyncCB, "Temp");
        Database.push<number_t>(aClient, "/TempHum/Hum", number_t(h, 2), asyncCB, "Hum");
    }
}

void asyncCB(AsyncResult &aResult)
{
    // WARNING!
    // Do not put your codes inside the callback and printResult.

    printResult(aResult);
}

void printResult(AsyncResult &aResult)
{
    if (aResult.isEvent())
    {
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
    }

    if (aResult.isDebug())
    {
        Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
    }

    if (aResult.isError())
    {
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
    }

    if (aResult.available())
    {
        if (aResult.to<RealtimeDatabaseResult>().name().length())
            Firebase.printf("task: %s, name: %s\n", aResult.uid().c_str(), aResult.to<RealtimeDatabaseResult>().name().c_str());
        Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());

        // Parse the result and set the corresponding pin
        int pinValue = atoi(aResult.c_str());
        if (aResult.uid() == "16")
        {
            digitalWrite(16, pinValue);
        }
        else if (aResult.uid() == "5")
        {
            digitalWrite(5, pinValue);
        }
        else if (aResult.uid() == "4")
        {
            digitalWrite(4, pinValue);
        }
        else if (aResult.uid() == "0")
        {
            digitalWrite(0, pinValue);
        }
        else if (aResult.uid() == "2")
        {
            digitalWrite(2, pinValue);
        }
        else if (aResult.uid() == "14")
        {
            digitalWrite(14, pinValue);
        }
    }
}
