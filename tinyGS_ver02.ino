#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "controlStepper.h"
#include "Radio.h"
#include "ESPAsyncWebServer.h"
#include <SPIFFS.h>

#define SSID_WIFI                             "RFThings Vietnam"
#define PASSWORD_WIFI                         "khongvaoduoc!"

#define URL_TLE_TINYGS                        "https://api.tinygs.com/v1/tinygs_supported.txt"
#define SERVER_NTP                            "pool.ntp.org"

#define uS_TO_S_FACTOR                        1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_PREPARE_AFTER_WAKEUP             10          /* in second */
#define TIME_ACCEPT_PASS_LISTEN               5          /* in second */
#define SAVE_MODE                             0
#define FOCUS_MODE                            1

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
uint8_t totalSat = 0, posInList = 0;
uint8_t state = FOCUS_MODE;

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
  Serial.println("Check 1");
  mySat.site(10.954,106.852,18);
  totalSat = NUM_ORDER_SAT;
  createUpcomingOrderList(orderSatList, mySat, payload);
  Serial.println("Check 2");
  
  EEPROM.begin(EEPROM_SIZE);
  if((EEPROM.read(ADDR_ID_EEPROM) > 0) && (EEPROM.read(ADDR_ID_EEPROM) < 255)){
    status.lastPacketInfo.id = EEPROM.read(ADDR_ID_EEPROM);
  }
  Serial.println("Check 3");
  initFirebase();
  Serial.println("Check 4");
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  createWebPage();
  server.begin();
  Serial.println("Check 5");
  status.stateSD = initSDCard();
  File file = SD.open("/LoRa.txt");
  if(!file) {
    Serial.println("File doesn't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/alo.txt", "JSON message \r\n");
  }
  else {
    Serial.println("File already exists");  
  }
  file.close();
}
void loop(){
  getEpochTimeNow(epochNow);
  //Serial.print("now: "); Serial.println(epochNow);
  initialize_Sat(orderSatList[posInList], mySat, payload);
  status.statePredict = Predict(mySat, epochNow);

  switch(state){
    case SAVE_MODE:
      intoModeOP(posInList, epochNow, totalSat, SAVE_MODE);
      break;
    case FOCUS_MODE:
      intoModeOP(posInList, epochNow, totalSat, FOCUS_MODE);
      break;
    default:
      break;
  }
}
void intoModeOP(uint8_t& positionInOrder, unsigned long& unixtNow, uint8_t totalSatOrder, uint8_t modeOP){
  if(epochInfo.epochStart - TIME_PREPARE_AFTER_WAKEUP > unixtNow){
    if(modeOP == SAVE_MODE){
      uint64_t timeToSleep = calculateSleepTime(unixtNow, epochInfo.epochStart);
      goToSleep(timeToSleep);
    }else{
      configParamsLoRa(status, radio, "GaoFen-x");
      unsigned long unixtStart = epochInfo.epochStart - TIME_PREPARE_AFTER_WAKEUP;
      while(unixtStart > unixtNow){
        listenRadio(radio);
        getEpochTimeNow(unixtNow);
      }
    }
  }else if(unixtNow < epochInfo.epochStop){
    Serial.print("Sat listen: "); Serial.println(orderSatList[posInList]);
    configParamsLoRa(status, radio, orderSatList[posInList]);
    while(unixtNow <= epochInfo.epochStop){
      listenRadio(radio);
      getEpochTimeNow(unixtNow);
    }
    Serial.println("update TLE");
    updateTleData(payload, URL_TLE_TINYGS);
    sortUpcomingList(orderSatList, mySat, payload, totalSatOrder);
    positionInOrder = 0;
  }else{
    ++positionInOrder;
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
