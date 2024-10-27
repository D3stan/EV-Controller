// #include <ArduinoOTA.h>

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


#include <ArduinoJson.h>
#include <Ticker.h>
#include <LittleFS.h>

#include <ESPAsyncWebServer.h>
#include <WiFiClientSecure.h>
#include <DNSServer.h>

#include "notfound.h"
#include "vars.h"
#include "cert.h"


// ----------------------------------------------------------------------------
// Definition of global constants
// ----------------------------------------------------------------------------

struct Config {
    int wifiMode;
    int raveRpmOpen;
    int raveRpmClose;

    String WIFI_SSID;
    String WIFI_PASS;
    String AP_SSID;
    String AP_PASS;

    String hwid;
    String fw_version;
    String fs_version;

    String lastError;
};

Config config;

// FS globals
bool FSmounted = false;

// WiFi globals
String errorReplacementPage = index_html;
IPAddress AP_IP (42, 42, 42, 42);

// Rpm count
unsigned long RPM = 0;
unsigned long currentMicros = 0;
unsigned long lastMicros = 0;
unsigned long displayMillis = 0;


// WebServer
AsyncWebServer server(HTTP_SERVER_PORT);
AsyncWebSocket ws("/ws");
DNSServer dnsServer;

#if defined(ESP8266)
    X509List cert(IRG_Root_X1);
#endif


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
// Remote Update Utils
// ----------------------------------------------------------------------------

void updateGlobals(JsonDocument config);
void loadConfiguration(const char* filePath, Config& config);
void saveConfiguration(const char* filePath, const Config& config);

String modifyHWID(String hwid) {
    String modifiedHWID = "";
    for (unsigned int i = 0; i < hwid.length(); i++) {
        char ch = hwid[i];
        
        // Skip colons
        if (ch == ':') continue;
        
        // Shift letters backward by 1, wrap around from A to Z
        if (ch >= 'A' && ch <= 'Z') {
            ch = (ch == 'A') ? 'Z' : ch - 1;
        } else if (ch >= 'a' && ch <= 'z') {
            ch = (ch == 'a') ? 'z' : ch - 1;
        }
        
        // Append to the modified HWID
        modifiedHWID += ch;
    }
    return modifiedHWID;
}

void syncTime() {
    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.print("Waiting for NTP time sync: ");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
    }
    Serial.println("");

    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print(asctime(&timeinfo));
}

void update_started() {
  Serial.println("CALLBACK:  HTTP update process started");
}

void update_finished() {
  Serial.println("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total) {
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}


void checkForUpdate(bool firstTime = true) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Not connected");
        return;
    }

    // http client created outside from the HTTPupdate request so
    // we can add custom headers
    WiFiClientSecure wifiClient;
    HTTPClient httpClient;

    #if defined(ESP8266)
        wifiClient.setTrustAnchors(&cert);
    #elif defined(ESP32)
        wifiClient.setCACert(IRG_Root_X1);
    #endif

    // HTTPupdate class does this automatically if we pass the wificlient
    // in this case we are passing it the http client so
    // we have to manually begin the req
    httpClient.begin(wifiClient, update_server_url);  // Start connection to the server
    httpClient.setTimeout(httpTimeout);

    // Add security headers
    httpClient.addHeader("hwid", config.hwid);     // HTTPupdate class adds ESP.getChipId() in the x-ESP8266-Chip-ID header

    // Add functionality headers
    httpClient.addHeader("fwid", config.fw_version);
    httpClient.addHeader("fsid", config.fs_version);
    httpClient.addHeader("device", device);
    httpClient.addHeader("platform", platform);
    httpClient.addHeader("mode", firstTime ? "firmware" : "filesystem");
    
    ESP8266HTTPUpdate myESPhttpUpdate;
    // Add optional led flashing
    myESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

    // Add optional callback notifiers
    myESPhttpUpdate.onStart(update_started);
    myESPhttpUpdate.onEnd(update_finished);
    myESPhttpUpdate.onProgress(update_progress);
    myESPhttpUpdate.onError(update_error);

    Serial.println("Starting update from: " + String(update_server_url));
    t_httpUpdate_return ret = firstTime ? myESPhttpUpdate.update(httpClient) : myESPhttpUpdate.updateFS(httpClient);

    if (ret == HTTP_UPDATE_FAILED) {
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", myESPhttpUpdate.getLastError(), myESPhttpUpdate.getLastErrorString().c_str());
        char err[100];
        sprintf(err, "Update Failed: %s", myESPhttpUpdate.getLastErrorString().c_str());
        config.lastError = err;
        config.wifiMode = WIFI_AP;
        saveConfiguration(CONFIG_FILE_PATH, config);
        delay(1500);
        ESP.restart();

    } else {
        if (ret == HTTP_UPDATE_NO_UPDATES) {
            Serial.println("No updates available");
        } else {
            Serial.printf("Update %s successfully", firstTime ? "firmware" : "filesystem");
        }

        if (firstTime) {
            httpClient.end();
            wifiClient.stop();
            checkForUpdate(false);
        }
        else ESP.restart();
    }

}

