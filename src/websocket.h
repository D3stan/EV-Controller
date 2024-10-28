#ifndef WEBSOCKET_MODULE
#define WEBSOCKET_MODULE


// Include of commons libraries
#include "Commons.h"



// Export
void initWebSocket();



// Import
extern Config config;
extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern const char* config_file_path;

extern void saveConfiguration(const char* filePath, const Config& config);
extern void checkForUpdate(bool firstTime);
extern void syncTime();






#endif