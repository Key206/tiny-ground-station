#include <RadioLib.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <base64.h>
//#include <Firebase_ESP_Client.h>
#include "Status.h"
#include "SDcard.h"

#ifndef _PREDICT_
  #include "predictSat.h"
#endif

#define BEGIN_LORA_OK           1
#define BEGIN_LORA_FAULT        0
#define EEPROM_SIZE             2
#define ADDR_ID_EEPROM          0
#define ADDR_MODE_EEPROM        1
#define LNA_GAIN                0

#define API_KEY                 "AIzaSyBu1oFsiXLwDM_QzTEmeNTXlD0-NKakTp8"
#define DATABASE_URL            "https://tinygs2-default-rtdb.asia-southeast1.firebasedatabase.app/"
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

bool beginLoRa(SX1278& radio, bool isPassing);
void setFlag(void);
void listenRadio(SX1278& radio);
void saveDataToSD(String packet);
bool configParamsLoRa(Status& param, SX1278& myRadio, String orderSat, bool isPassing);
bool initLoRa(Status& param, float* paramsSat, SX1278& myRadio, bool isPassing);
bool initFirebase();
bool sendPacketToDatabase();
