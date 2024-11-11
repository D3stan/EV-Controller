#ifndef UPDATE_MODULE
#define UPDATE_MODULE


// Include of commons libraries
#include "Commons.h"
#include "utils.h"


// Global variables used
// Declare of the custom structure
extern Config config;
extern AsyncWebServer server;
//extern DNSServer dnsServer;
extern IPAddress AP_IP;

extern const char* config_file_path; 
extern const char* device;
extern const char* platform;
extern unsigned int httpTimeout;
extern const char* update_server_url;

extern const char IRG_Root_X1 [] PROGMEM;




// Exported functions
void checkForUpdate(bool firstTime = true);



#endif