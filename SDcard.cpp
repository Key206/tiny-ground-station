#include "SDcard.h"

bool initSDCard(){
  if (!SD.begin(15)) {
    return false;
  }
  return true;
}

// Write to the SD card
bool writeFile(fs::FS &fs, const char * path, const char * message) {
  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    return false;
  }
  if(file.print(message)) {
    file.close();
    return true;
  } else {
    file.close();
    return false;
  }
}
// Append data to the SD card
bool appendFile(fs::FS &fs, const char * path, const char * message) {
  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    return false;
  }
  if(file.print(message)) {
    file.close();
    return true;
  } else {
    file.close();
    return false;
  }
}
