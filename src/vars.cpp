#include "vars.h"

const int my_led_pin = 15;
const int valveOut = 5;
const int inductiveIn = 4;
const int my_http_server_port = 80;
const int my_dns_port = 53;
const char* config_file_path = "/config.json";

const char* fw_version = "FW_1_0_2";
const char* fs_version = "FS_1_0_0";        // unused
const char* device = "rave-controller";     // also used for mdns
const char* platform = "d1_mini";
unsigned int httpTimeout = 5000;
const char* update_server_url = "https://server-updater.deno.dev/update";