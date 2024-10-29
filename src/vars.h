#include <Arduino.h>//la donna e- bellissima

// Macros
#define LED_PIN             15
#define VALVE_OUT           5 
#define INDUCTIVE_IN        4
#define HTTP_SERVER_PORT    80
#define DNS_PORT            53
#define CONFIG_FILE_PATH "/config.json"

// Device infos
const char* fw_version = "FW_1_0_2";
const char* fs_version = "FS_1_0_0";        // unused
const char* device = "rave-controller";     // also used for mdns
const char* platform = "d1_mini";

// Server
uint16_t httpTimeout = 5000;
const char* update_server_url = "https://server-updater.deno.dev/update";