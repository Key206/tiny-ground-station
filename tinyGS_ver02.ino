#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "controlStepper.h"
#include "Radio.h"
#include "ESPAsyncWebServer.h"
#include <SPIFFS.h>

#define SSID_WIFI                             "Thanh Tai"
#define PASSWORD_WIFI                         "123456789"

#define URL_TLE_TINYGS                        "https://api.tinygs.com/v1/tinygs_supported.txt"
#define SERVER_NTP                            "pool.ntp.org"

#define uS_TO_S_FACTOR                        1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_PREPARE_AFTER_WAKEUP             10          /* in second */
#define TIME_ACCEPT_PASS_LISTEN               5          /* in second */

/* Create OBJ use for prediction, LoRa, epoch time */
Sgp4 mySat;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
timeInfoSat epochInfo;
Status status;
SX1278 radio = new Module(5, 4, 27, 17);
AsyncWebServer server(80);
/* Create order sat list */
String orderSatList[8] = {"Norbi", "FossaSat-2E8", "FossaSat-2E11", "FossaSat-2E12", "GaoFen-7", "GaoFen-13", "GaoFen-19", "GaoFen-17"};
/* Create global variables */
String payload;
unsigned long epochNow = 1660138928;
//String* upcomingSatList;
uint8_t totalSat = 0;
uint8_t posInList = 0;

void setup() {
  Serial.begin(9600);
  WiFi.begin(SSID_WIFI, PASSWORD_WIFI);
  while (WiFi.status() != WL_CONNECTED);
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  configTime(GMT_OFFSET_SECOND, DAYLIGHT_OFFSET_SECOND, SERVER_NTP);
  getEpochTimeNow(epochNow);
  Serial.println(epochNow);
  
  updateTleData(payload, URL_TLE_TINYGS);
  mySat.site(10.954,106.852,18);
  totalSat = NUM_ORDER_SAT;
  createUpcomingOrderList(orderSatList, mySat, payload);
  
  EEPROM.begin(EEPROM_SIZE);
  if((EEPROM.read(ADDR_ID_EEPROM) > 0) && (EEPROM.read(ADDR_ID_EEPROM) < 255)){
    status.lastPacketInfo.id = EEPROM.read(ADDR_ID_EEPROM);
  }
  initFirebase();

  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  // Route for root / web page
  createWebPage();
  // Start server
  server.begin();
}
void loop(){
  getEpochTimeNow(epochNow);
  Serial.print("now: "); Serial.println(epochNow);
  initialize_Sat(orderSatList[posInList], mySat, payload);
  status.statePredict = Predict(mySat, epochNow);
  if(epochInfo.epochStart - TIME_PREPARE_AFTER_WAKEUP > epochNow){
    uint64_t timeToSleep = calculateSleepTime(epochNow, epochInfo.epochStart);
    goToSleep(timeToSleep);
  }else if(epochNow < epochInfo.epochStop){
    Serial.print("Sat listen: "); Serial.println(orderSatList[posInList]);
    configParamsLoRa(status, radio, orderSatList[posInList]);
    while(epochNow <= epochInfo.epochStop){
      listenRadio(radio);
      getEpochTimeNow(epochNow);
    }
    Serial.println("update TLE");
    updateTleData(payload, URL_TLE_TINYGS);
    sortUpcomingList(orderSatList, mySat, payload, totalSat);
    posInList = 0;
  }else{
    ++posInList;
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
void createWebPage(void){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/rssi", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(status.lastPacketInfo.rssi).c_str());
  });
  server.on("/packet", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(status.lastPacketInfo.packet).c_str());
  });
  server.on("/checkLoRa", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(status.stateLoRa).c_str());
  });
  server.on("/checkSD", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(status.stateSD).c_str());
  });
  server.on("/checkPredict", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(status.statePredict).c_str());
  });
  server.on("/checkEpoch", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(epochNow).c_str());
  });
  server.on("/winter", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/winter.jpg", "image/jpg");
  });
}
String processor(const String& var){
  //Serial.println(var);
  if (var == "RRSI"){
    return String(status.lastPacketInfo.rssi);
  }else if(var == "PACKET"){
    return String(status.lastPacketInfo.packet);
  }else if(var == "LORA"){
    return ((String)status.stateLoRa);
  }else if(var == "SD"){
    return ((String)status.stateSD);
  }else if(var == "PREDICT"){
    return ((String)status.statePredict);
  }else if(var == "EPOCH"){
    return ((String)epochNow);
  }else{
    return String();
  } 
}
