#ifndef WIFI_MODULE
#define WIFI_MODULE


// Include of commons libraries
#include "Commons.h"

// Local Modules
#include "update.h"

// Export
void initWiFi();

extern IPAddress AP_IP;



// Import
extern Config config;
// extern DNSServer dnsServer;
extern const int my_dns_port;
extern const char* config_file_path;

extern void saveConfiguration(const char* filePath, const Config& config);
extern void syncTime();


#endif