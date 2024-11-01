#include "utils.h"

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
    config.wifiMode         = doc["wifiMode"];
    config.WIFI_PASS        = doc["wifiPsw"].as<const char*>();
    config.WIFI_SSID        = doc["wifiSsid"].as<const char*>();
    config.AP_SSID          = doc["apSsid"].as<const char*>();
    config.AP_PASS          = doc["apPsw"].as<const char*>();
    config.fw_version       = fw_version;
    config.fs_version       = doc["fsid"].as<const char*>();
    config.raveRpmOpen      = doc["raveRpmOpen"];
    config.raveRpmClose     = doc["raveRpmClose"];
    config.lastMessage        = doc["lastMessage"].as<const char*>();
    config.hysteresisMillis = doc["hysteresisMillis"];
    
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
    doc["hwid"]                 = config.hwid;
    doc["fwid"]                 = fw_version;

    // NEVER modified
    doc["fsid"]                 = config.fs_version;

    // Read (modified) and writted
    doc["wifiMode"]             = config.wifiMode;
    doc["wifiPsw"]              = config.WIFI_PASS;
    doc["wifiSsid"]             = config.WIFI_SSID;
    doc["apSsid"]               = config.AP_SSID;
    doc["apPsw"]                = config.AP_PASS;
    doc["raveRpmOpen"]          = config.raveRpmOpen;
    doc["raveRpmClose"]         = config.raveRpmClose;
    
    doc["lastMessage"]            = config.lastMessage;
    doc["hysteresisMillis"]     = config.hysteresisMillis;

    serializeJson(doc, configFile);
    configFile.close();
    doc.clear();
}

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