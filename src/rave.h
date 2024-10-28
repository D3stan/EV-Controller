#ifndef RAVE_MODULE
#define RAVE_MODULE

// Include of commons libraries
#include "Commons.h"

// Export
void updateRPM(unsigned long rpm);




// Import
extern Config config;
extern AsyncWebSocket ws;
extern unsigned long displayMillis;
extern unsigned long currentMicros;
extern unsigned long lastMicros;
extern unsigned long RPM;

#endif