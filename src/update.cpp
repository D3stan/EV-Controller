#include "update.h"

#if defined(ESP8266)
    X509List cert(IRG_Root_X1);
#endif

Ticker updateTimer;

// ----------------------------------------------------------------------------
// Remote Update Utils
// ----------------------------------------------------------------------------

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


void checkForUpdate(bool firstTime) {
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

    // Remove automatic reboot
    myESPhttpUpdate.rebootOnUpdate(false);
    // Add optional led flashing
    myESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

    // Add optional callback notifiers
    myESPhttpUpdate.onStart(update_started);
    myESPhttpUpdate.onEnd(update_finished);
    myESPhttpUpdate.onProgress(update_progress);
    myESPhttpUpdate.onError(update_error);

    Serial.println("Starting update from: " + String(update_server_url));
    t_httpUpdate_return ret = firstTime ? myESPhttpUpdate.update(httpClient) : myESPhttpUpdate.updateFS(httpClient);
    char msg[100];

    if (ret == HTTP_UPDATE_FAILED) {
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", myESPhttpUpdate.getLastError(), myESPhttpUpdate.getLastErrorString().c_str());
        sprintf(msg, "Update Failed: %s", myESPhttpUpdate.getLastErrorString().c_str());

    } else {
        if (ret == HTTP_UPDATE_NO_UPDATES) {
            Serial.println("No updates available");
            sprintf(msg, "System already updated");

        } else {
            Serial.printf("Update %s successfully", firstTime ? "firmware" : "filesystem");
            sprintf(msg, "Update successfully");
        }

        if (firstTime) {
            httpClient.end();
            wifiClient.stop();
            checkForUpdate(false);
        }
    }

    config.lastMessage = msg;
    config.wifiMode = WIFI_AP;
    saveConfiguration(config_file_path, config);
    updateTimer.once(1.5, [] {ESP.restart();});

}