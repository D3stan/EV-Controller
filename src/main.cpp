#include "Commons.h"
#include "update.h"
#include "utils.h"
#include "webserver.h"
#include "websocket.h"
#include "wifi.h"
#include "rave.h"
#include "vars.h"

// Local Modules
// #include "vars.h" //already inside of commons
#include "cert.h"
#include "notfound.h"

// ----------------------------------------------------------------------------
// Definition of global constants
// ----------------------------------------------------------------------------

Config config;


// FS globals
bool FSmounted = false;


// Rpm count
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


// I/O
Led    onboard_led = { LED_BUILTIN, true };


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
// Filesystem
// ----------------------------------------------------------------------------



void initFS() {
    if (!LittleFS.begin()) {
        Serial.println("An error has occurred while mounting LittleFS");
        FSmounted = false;
        return;
    }

    FSmounted = true;
    loadConfiguration(config_file_path, config);
    saveConfiguration(config_file_path, config); // update fwid
}

void initGPIO() {
    pinMode(onboard_led.pin, OUTPUT);
    pinMode(my_inductive_in,      INPUT);
    attachInterrupt(digitalPinToInterrupt(my_inductive_in), signalDetected, FALLING);

    Serial.begin(115200);
}


// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

void setup() {
    initGPIO();
    for (int i = 0; i < 5; i++) {
        Serial.printf("\n.");
        delay(1000);
    }
    initHwid();
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

    // ArduinoOTA.handle();
    onboard_led.update();
    ws.cleanupClients();
    
    handlehostname();
    
}