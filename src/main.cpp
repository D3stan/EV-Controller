#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "AsyncJson.h"

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <Ticker.h>
#include <LittleFS.h>

// ----------------------------------------------------------------------------
// Definition of macros
// ----------------------------------------------------------------------------

#define LED_PIN      15
#define BTN_PIN      5 
#define INDUCTIVE_IN 4
#define HTTP_PORT    80

// ----------------------------------------------------------------------------
// Definition of global constants
// ----------------------------------------------------------------------------

// Button debouncing
const uint8_t DEBOUNCE_DELAY = 10; // in milliseconds

// WiFi credentials
const char *WIFI_SSID = "GiuseppeBagnile";
const char *WIFI_PASS = "MikroTik2020";

const char *AP_SSID = "rpmcounter";
const char *AP_PASS = "ciaociao";
IPAddress AP_IP (42, 42, 42, 42);

// rpm count
unsigned long RPM = 0;
unsigned long currentMicros = 0;
unsigned long lastMicros = 0;
unsigned long displayMillis = 0;

// ----------------------------------------------------------------------------
// Definition of the LED component
// ----------------------------------------------------------------------------

struct Led {
    // state variables
    uint8_t pin;
    bool    on;

    // methods
    void update() {
        digitalWrite(pin, on ? HIGH : LOW);
    }
};

// ----------------------------------------------------------------------------
// Definition of global variables
// ----------------------------------------------------------------------------

Led    onboard_led = { LED_BUILTIN, true };

AsyncWebServer server(HTTP_PORT);
AsyncWebSocket ws("/ws");

// ----------------------------------------------------------------------------
// Interrupt for RPM measurement
// ----------------------------------------------------------------------------


IRAM_ATTR void signalDetected() {
    currentMicros = micros();

    if (lastMicros != currentMicros && (currentMicros - lastMicros) >= 500) {
        RPM = 60000000 / (currentMicros - lastMicros);
        lastMicros = currentMicros;
    }

}

// ----------------------------------------------------------------------------
// Connecting to the WiFi network
// ----------------------------------------------------------------------------

void initWiFi() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_SSID, AP_PASS);

    Serial.printf(" %s\n", WiFi.softAPIP().toString().c_str());
}

// ----------------------------------------------------------------------------
// Web server initialization
// ----------------------------------------------------------------------------

void onRootRequest(AsyncWebServerRequest *request) {
    request->send(500, "text/html", "File System not mounted correclty");
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not Found");
}

void initWebServer(bool FSmounted) {
    if (FSmounted) {
        server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    } else {
        server.on("/", onRootRequest);
    }
    server.onNotFound(notFound);

    server.begin();
    ArduinoOTA.begin();
}

// ----------------------------------------------------------------------------
// WebSocket initialization
// ----------------------------------------------------------------------------

void updateRPM(unsigned long rpm = 0) {
    JsonDocument json;
    json["rpm"] = rpm;

    char data[30];
    size_t len = serializeJson(json, data);
    ws.textAll(data, len);
    displayMillis = millis();
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {

        JsonDocument json;
        DeserializationError err = deserializeJson(json, data);
        if (err) {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(err.c_str());
            return;
        }

        /*
        const char *action = json["action"];
        if (strcmp(action, "toggle") == 0) {
            led.on = !led.on;
            notifyClients();
        }
        */
    }
}

void onEvent(AsyncWebSocket       *server,
             AsyncWebSocketClient *client,
             AwsEventType          type,
             void                 *arg,
             uint8_t              *data,
             size_t                len) {

    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void initWebSocket() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}


// ----------------------------------------------------------------------------
// Utils
// ----------------------------------------------------------------------------

bool initFS() {
    if (!LittleFS.begin()) {
        Serial.println("An error has occurred while mounting LittleFS");
        return false;
    }
    return true;
}

void initGPIO() {
    pinMode(onboard_led.pin, OUTPUT);
    pinMode(INDUCTIVE_IN,      INPUT);
    attachInterrupt(digitalPinToInterrupt(INDUCTIVE_IN), signalDetected, FALLING);

    Serial.begin(115200);
}


// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void setup() {
    initGPIO();
    initWiFi();
    initWebSocket();
    initWebServer(initFS());
}

// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop() {
    onboard_led.on = millis() % 1000 < 500;
    if (millis() - displayMillis >= 35) updateRPM(RPM);

    ArduinoOTA.handle();
    onboard_led.update();
    ws.cleanupClients();
}