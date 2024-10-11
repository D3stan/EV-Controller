#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

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

const char index_html[] PROGMEM = R"(<!DOCTYPE html>
<html>

<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta name="apple-mobile-web-app-capable" content="yes">
  <meta name="description" content="ESP32 Remote Control with WebSocket">
  <title>RPM Counter</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }

    html, body {
      height: 100%;
      font-family: Roboto, sans-serif;
      font-size: 12pt;
      overflow: hidden;
    }

    body {
      display: grid;
      grid-template-rows: 1fr;
      align-items: center;
      justify-items: center;
    }

    .panel {
      display: grid;
      grid-gap: 3em;
      justify-items: center;
    }

    .rpm-container {
      display: flex;
      align-items: center;
    }

    h1 {
      font-size: 1.5rem;
      text-shadow: 0 2px 4px rgba(0, 0, 0, 0.3);
      margin-left: 10px;
    }

    #led, .led-red, .led-green {
      position: relative;
      width: 5em;
      height: 5em;
      border: 2px solid #000;
      border-radius: 2.5em;
      box-shadow: 0 0.5em 1em rgba(102, 0, 0, 0.3);
    }

    #led {
      background-image: radial-gradient(farthest-corner at 50% 20%, #b30000 0%, #330000 100%);
    }

    #led.on {
      background-image: radial-gradient(farthest-corner at 50% 75%, red 0%, #990000 100%);
      box-shadow: 0 1em 1.5em rgba(255, 0, 0, 0.5);
    }

    #led:after, .led-red:after, .led-green:after {
      content: '';
      position: absolute;
      top: .3em;
      left: 1em;
      width: 60%;
      height: 40%;
      border-radius: 60%;
      background-image: linear-gradient(rgba(255, 255, 255, 0.4), rgba(255, 255, 255, 0.1));
    }

    .led-red {
      width: 1em;
      height: 1em;
      background-color: red;
      border-radius: 50%;
      margin-right: 10px;
      box-shadow: 0 0.5em 1em rgba(102, 0, 0, 0.3);
    }

    .led-green {
      width: 1em;
      height: 1em;
      background-color: green;
      border-radius: 50%;
      margin-right: 10px;
      box-shadow: 0 0.5em 1em rgba(0, 102, 0, 0.3);
    }

    /* Gauge Styles */
    #gauge-container {
      position: relative;
      width: 200px;
      height: 200px;
      border-radius: 50%;
      display: flex;
      align-items: center;
      justify-content: center;
    }

    #gauge {
      position: relative;
      width: 100%;
      height: 100%;
      border-radius: 50%;
      display: flex;
      align-items: center;
      justify-content: center;
      background: conic-gradient(
        rgb(170, 170, 170) 0%, rgb(170, 170, 170) 25%,
        white 25%, white 50%,
        rgb(170, 170, 170) 50%, rgb(170, 170, 170) 100%
      )
    }

    #gauge-in {
      width: 80%;
      height: 80%;
      border-radius: 50%;
      background: white;
    }

    .rpm, .rpm-value {
      position: absolute;
      bottom: 10%;
      right: 5%;
      font-size: 1.3rem;
      color: #000000;
      text-shadow: 0 2px 4px rgba(0, 0, 0, 0.3);
    }

    .rpm-value {
        bottom: 20%;
        font-size: 1.8rem;
    }

  </style>
</head>

<body>
  <div class="panel">
    <div id="gauge-container">
      <div id="gauge">
        <div id="gauge-in"></div>
      </div>
      <div id="rpm-value" class="rpm-value">0</div>
      <div class="rpm">RPM</div>
    </div>
    <h1 id="maxrpm">MAX RPM: 0</h1>
    <div class="rpm-container">
      <div id="status-led" class="led-red"></div>
    </div>
  </div>
</body>

<script>
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
var maxRPM = 0;
var currentDegrees = 0;
var animationSpeed = 0.025; // Adjust this value to control the speed of the gauge animation

// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
}

// ----------------------------------------------------------------------------
// WebSocket handling
// ----------------------------------------------------------------------------

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
    document.getElementById('status-led').className = 'led-green';
}

function onClose(event) {
    console.log('Connection closed');
    document.getElementById('status-led').className = 'led-red';
    //document.getElementById('rpm-value').textContent = "0";
    updateGauge(0);
    document.getElementById('maxrpm').textContent = "MAX RPM: " + 0;
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    let data = JSON.parse(event.data);
    if (data.status)
        document.getElementById('led').className = data.status;
    if (data.rpm) {
        updateGauge(data.rpm);
        if (data.rpm > maxRPM) maxRPM = data.rpm;
        document.getElementById('maxrpm').textContent = "MAX RPM: " + maxRPM;
    }
}

function updateGauge(rpm) {
    const maxRpm = 15000; // Maximum RPM value
    const percentage = rpm * 100 / maxRpm;
    const targetDegrees = (percentage * 270) / 100; // Convert the percentage to degrees from 0 to 270

    function animateGauge() {
        if (currentDegrees < targetDegrees) {
            currentDegrees += animationSpeed * 270; // Increase angle
            if (currentDegrees > targetDegrees) currentDegrees = targetDegrees;
        } else if (currentDegrees > targetDegrees) {
            currentDegrees -= animationSpeed * 270; // Decrease angle
            if (currentDegrees < targetDegrees) currentDegrees = targetDegrees;
        }

        const degrees = (currentDegrees * 360) / 270 - 90;
        let gaugeColor;

        if (degrees > 180) {
            gaugeColor = `conic-gradient(
                red 0deg, 
                red ${degrees - 180}deg,
                rgb(170, 170, 170) ${degrees - 180}deg,
                rgb(170, 170, 170) 90deg,
                white 90deg,
                white 180deg,
                red 180deg,
                red 360deg
            )`;
        } else {
            gaugeColor = `conic-gradient(
                rgb(170, 170, 170) 0deg,
                rgb(170, 170, 170) 90deg,
                white 90deg,
                white 180deg,
                red 180deg,
                red ${degrees + 180}deg,
                rgb(170, 170, 170) ${degrees + 180}deg,
                rgb(170, 170, 170) 360deg
            )`;
        }

        document.getElementById('gauge').style.background = gaugeColor;
        document.getElementById('rpm-value').textContent = rpm;

        if (currentDegrees !== targetDegrees) {
            requestAnimationFrame(animateGauge);
        }
    }

    animateGauge();
}
</script>

</html>

)";


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
    /*
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    
    Serial.printf("Trying to connect [%s] ", WiFi.macAddress().c_str());
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }*/

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_SSID, AP_PASS);


    Serial.printf(" %s\n", WiFi.softAPIP().toString().c_str());
}

// ----------------------------------------------------------------------------
// Web server initialization
// ----------------------------------------------------------------------------

void onRootRequest(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", index_html);
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not Found");
}

void initWebServer() {
    server.on("/", onRootRequest);
    server.onNotFound(notFound);
    server.begin();
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

        JsonDocument  json;
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
// Initialization
// ----------------------------------------------------------------------------

void setup() {
    pinMode(onboard_led.pin, OUTPUT);
    pinMode(INDUCTIVE_IN,      INPUT);
    attachInterrupt(digitalPinToInterrupt(INDUCTIVE_IN), signalDetected, FALLING);

    Serial.begin(115200); delay(500);

    initWiFi();
    initWebSocket();
    initWebServer();
}

// ----------------------------------------------------------------------------
// Main control loop
// ----------------------------------------------------------------------------

void loop() {
    ws.cleanupClients();
    
    onboard_led.on = millis() % 1000 < 500;
    if (millis() - displayMillis >= 35) updateRPM(RPM);

    onboard_led.update();
}