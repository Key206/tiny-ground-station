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

//AccelStepper motorAz(MOTOR_INTERFACE_TYPE, STEP_AZ, DIR_AZ);
//AccelStepper motorEl(MOTOR_INTERFACE_TYPE, STEP_EL, DIR_EL);

String listWantSat[6] = {"Norbi", "FossaSat-2E8", "GaoFen-7", "GaoFen-13", "GaoFen-17", "GaoFen-19"};
float paramsNorbi[4] = {436.703, 250, 10, 5};
float paramsFossa8[4] = {401.7, 125, 11, 8}; 
float paramsGaoFen[4] = {400.45, 500, 9, 5};

String payload;
unsigned long epochNow = 1660138928;
String* upcomingSatList;

void setup() {
  Serial.begin(9600);
  WiFi.begin(SSID_WIFI, PASSWORD_WIFI);
  while (WiFi.status() != WL_CONNECTED);
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  //motorAz.setMaxSpeed(1000);
  //motorAz.setAcceleration(1000);
  //motorEl.setMaxSpeed(1000);
  //motorEl.setAcceleration(1000);
  
  configTime(GMT_OFFSET_SECOND, DAYLIGHT_OFFSET_SECOND, SERVER_NTP);
  getEpochTimeNow(epochNow);
  Serial.println(epochNow);
  
  updateTleData(&http, payload, SERVER_TLE_TINYGS);
  mySat.site(10.954,106.852,18);
  
  upcomingSatList = new String[NUM_OF_SAT];
  getAllSatName(upcomingSatList, payload);
  createUpcomingList(upcomingSatList, mySat, payload, epochNow);
  for(int i = 0; i < NUM_OF_SAT; i++){
    Serial.println(upcomingSatList[i]);
  }
  Serial.print("Higher: "); Serial.println(getHigherSat(upcomingSatList));
  delay(5000);
  //INIT SD CARD
  status.stateSD = initSDCard();
  File file = SD.open("/LoRa.txt");
  if(!file) {
    Serial.println("File doesn't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/LoRa.txt", "JSON message \r\n");
  }
  else {
    Serial.println("File already exists");  
  }
  file.close();
  
}
void loop() {
  Serial.println("Hello world");
  delay(5000);
  getEpochTimeNow(epochNow);
  Serial.println(epochNow);
  sortUpcomingList(upcomingSatList, mySat, payload, epochNow);
  Serial.println("Update higher: ");
  for(int i = 0; i < NUM_OF_SAT; i++){
    Serial.println(upcomingSatList[i]);
  }
  delay(5000);
  initialize_Sat(upcomingSatList[0], mySat, payload);
  mySat.findsat(epochNow);
  //rotateInTrackingMode(motorAz, motorEl, mySat, false);
  delay(20000);
}

void configParamsLoRa(Status& param, SX1278& myRadio, String* wantList){
  if(wantList[0] == "Norbi"){
    param.modeminfo.satellite = "Norbi"; 
    initLoRa(param, paramsNorbi,myRadio);
  }else if(wantList[0] == "FossaSat-2E8"){
    param.modeminfo.satellite = "FossaSat-2E8";
    initLoRa(param, paramsFossa8,myRadio);
  }else{
    param.modeminfo.satellite = wantList[0];
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
    Serial.println("deo!");
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
