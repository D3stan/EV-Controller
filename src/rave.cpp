#include "rave.h"

Ticker peakAndHoldTimer;
Ticker hysteresisTimer;
bool reachedRpmOpenFirstTime = false;

void updateRPM(unsigned long rpm = 0) {
    JsonDocument json;
    json["rpm"] = rpm;

    char data[30];
    size_t len = serializeJson(json, data);
    ws.textAll(data, len);
    displayMillis = millis();
}

void checkIfEngineRunnig(unsigned long current) {
    if ((RPM > 0 && ((current - (lastMicros / 1000)) >= (1 * 1000)))) {
        RPM = 0;
        Serial.printf("\nResetted rpm");
    }

    checkEngineMillis = millis();
}

void setValveState(int rpm) {
    if (rpm > 0) {

        if (!reachedRpmOpenFirstTime && rpm > config.raveRpmOpen) {
            reachedRpmOpenFirstTime = true;
        } else if (hysteresisTimer.active()) {
            return;
        }

        if (reachedRpmOpenFirstTime && (rpm > config.raveRpmOpen || rpm < config.raveRpmClose) && !raveOpen) {
            raveOpen = true;
            operateValve(valveOut, OPEN);

        } else if (raveOpen && (rpm < config.raveRpmOpen && rpm > config.raveRpmClose)) {
            raveOpen = false;
            operateValve(valveOut, CLOSE);

        }

    } else if (raveOpen) {
        raveOpen = false;
        operateValve(valveOut, CLOSE);
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
    hysteresisTimer.once_ms(config.hysteresisMillis, [] {});
}