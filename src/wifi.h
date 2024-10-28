#ifndef WIFI_MODULE
#define WIFI_MODULE


// Include of commons libraries
#include "Commons.h"

// Local Modules
#include "update.h"

// Global variables used
extern Config config;
extern IPAddress AP_IP;
extern DNSServer dnsServer;

extern const int my_led_pin;
extern const int my_btn_pin;
extern const int my_inductive_in;
extern const int my_http_server_port;
extern const int my_dns_port;
extern const char* config_file_path; 
extern const char* fw_version;
extern const char* fs_version;
extern const char* device;
extern const char* platform;
extern unsigned int httpTimeout;
extern const char* update_server_url;

// Global functions used
extern void saveConfiguration(const char* filePath, const Config& config);
extern void syncTime();


void initWiFi();


#endif