void initHwid() {
    // Get MAC Address and remove colons
    String macAddress = WiFi.macAddress();

    // Get Flash Chip ID and convert to hexadecimal string
    String flashChipId = String(ESP.getFlashChipId(), HEX);

    // Combine MAC Address and Flash Chip ID
    String tempHwid = macAddress + flashChipId;

    // Apply modification to HWID
    config.hwid = modifyHWID(tempHwid);

    // Print the modified HWID
    Serial.print("Modified HWID: ");
    Serial.println(config.hwid);
}


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
    Serial.printf("WiFi mode: %d", config.wifiMode);

    if (config.wifiMode == WIFI_AP) {
        Serial.println("\nStarting AP");
        Serial.printf("SSID: %s, PSW: %s, IP: ", config.AP_SSID.c_str(), config.AP_PASS.c_str());
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
        WiFi.softAP(config.AP_SSID.c_str(), config.AP_PASS.c_str());

        Serial.printf("%s\n", WiFi.softAPIP().toString().c_str());

        dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        dnsServer.start(DNS_PORT, "*", AP_IP);
        
    
    } else if (config.wifiMode == WIFI_STA) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(config.WIFI_SSID, config.WIFI_PASS);

        // Wait for WiFi connection
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
        }
        Serial.println("\nWiFi connected");

        Serial.printf(" %s\n", WiFi.localIP().toString().c_str());
        syncTime();
        checkForUpdate();

        
    }
}

// ----------------------------------------------------------------------------
// Web server
// ----------------------------------------------------------------------------

void onRootRequest(AsyncWebServerRequest *request) {
    const char* errorMsg = "";

    if (!FSmounted) {
        errorMsg = "Error: filesystem not mounted correctly";
    } else if (!config.wifiMode) {
        errorMsg = "Error: cannot parse config file";
    }

    errorReplacementPage = index_html;
    errorReplacementPage.replace(errorPlaceHolder, errorMsg);
    errorReplacementPage.replace(errorCodePlaceHolder, "500");
    request->send(500, "text/html", errorReplacementPage);
}

void notFound(AsyncWebServerRequest *request) {
    errorReplacementPage = index_html;
    errorReplacementPage.replace(errorPlaceHolder, "Error: Page Not Found");
    errorReplacementPage.replace(errorCodePlaceHolder, "404");
    request->send(404, "text/html", errorReplacementPage);
}

void initWebServer() {
    if (FSmounted) {
        server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    } else {
        server.on("/", onRootRequest);
    }

    server.onNotFound(notFound);
    server.begin();

    if (!MDNS.begin(device)) {
        Serial.println("Error setting up MDNS responder!");
    }
    MDNS.addService("http", "tcp", 80);

    // ArduinoOTA.begin();
}

