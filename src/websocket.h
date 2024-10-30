#ifndef WEBSOCKET_MODULE
#define WEBSOCKET_MODULE


// Include of commons libraries
#include "Commons.h"
#include "update.h"
#include "utils.h"


// Export
void initWebSocket();


// Import
extern Config config;
extern AsyncWebServer server;
extern const char* config_file_path;



#endif