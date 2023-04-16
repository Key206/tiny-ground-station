#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "controlStepper.h"
#include "Radio.h"

#define SSID_WIFI                             "Thanh Tai"
#define PASSWORD_WIFI                         "123456789"

#define SERVER_TLE_TINYGS                     "https://api.tinygs.com/v1/tinygs_supported.txt"
#define SERVER_NTP                            "pool.ntp.org"

#define uS_TO_S_FACTOR                        1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_PREPARE_AFTER_WAKEUP             20          /* in second */

Sgp4 mySat;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
HTTPClient http;
timeInfoSat epochInfo;
Status status;
SX1278 radio = new Module(5, 4, 27, 17);

String orderSatList[4] = {"Norbi", "FossaSat-2E8", "FossaSat-2E11", "FossaSat-2E12"};
float paramsNorbi[4] = {436.703, 250, 10, 5};
float paramsFossa[4] = {401.7, 125, 11, 8}; 
float paramsGaoFen[4] = {400.45, 500, 9, 5};

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
  
  updateTleData(&http, payload, SERVER_TLE_TINYGS);
  mySat.site(10.954,106.852,18);

  totalSat = NUM_ORDER_SAT;
  createUpcomingOrderList(orderSatList, mySat, payload);
  /*
  totalSat = getAmountOfSat(payload);
  upcomingSatList = new String[totalSat];
  getAllSatName(upcomingSatList, payload);
  createUpcomingList(upcomingSatList, mySat, payload);
  for(int i = 0; i < totalSat; i++){
    Serial.println(upcomingSatList[i]);
  }
  Serial.print("Higher: "); Serial.println(getHigherSat(upcomingSatList));
  */
}
void loop() {
  listenRadio(radio);
  getEpochTimeNow(epochNow);
  initialize_Sat(orderSatList[0], mySat, payload);
  status.statePredict = Predict(mySat, epochNow);
  if(epochInfo.epochStart - TIME_PREPARE_AFTER_WAKEUP > epochNow){
    configParamsLoRa(status, radio, "GeoFen");
    unsigned long epochStart = epochInfo.epochStart - TIME_PREPARE_AFTER_WAKEUP;
    while(epochStart > epochNow){
      listenRadio(radio);
      getEpochTimeNow(epochNow);
    }
  }else{
    configParamsLoRa(status, radio, orderSatList[0]);
    while(epochNow <= epochInfo.epochStop){
      listenRadio(radio);
      getEpochTimeNow(epochNow);
    }
  }
}

void configParamsLoRa(Status& param, SX1278& myRadio, String orderSat){
  if(orderSat[0] == 'N'){
    param.modeminfo.satellite = "Norbi"; 
    initLoRa(param, paramsNorbi,myRadio);
  }else if(orderSat[0] == 'F'){
    param.modeminfo.satellite = orderSat;
    initLoRa(param, paramsFossa,myRadio);
  }else{
    param.modeminfo.satellite = "GeoFen";
    initLoRa(param, paramsGaoFen,myRadio);
  }
}
void initLoRa(Status& param, float* arr, SX1278& myRadio){
  param.modeminfo.frequency = arr[0]; 
  param.modeminfo.bw = arr[1];
  param.modeminfo.sf = arr[2];
  param.modeminfo.cr = arr[3];
  param.stateLoRa = beginLoRa(myRadio);
  if(!param.stateLoRa){
    Serial.println("Fail");
  }
}
void goToSleep(unsigned int timeToSleep)
{
  esp_sleep_enable_timer_wakeup(timeToSleep * uS_TO_S_FACTOR);
  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start();
}
unsigned int calculateSleepTime(unsigned long Now, unsigned long Start)
{
  return (unsigned int)(Start - Now - TIME_PREPARE_AFTER_WAKEUP);
}
