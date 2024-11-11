#include "wifi.h"

// ----------------------------------------------------------------------------
// Connecting to the WiFi network
// ----------------------------------------------------------------------------

IPAddress AP_IP(42, 42, 42, 42);

void initWiFi() {
    Serial.printf("WiFi mode: %d", config.wifiMode);

    if (config.wifiMode == WIFI_AP) {
        Serial.println("\nStarting AP");
        Serial.printf("SSID: %s, PSW: %s, IP: ", config.AP_SSID.c_str(), config.AP_PASS.c_str());
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255, 255, 255, 0));
        WiFi.softAP(config.AP_SSID.c_str(), config.AP_PASS.c_str());

        Serial.printf("%s\n", WiFi.softAPIP().toString().c_str());

        //dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
        //dnsServer.start(my_dns_port, "*", AP_IP);
        
    
    } else if (config.wifiMode == WIFI_STA) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(config.WIFI_SSID, config.WIFI_PASS);

        // Wait for WiFi connection
        for (int i = 0; WiFi.status() != WL_CONNECTED; i++) {
            delay(500);
            Serial.print(".");

            if (i >= 40) {
                config.wifiMode = WIFI_AP;
                config.lastMessage = "Incorrect WiFi password!";
                saveConfiguration(config_file_path, config);
                delay(1500);
                ESP.restart();
            }
        }
        Serial.println("\nWiFi connected");

        Serial.printf(" %s\n", WiFi.localIP().toString().c_str());
        syncTime();
        checkForUpdate();

        
    }
}