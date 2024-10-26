#include <Arduino.h>//la donna e- bellissima

// Macros
#define LED_PIN      15
#define BTN_PIN      5 
#define INDUCTIVE_IN 4
#define HTTP_SERVER_PORT    80

// Programm
const char* configFileLocation = "/config.json";

// Device infos
const char* fw_version = "FW_1_0_2";
const char* fs_version = "FS_1_0_0";
const char* device = "rave-controller";
const char* platform = "d1_mini";

// Server
uint16_t httpTimeout = 5000;
const char* update_server_url = "https://server-updater.deno.dev/update";