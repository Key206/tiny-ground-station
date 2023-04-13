#include <Sgp4.h>
#include "tleFunction.h"

#define GMT_OFFSET_SECOND           7
#define DAYLIGHT_OFFSET_SECOND      25200
#define PREDICT_OFFSET_SECOND       600
#define NUM_WANT_SAT                6

struct timeInfoSat{
  unsigned long epochStart = 0;
  unsigned long epochStop = 0;
  unsigned long epochMax = 0;
};
extern timeInfoSat epochInfo;

void getEpochTimeNow(unsigned long& epochTime);
void updateEpochTimeNow(const char* ntpServerName, unsigned long& epochTime);
unsigned long unixTimestamp(int year, int month, int day, int hour, int min, int sec);
void initialize_Sat(String nameOfSat, Sgp4& sat, String payload);
unsigned long Predict(Sgp4& sat, unsigned long unix_t);
void createUpcomingList(String* listUpcomingSat, Sgp4 sat, String payload, unsigned long unix_t);
void createUpcomingWantList(String* listUpcomingSat, Sgp4 sat, String payload, unsigned long unix_t);
String getHigherSat(String* listUpcomingSat);
void sortUpcomingList(String* listUpcomingSat, Sgp4 sat, String payload, unsigned long unix_t);
