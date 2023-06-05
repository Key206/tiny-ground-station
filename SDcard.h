#include "FS.h"
#include "SD.h"
#include <SPI.h>

#define PATH_TLE_IN_SD          "/TLE.txt"

bool initSDCard(void);
bool writeFile(fs::FS &fs, const char * path, const char * message);
bool appendFile(fs::FS &fs, const char * path, const char * message);
void readFile(fs::FS &fs, const char * path, String& payload);
void saveTleDataToSD(String& payload);
void updateTleDataFromSD(String& payload);
