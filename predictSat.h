#include <Sgp4.h>
#include "tleFunction.h"

#define GMT_OFFSET_SECOND           7
#define DAYLIGHT_OFFSET_SECOND      25200
#define PREDICT_OFFSET_SECOND       600
#define NUM_ORDER_SAT               4
#define FAIL_CODE                   0
#define SUCCESS_CODE                1

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
bool createUpcomingList(String* listUpcomingSat, Sgp4 sat, String payload);
bool createUpcomingOrderList(String* listUpcomingSat, Sgp4 sat, String payload);
String getHigherSat(String* listUpcomingSat);
bool sortUpcomingList(String* listUpcomingSat, Sgp4 sat, String payload, uint8_t totalSat);
