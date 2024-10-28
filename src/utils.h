#ifndef UTILS_MODULE
#define UTILS_MODULE


// Include of commons libraries
#include "Commons.h"


// Global variables used
// Declare of the custom structure
extern Config config;
extern const char* fw_version;
extern boolean FSmounted;

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


void loadConfiguration(const char* filePath, Config& config);
void saveConfiguration(const char* filePath, const Config& config);
String modifyHWID(String hwid);
void initHwid();
void syncTime();

#endif