#ifndef UTILS_MODULE
#define UTILS_MODULE


// Include of commons libraries
#include "Commons.h"


// Global variables used
// Declare of the custom structure
extern Config config;
extern const char* fw_version;
extern boolean FSmounted;


// Export functions
void loadConfiguration(const char* filePath, Config& config);
void saveConfiguration(const char* filePath, const Config& config);
String modifyHWID(String hwid);
void initHwid();
void syncTime();

#endif