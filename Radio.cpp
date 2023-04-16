#include "Radio.h"

float paramsNorbi[4] = {436.703, 250, 10, 5};
float paramsFossa[4] = {401.7, 125, 11, 8}; 
float paramsGaoFen[4] = {400.45, 500, 9, 5};

volatile bool receivedFlag = false;
volatile bool enableInterrupt = true;

void setFlag(void) {
  if(!enableInterrupt) {
    return;
  }
  receivedFlag = true;
}

bool beginLoRa(SX1278& radio)
{
  ModemInfo &m = status.modeminfo;
  int state = radio.begin(m.frequency, m.bw, m.sf, m.cr, m.sw, m.power, m.preambleLength, m.gain);
  if (state == RADIOLIB_ERR_NONE) {
    radio.setCRC(m.crc);
    radio.forceLDRO(m.fldro);
    if (radio.setCurrentLimit(120) == RADIOLIB_ERR_INVALID_CURRENT_LIMIT) {
      Serial.println(F("Selected current limit is invalid for this module!"));
    }
    radio.setDio0Action(setFlag);
    state = radio.startReceive();
    if (state == RADIOLIB_ERR_NONE) {
      return BEGIN_LORA_OK;
    } else {
      return BEGIN_LORA_FAULT;
    }
  } else {
    return BEGIN_LORA_FAULT;
  }
}

void listenRadio(SX1278& radio)
{
  if(receivedFlag) {
    enableInterrupt = false;
    receivedFlag = false;
    
    size_t respLen = 0;
    uint8_t *respFrame = 0;
    int16_t state = 0;
    
    respLen = radio.getPacketLength();
    respFrame = new uint8_t[respLen];
    state = radio.readData(respFrame, respLen);
    status.lastPacketInfo.rssi = radio.getRSSI();
    status.lastPacketInfo.snr = radio.getSNR();
    //status.lastPacketInfo.frequencyerror = radio.getFrequencyError();
    Serial.println("received");
    if(state == RADIOLIB_ERR_NONE){
      String encoded = base64::encode(respFrame, respLen);
      status.lastPacketInfo.packet = encoded;
      //saveDataToSD(encoded);
    }
    delete[] respFrame;
    enableInterrupt = true;
    radio.startReceive();
  }
} 
void saveDataToSD(String packet){
  StaticJsonDocument<300> JSONbuffer;
  //unsigned long epochtime = 0;
  JSONbuffer["satellite"] = status.modeminfo.satellite;
  //getEpochTimeNow(epochtime);
 // JSONbuffer["epoch"] = epochtime;
  JSONbuffer["packet"] = packet;
  JSONbuffer["rssi"] = status.lastPacketInfo.rssi;
  JSONbuffer["snr"] = status.lastPacketInfo.snr;
  char JSONmessageBuffer[300]; // be careful the size of buffer
  serializeJson(JSONbuffer, JSONmessageBuffer);
  status.stateSD = appendFile(SD, "/LoRa.txt", JSONmessageBuffer);
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
