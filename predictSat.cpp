#include "predictSat.h"
#include "TLEFunction.h"

void getEpochTimeNow(unsigned long& epochTime){
  struct tm timeinfo;
  int Year, Month, Day, Hour, Minute, Second;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Year = timeinfo.tm_year + 1900;
  Month = timeinfo.tm_mon + 1;
  Day = timeinfo.tm_mday;
  Hour = timeinfo.tm_hour + 7;
  Minute = timeinfo.tm_min;
  Second = timeinfo.tm_sec;
  epochTime = unixTimestamp(Year,Month,Day,Hour, Minute, Second) - DAYLIGHT_OFFSET_SECOND;
}
void updateEpochTimeNow(const char* ntpServerName, unsigned long& epochTime)
{
  configTime(7, 25200, ntpServerName);
  getEpochTimeNow(epochTime);
}
unsigned long unixTimestamp(int year, int month, int day, int hour, int min, int sec) {
  const short days_since_beginning_of_year[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  int leap_years = ((year - 1) - 1968) / 4
                   - ((year - 1) - 1900) / 100
                   + ((year - 1) - 1600) / 400;
  long days_since_1970 = (year - 1970) * 365 + leap_years
                         + days_since_beginning_of_year[month - 1] + day - 1;
  if ( (month > 2) && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) )
    days_since_1970 += 1; /* +leap day, if year is a leap year */
  return sec + 60 * ( min + 60 * (hour + 24 * days_since_1970) );
}
void initialize_Sat(String nameOfSat, Sgp4& sat, String payload)
{
  char arrSatName[20];
  static char tleLine1[70];
  static char tleLine2[70];
  uint8_t len = 0;
  getTLE(nameOfSat, tleLine1, tleLine2, payload);
  Serial.println(tleLine1);
  Serial.println(tleLine2);
  len = nameOfSat.length() + 1; 
  nameOfSat.toCharArray(arrSatName, len);
  sat.init(arrSatName, tleLine1,tleLine2);
}
