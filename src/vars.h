#ifndef VARS_MODULE
#define VARS_MODULE

/*
// Macros
#define LED_PIN             15
#define BTN_PIN             5 
#define INDUCTIVE_IN        4
#define HTTP_SERVER_PORT    80
#define DNS_PORT            53
#define CONFIG_FILE_PATH "/config.json"
*/

extern const int my_led_pin;
extern const int my_btn_pin;
extern const int my_inductive_in;
extern const int my_http_server_port;
extern const int my_dns_port;
extern const char* config_file_path;


// Device infos
extern const char* fw_version;
extern const char* fs_version;        // unused
extern const char* device;     // also used for mdns
extern const char* platform ;

// Server
extern unsigned int httpTimeout;
extern const char* update_server_url;

#endif