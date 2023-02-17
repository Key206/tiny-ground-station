#include "FS.h"
#include "SD.h"
#include <SPI.h>

bool initSDCard(void);
bool writeFile(fs::FS &fs, const char * path, const char * message);
bool appendFile(fs::FS &fs, const char * path, const char * message);
