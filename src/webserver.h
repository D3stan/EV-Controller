#ifndef WEBSERVER_MODULE
#define WEBSERVER_MODULE


// Include of commons libraries
#include "Commons.h"

extern boolean FSmounted;
extern Config config;
extern String errorReplacementPage;
extern AsyncWebServer server;
extern DNSServer dnsServer;

extern const int my_http_server_port;
extern const char* device;

extern const char index_html[] PROGMEM;
extern const char* errorCodePlaceHolder;
extern const char* errorPlaceHolder;

void handlehostname();
void initWebServer();

#endif