void handlehostname() {
    if (config.wifiMode == WIFI_AP) {
        dnsServer.processNextRequest();
        MDNS.update();
    } else {
        MDNS.update();
    }
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
        // Dati in ingresso
        JsonDocument json;
        DeserializationError err = deserializeJson(json, data);
        if (err) {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(err.c_str());
            return;
        }

        const char* dataType = json["type"].as<const char*>();
        if (!dataType) {
            Serial.println("Cannot resolve data type");
            return;
        }
        
        boolean toRestart = false;
        if (!strcmp(dataType, "update")) {
            // start update process
            config.wifiMode = WIFI_STA;
            toRestart = true;

        } else if (!strcmp(dataType, "rave-settings")) {
            // update rave settings
            config.raveRpmOpen = json["raveRpmOpen"];
            config.raveRpmClose = json["raveRpmClose"];

        } else if (!strcmp(dataType, "wifi-settings")) {
            // update wifi settings
            config.WIFI_PASS = json["wifiPsw"].as<String>();
            config.WIFI_SSID = json["wifiSsid"].as<String>();
            config.AP_PASS = json["apPsw"].as<String>();
            config.AP_SSID = json["apSsid"].as<String>();

        } else {
            Serial.println("Unknown data type");
            
        }

        // Sovrascrivo il vecchio config
        saveConfiguration(CONFIG_FILE_PATH, config);
        json.clear();
        JsonDocument doc;
        char data[30];
        
        if (toRestart) {
            doc["update"] = "restarting";
            size_t len = serializeJson(doc, data);
            ws.textAll(data, len);
            doc.clear();
            delay(1500);
            ESP.restart();
        } else {
            doc["update"] = "updated";
            size_t len = serializeJson(doc, data);
            ws.textAll(data, len);
            doc.clear();
        }
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

void loadConfiguration(const char* filePath, Config& config) {
    File configFile = LittleFS.open(filePath, "r");
    if (!configFile) {
        Serial.println("Error reading config file");
        FSmounted = false;
        return;
    }
    
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, configFile);
    if (err) {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.c_str());
        return;
    }
    
    serializeJsonPretty(doc, Serial);
    config.wifiMode     = doc["wifiMode"];
    config.WIFI_PASS    = doc["wifiPsw"].as<const char*>();
    config.WIFI_SSID    = doc["wifiSsid"].as<const char*>();
    config.AP_SSID      = doc["apSsid"].as<const char*>();
    config.AP_PASS      = doc["apPsw"].as<const char*>();
    config.fw_version   = fw_version;
    config.fs_version   = doc["fsid"].as<const char*>();
    config.raveRpmOpen  = doc["raveRpmOpen"];
    config.raveRpmClose = doc["raveRpmClose"];
    config.lastError    = doc["lastError"].as<const char*>();
    
    doc.clear();
    configFile.close();
}

void saveConfiguration(const char* filePath, const Config& config) {
    
    LittleFS.remove(filePath);
    File configFile = LittleFS.open(filePath, "w");
    if (!configFile) {
        Serial.println("Error reading config file");
        FSmounted = false;
        return;
    }
    JsonDocument doc;

    // These values are NEVER read from the file, ONLY written
    doc["hwid"] = config.hwid;
    doc["fwid"] = fw_version;

    // NEVER modified
    doc["fsid"] = config.fs_version;

    // Read (modified) and writted
    doc["wifiMode"] = config.wifiMode;
    doc["wifiPsw"] = config.WIFI_PASS;
    doc["wifiSsid"] = config.WIFI_SSID;
    doc["apSsid"] = config.AP_SSID;
    doc["apPsw"] = config.AP_PASS;
    doc["raveRpmOpen"] = config.raveRpmOpen;
    doc["raveRpmClose"] = config.raveRpmClose;
    
    doc["lastError"] = config.lastError;

    serializeJson(doc, configFile);
    configFile.close();
    doc.clear();
}

void initFS() {
    if (!LittleFS.begin()) {
        Serial.println("An error has occurred while mounting LittleFS");
        FSmounted = false;
        return;
    }

    FSmounted = true;
    loadConfiguration(CONFIG_FILE_PATH, config);
    saveConfiguration(CONFIG_FILE_PATH, config); // update fwid
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