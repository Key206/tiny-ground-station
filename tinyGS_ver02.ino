#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "controlStepper.h"
#include "Radio.h"

#define SSID_WIFI                             "Thanh Tai"
#define PASSWORD_WIFI                         "123456789"

#define URL_TLE_TINYGS                     "https://api.tinygs.com/v1/tinygs_supported.txt"
#define SERVER_NTP                            "pool.ntp.org"

#define uS_TO_S_FACTOR                        1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_PREPARE_AFTER_WAKEUP             20          /* in second */

/* Create OBJ use for prediction, LoRa, epoch time */
Sgp4 mySat;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
timeInfoSat epochInfo;
Status status;
SX1278 radio = new Module(5, 4, 27, 17);
/* Create order sat list */
String orderSatList[8] = {"Norbi", "FossaSat-2E8", "FossaSat-2E11", "FossaSat-2E12", "GaoFen-7", "GaoFen-13", "GaoFen-19", "GaoFen-17"};
/* Create global variables */
String payload;
unsigned long epochNow = 1660138928;
String* upcomingSatList;
uint8_t totalSat = 0;

void setup() {
  Serial.begin(9600);
  WiFi.begin(SSID_WIFI, PASSWORD_WIFI);
  while (WiFi.status() != WL_CONNECTED);
  Serial.println("WiFi connected.");
  
  configTime(GMT_OFFSET_SECOND, DAYLIGHT_OFFSET_SECOND, SERVER_NTP);
  getEpochTimeNow(epochNow);
  Serial.println(epochNow);
  
  updateTleData(payload, URL_TLE_TINYGS);
  mySat.site(10.954,106.852,18);
  totalSat = NUM_ORDER_SAT;
  createUpcomingOrderList(orderSatList, mySat, payload);
  
  initFirebase();
}
void loop(){
  getEpochTimeNow(epochNow);
  initialize_Sat(orderSatList[0], mySat, payload);
  status.statePredict = Predict(mySat, epochNow);
  if(epochInfo.epochStart - TIME_PREPARE_AFTER_WAKEUP > epochNow){
    uint64_t timeToSleep = calculateSleepTime(epochNow, epochInfo.epochStart);
    goToSleep(timeToSleep);
  }else{
    configParamsLoRa(status, radio, orderSatList[0]);
    while(epochNow <= epochInfo.epochStop){
      listenRadio(radio);
      getEpochTimeNow(epochNow);
    }
    updateTleData(payload, URL_TLE_TINYGS);
    sortUpcomingList(orderSatList, mySat, payload, totalSat);
  }
}

void goToSleep(uint64_t timeToSleep)
{
  esp_sleep_enable_timer_wakeup(timeToSleep * uS_TO_S_FACTOR);
  Serial.println("Going to sleep now");
  Serial.println(timeToSleep * uS_TO_S_FACTOR);
  Serial.flush(); 
  esp_deep_sleep_start();
}
uint64_t calculateSleepTime(unsigned long Now, unsigned long Start)
{
  return (uint64_t)(Start - Now - TIME_PREPARE_AFTER_WAKEUP);
}
