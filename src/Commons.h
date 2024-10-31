#ifndef COMMONS_H  // Check if COMMONS_H is not defined
#define COMMONS_H  // Define COMMONS_H


// Include of commons libraries

// Generic Libraries
#include <ArduinoJson.h>
#include <Ticker.h>
#include <LittleFS.h>

#include <ESPAsyncWebServer.h>
#include <WiFiClientSecure.h>
#include <DNSServer.h>


// MCU sensitive lbraries
#if defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <ESP8266HTTPClient.h>
    #include <ESP8266httpUpdate.h>
    #include <ESPAsyncTCP.h>
    #include <ESP8266mDNS.h>
#elif defined(ESP32)
    #include <WiFi.h>
    #include <HTTPClient.h>
    #include <HTTPUpdate.h>
    #include <AsyncTCP.h>
    #include <ESPmDNS.h>
#else
#error "Only ESP8266 or ESP32"
#endif


struct Config {
    int wifiMode;
    int raveRpmOpen;
    int raveRpmClose;
    int hysteresisMillis;

    String WIFI_SSID;
    String WIFI_PASS;
    String AP_SSID;
    String AP_PASS;

    String hwid;
    String fw_version;
    String fs_version;

    String lastError;
};

#endif  // COMMONS_H