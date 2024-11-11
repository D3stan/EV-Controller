#include "rave.h"

Ticker peakAndHoldTimer;
Ticker hysteresisTimer;
bool reachedRpmOpenFirstTime = false;



void updateRPM(unsigned long rpm = 0) {
    // TO test --> jsondocument global and reuse it with .clear()
    JsonDocument json;
    json["rpm"] = rpm;

    char data[30];
    size_t len = serializeJson(json, data);
    ws.textAll(data, len);

    // Tacho handling



    displayMillis = millis();
}

void checkIfEngineRunnig(unsigned long current) {
    if ((RPM > 0 && ((current - (lastMicros / 1000)) >= (1 * 1000)))) {
        RPM = 0;
        Serial.printf("\nResetted rpm");
    }

    checkEngineMillis = millis();
}

void valveHold(int outputPin) {
    // This function will be called after the specified delay
    Serial.printf("\nValve in hold");
    #if defined(ESP8266)
        analogWrite(outputPin, 102);
    #elif defined(ESP32)
        ledcWrite(outputPin, 102);
    #endif
}

void setValveState(int rpm) {
    if (raveManualOpen) return;
    if (rpm > 0) {

        if (!reachedRpmOpenFirstTime && rpm > config.raveRpmOpen) {
            reachedRpmOpenFirstTime = true;
        } else if (hysteresisTimer.active()) {
            return;
        }

        if (reachedRpmOpenFirstTime && (rpm > config.raveRpmOpen || rpm < config.raveRpmClose) && !raveOpen) {
            operateValve(valveOut, OPEN);

        } else if (raveOpen && (rpm < config.raveRpmOpen && rpm > config.raveRpmClose)) {
            operateValve(valveOut, CLOSE);

        }

    } else if (raveOpen) {
        operateValve(valveOut, CLOSE);
    }
}

void operateValve(int outputPin, Valve mode, int peakMillis) {
    if (mode == OPEN) {
        raveOpen = true;
        Serial.printf("\nValve opened");
        #if defined(ESP8266)
            analogWrite(outputPin, 100);
        #elif defined(ESP32)
            //
        #endif
        peakAndHoldTimer.once_ms(peakMillis, &valveHold, outputPin);
    } else {
        Serial.printf("\nValve closed");
        raveOpen = false;
        #if defined(ESP8266)
            analogWrite(outputPin, 0);
        #elif defined(ESP32)
            //
        #endif
        
    }
    hysteresisTimer.once_ms(config.hysteresisMillis, [] {});
}