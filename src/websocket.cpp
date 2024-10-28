#include "websocket.h"

// ----------------------------------------------------------------------------
// WebSocket
// ----------------------------------------------------------------------------

// Local variables
Ticker restartTimer;
AsyncWebSocket ws("/ws");

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
        
        boolean toRestart = false, settingsUpdate = false;
        if (!strcmp(dataType, "update")) {
            // start update process
            config.wifiMode = WIFI_STA;
            toRestart = true;

        } else if (!strcmp(dataType, "rave-settings")) {
            // update rave settings
            config.raveRpmOpen = json["raveRpmOpen"];
            config.raveRpmClose = json["raveRpmClose"];
            settingsUpdate = true;

        } else if (!strcmp(dataType, "wifi-settings")) {
            // update wifi settings
            config.WIFI_PASS = json["wifiPsw"].as<String>();
            config.WIFI_SSID = json["wifiSsid"].as<String>();
            config.AP_PASS = json["apPsw"].as<String>();
            config.AP_SSID = json["apSsid"].as<String>();
            settingsUpdate = true;

        } else if (!strcmp(dataType, "reset-error")) {
            config.lastError = "";

        } else {
            Serial.println("Unknown data type");
            
        }

        // Sovrascrivo il vecchio config
        saveConfiguration(config_file_path, config);
        json.clear();
        JsonDocument doc;
        char data[60];
        
        if (toRestart) {
            doc["update"] = "restarting";
            size_t len = serializeJson(doc, data);
            ws.textAll(data, len);
            doc.clear();
            restartTimer.once(2, []() {ESP.restart();});
        } else if (settingsUpdate) {
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