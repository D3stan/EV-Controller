#ifndef WEBSERVER_MODULE
#define WEBSERVER_MODULE


// Include of commons libraries
#include "Commons.h"

extern boolean FSmounted;
extern Config config;
extern String errorReplacementPage;
extern AsyncWebServer server;
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

extern const char index_html[] PROGMEM;
extern const char* errorCodePlaceHolder;
extern const char* errorPlaceHolder;

void handlehostname();
void initWebServer();

#endif