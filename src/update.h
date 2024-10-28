#ifndef UPDATE_MODULE
#define UPDATE_MODULE


// Include of commons libraries
#include "Commons.h"


// Global variables used
// Declare of the custom structure
extern Config config;
extern AsyncWebServer server;
extern DNSServer dnsServer;
extern IPAddress AP_IP;

extern const char* config_file_path; 
extern const char* device;
extern const char* platform;
extern unsigned int httpTimeout;
extern const char* update_server_url;

extern const char IRG_Root_X1 [] PROGMEM;

// Global functions used
extern void saveConfiguration(const char* filePath, const Config& config);



// Exported functions
void checkForUpdate(bool firstTime = true);



#endif