#include "predictSat.h"

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
  //Serial.println(tleLine1);
  //Serial.println(tleLine2);
  len = nameOfSat.length() + 1; 
  nameOfSat.toCharArray(arrSatName, len);
  sat.init(arrSatName, tleLine1,tleLine2);
}
unsigned long Predict(Sgp4& sat, unsigned long unix_t){
  passinfo overpass;                       
  sat.initpredpoint(unix_t , 0.0);       
  int  year; int mon; int day; int hr; int minute; double sec;
  bool nonError;
  nonError = sat.nextpass(&overpass,10);    
  if(nonError){ 
    invjday(overpass.jdmax ,GMT_OFFSET_SECOND ,true , year, mon, day, hr, minute, sec);
    unix_t = getUnixFromJulian(overpass.jdmax);
    return unix_t;
  }else{
    Serial.println("Prediction error");
    return 0;
  }
}
void createUpcomingList(String* listUpcomingSat, Sgp4 sat, String payload, unsigned long unix_t)
{  
  unsigned long* timeMaxElevationList = new unsigned long[NUM_OF_SAT];
  for(uint8_t i = 0; i < NUM_OF_SAT; ++i){
    initialize_Sat(listUpcomingSat[i], sat, payload);
    timeMaxElevationList[i] = Predict(sat, unix_t);
  }
  for(uint8_t i = 0; i < NUM_OF_SAT - 1; ++i){
    for(uint8_t j = i + 1; j < NUM_OF_SAT; ++j){
      if(timeMaxElevationList[i] > timeMaxElevationList[j]){
        String tmp_name = listUpcomingSat[i];
        unsigned long tmp_t = timeMaxElevationList[i];
        
        listUpcomingSat[i] = listUpcomingSat[j];
        timeMaxElevationList[i] = timeMaxElevationList[j];
        
        listUpcomingSat[j] = tmp_name;
        timeMaxElevationList[j] = tmp_t; 
      }
    }
  }
  delete[] timeMaxElevationList;
}
String getHigherSat(String* listUpcomingSat)
{
  return listUpcomingSat[0];
}
void sortUpcomingList(String* listUpcomingSat, Sgp4 sat, String payload, unsigned long unix_t)
{
  unsigned long* timeMaxElevationList = new unsigned long[NUM_OF_SAT];  
  initialize_Sat(listUpcomingSat[0], sat, payload);
  unsigned long tmp_t = unix_t + PREDICT_OFFSET_SECOND;
  timeMaxElevationList[0] = Predict(sat, tmp_t);
  for(uint8_t i = 1; i < NUM_OF_SAT; ++i){
    initialize_Sat(listUpcomingSat[i], sat, payload);
    timeMaxElevationList[i] = Predict(sat, unix_t);
  }
  int j;
  for (uint8_t i = 1; i < NUM_OF_SAT; ++i){
    tmp_t = timeMaxElevationList[i];
    String tmp_name = listUpcomingSat[i];
    j = i - 1;
    while (j >= 0 && timeMaxElevationList[j] > tmp_t)
    {
      timeMaxElevationList[j + 1] = timeMaxElevationList[j];
      listUpcomingSat[j + 1] = listUpcomingSat[j];
      j = j - 1;
    }
    timeMaxElevationList[j + 1] = tmp_t;
    listUpcomingSat[j + 1] = tmp_name;
  }
  delete[] timeMaxElevationList;
}
