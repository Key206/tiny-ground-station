#include <Arduino.h>
#include <Sgp4.h>

#define GMT_OFFSET_SECOND           7
#define DAYLIGHT_OFFSET_SECOND      25200

void getEpochTimeNow(unsigned long& epochTime);
void updateEpochTimeNow(const char* ntpServerName, unsigned long& epochTime);
unsigned long unixTimestamp(int year, int month, int day, int hour, int min, int sec);
void initialize_Sat(String nameOfSat, Sgp4& sat, String payload);
