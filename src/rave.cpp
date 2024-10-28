#include "rave.h"

Ticker peakAndHoldTimer;
bool reachedRpmOpenFirstTime = false;

void updateRPM(unsigned long rpm = 0) {
    JsonDocument json;
    json["rpm"] = rpm;

    char data[30];
    size_t len = serializeJson(json, data);
    ws.textAll(data, len);
    displayMillis = millis();
}


void checkIfEngineRunnig() {
    if ((displayMillis - lastMicros * 1000) >= 1 * 1000) {
        RPM = 0;
    }

    checkEngineMillis = millis();
}

void setValveState() {
    if (RPM > 0) {
        if (!reachedRpmOpenFirstTime && RPM > config.raveRpmOpen) {
            reachedRpmOpenFirstTime = true;
        }

        if (reachedRpmOpenFirstTime && (RPM > config.raveRpmOpen || RPM < config.raveRpmClose) && !raveOpen) {
            operateValve(valveOut, OPEN);
            raveOpen = true;

        } else if (raveOpen) {
            operateValve(valveOut, CLOSE);
            raveOpen = false;
        }
    }
}

void operateValve(int outputPin, Valve mode, int peakMillis) {
    if (mode == OPEN) {
        Serial.printf("\nValve opened");
        analogWrite(outputPin, 100);
        peakAndHoldTimer.once_ms(peakMillis, [outputPin] {
            // switch to hold state
            Serial.printf("\nValve in hold");
            analogWrite(outputPin, 40);
        });
    } else {
        Serial.printf("\nValve closed");
        analogWrite(outputPin, 0);
    }
}