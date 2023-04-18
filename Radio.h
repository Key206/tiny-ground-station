#include <RadioLib.h>
#include <base64.h>
#include <ArduinoJson.h>
#include "Status.h"
#include "SDcard.h"

#ifndef _PREDICT_
  #include "predictSat.h"
#endif
#define BEGIN_LORA_OK           1
#define BEGIN_LORA_FAULT        0

#define API_KEY                 "AIzaSyA7SwCrgGEMQWbH_J0mSVIN77O6N388p0g"
#define DATABASE_URL            "https://thongletest-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define DATABASE_PATH           "/UsersData/packages"
#define SAT_PATH                "/satellite"
#define EPOCH_PATH              "/epoch"
#define PACKET_PATH             "/packet"
#define RSSI_PATH               "/rssi"
#define SNR_PATH                "/snr"
#define LON_PATH                "/longitude"
#define LAT_PATH                "/latitude"
#define TIMESTAMP_PATH          "/timestamp"

extern Status status;
extern Sgp4 mySat;

bool beginLoRa(SX1278& radio);
void setFlag(void);
void listenRadio(SX1278& radio);
void saveDataToSD(String packet);
void configParamsLoRa(Status& param, SX1278& myRadio, String orderSat);
void initLoRa(Status& param, float* arr, SX1278& myRadio);
bool initFirebase();
bool sendPacketToDatabase();
