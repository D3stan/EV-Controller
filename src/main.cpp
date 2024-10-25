#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "AsyncJson.h"

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266httpUpdate.h>
#include <notfound.h>

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

File configFile;
JsonDocument config;
bool FSmounted = false;
String errorReplacementPage = index_html;

// WiFi globals
unsigned int wifiMode = WIFI_AP;

const char *WIFI_SSID = "";
const char *WIFI_PASS = "";

const char *AP_SSID = "";
const char *AP_PASS = "";
IPAddress AP_IP (42, 42, 42, 42);

// rpm count
unsigned long RPM = 0;
unsigned long currentMicros = 0;
unsigned long lastMicros = 0;
unsigned long displayMillis = 0;

// rave controlling values
unsigned int raveRpmOpen = 0;
unsigned int raveRpmClose = 0;

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
    if (wifiMode == WIFI_AP) {
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
        WiFi.softAP(AP_SSID, AP_PASS);

        Serial.printf(" %s\n", WiFi.softAPIP().toString().c_str());
    
    } else if (wifiMode == WIFI_STA) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASS);

        Serial.printf(" %s\n", WiFi.localIP().toString().c_str());

    }
}

// ----------------------------------------------------------------------------
// Web server
// ----------------------------------------------------------------------------

void onRootRequest(AsyncWebServerRequest *request) {
    const char* errorMsg = "";

    if (!FSmounted) {
        errorMsg = "Error: filesystem not mounted correctly";
    } else if (!configFile) {
        errorMsg = "Error: cannot read config file";
    } else if (config.isNull()) {
        errorMsg = "Error: cannot parse config file";
    }

    errorReplacementPage = index_html;
    errorReplacementPage.replace(errorPlaceHolder, errorMsg);
    errorReplacementPage.replace(errorCodePlaceHolder, "500");
    request->send_P(500, "text/html", index_html);
}

void notFound(AsyncWebServerRequest *request) {
    errorReplacementPage = index_html;
    errorReplacementPage.replace(errorPlaceHolder, "Error: Page Not Found");
    errorReplacementPage.replace(errorCodePlaceHolder, "404");
    request->send(404, "text/html", index_html);
}

void initWebServer() {
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
// WebSocket
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

void initFS() {
    if (!LittleFS.begin()) {
        Serial.println("An error has occurred while mounting LittleFS");
        FSmounted = false;
        return;
    }

    FSmounted = true;
    configFile = LittleFS.open("/config.json", "r+");
    if (!configFile) {
        Serial.println("Error reading config file");
        return;
    }

    
    DeserializationError err = deserializeJson(config, configFile.readString());
    if (err) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.c_str());
        return;
    }

    wifiMode = config["wifi-mode"];
    WIFI_PASS = config["wifi-psw"];
    WIFI_SSID = config["wifi-ssid"];
    AP_SSID = config["ap-ssid"];
    AP_PASS = config["ap-psw"];
    raveRpmOpen = config["rave-rpm-open"];
    raveRpmClose = config["rave-rpm-close"];    

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
    initFS();
    
    initWiFi();
    initWebSocket();
    initWebServer();
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