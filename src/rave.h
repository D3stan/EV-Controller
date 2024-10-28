#ifndef RAVE_MODULE
#define RAVE_MODULE

// Include of commons libraries
#include "Commons.h"


// Locals
enum Valve {
    OPEN,
    CLOSE
};




// Export
void updateRPM(unsigned long rpm);
void checkIfEngineRunnig();
void operateValve(int outputPin, Valve mode, int peakMillis = 1000);
void setValveState();


// Import
extern Config config;
extern AsyncWebSocket ws;
extern unsigned long displayMillis;
extern unsigned long checkEngineMillis;
extern unsigned long currentMicros;
extern unsigned long lastMicros;
extern int RPM;
extern const int valveOut;
extern bool raveOpen;
#endif