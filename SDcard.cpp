#include "SDcard.h"

bool initSDCard(){
  if (!SD.begin(15)) {
    return false;
  }
  return true;
}

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
void readFile(fs::FS &fs, const char * path, String& payload){
  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }
  payload = file.readString();
  file.close();
}
