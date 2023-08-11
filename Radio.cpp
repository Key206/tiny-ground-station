#include "Radio.h"
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

FirebaseAuth auth;
FirebaseConfig config;
FirebaseData fbdo;
bool signupOK = false;
FirebaseJson json;

float paramsNorbi[6] = {436.703, 250, 10, 5, 18, 8};
float paramsFossa[6] = {401.7, 125, 11, 8, 18, 8}; 
float paramsGaoFen7[6] = {400.45, 500, 9, 5, 18, 8};
float paramsGaoFen19[6] = {400.45, 500, 9, 5, 18, 8};
float paramsSapling2[6] = {437.4, 125, 7, 8, 18, 8};
float paramsSATLLA2B[6] = {437.25, 62.5, 10, 5, 4, 8};
float paramsFEES[6] = {437.2, 125, 9, 5, 18, 6};

volatile bool receivedFlag = false;
volatile bool enableInterrupt = true;

void setFlag(void) {
  if(!enableInterrupt) {
    return;
  }
  receivedFlag = true;
}

bool beginLoRa(SX1278& radio, bool isPassing)
{
  ModemInfo &m = status.modeminfo;
  int state;
  if(isPassing){
    state = radio.begin(m.frequency, m.bw, m.sf, m.cr, m.sw, m.power, m.preambleLength, LNA_GAIN); 
  }else{
    state = radio.begin(m.frequency, m.bw, m.sf, m.cr, m.sw, m.power, m.preambleLength, m.gain);
  }
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
    String encoded;
    
    respLen = radio.getPacketLength();
    respFrame = new uint8_t[respLen];
    state = radio.readData(respFrame, respLen);
    status.lastPacketInfo.rssi = radio.getRSSI();
    status.lastPacketInfo.snr = radio.getSNR();
  
    status.lastPacketInfo.id += 1;
    EEPROM.write(ADDR_ID_EEPROM, status.lastPacketInfo.id);
    EEPROM.commit();
      
    unsigned long unixt = 0;
    getEpochTimeNow(unixt);
    mySat.findsat(unixt);
    status.lastPacketInfo.lat = mySat.satLat;
    status.lastPacketInfo.lon = mySat.satLon;
    if(state == RADIOLIB_ERR_NONE){
      encoded = base64::encode(respFrame, respLen); 
      Serial.println("sended");
    }else if(RADIOLIB_ERR_CRC_MISMATCH){
      encoded = "CRC_ERR";
    }else{
      encoded = "FAIL_CODE";
    }
    status.lastPacketInfo.packet = encoded;
    //sendPacketToDatabase();
    saveDataToSD(encoded);
    
    delete[] respFrame;
    enableInterrupt = true;
    radio.startReceive();
  }
} 
void saveDataToSD(String packet){
  StaticJsonDocument<300> JSONbuffer;
  JSONbuffer["satellite"] = status.modeminfo.satellite;
  JSONbuffer["date"] = getTimeDate();
  JSONbuffer["packet"] = packet;
  JSONbuffer["rssi"] = status.lastPacketInfo.rssi;
  JSONbuffer["snr"] = status.lastPacketInfo.snr;
  JSONbuffer["lat"] = status.lastPacketInfo.lat;
  JSONbuffer["lon"] = status.lastPacketInfo.lon;
  char JSONmessageBuffer[300]; // be careful the size of buffer
  serializeJson(JSONbuffer, JSONmessageBuffer);
  status.stateSD = appendFile(SD, "/LoRa.txt", JSONmessageBuffer);
}
bool configParamsLoRa(Status& param, SX1278& myRadio, String orderSat, bool isPassing){
  bool state = true;
  param.modeminfo.satellite = orderSat;
  // if-else to death: I don't know the best solution to clean this function. Sorry!
  if(orderSat == "Norbi"){
    state = initLoRa(param, paramsNorbi, myRadio, isPassing);
  }else if(orderSat == "FossaSat-2E11" || orderSat == "FossaSat-2E12"){
    state = initLoRa(param, paramsFossa, myRadio, isPassing);
  }else if(orderSat == "GaoFen-7"){
    state = initLoRa(param, paramsGaoFen7, myRadio, isPassing);
  }else if(orderSat == "GaoFen-19"){
    state = initLoRa(param, paramsGaoFen19, myRadio, isPassing);
  }else if(orderSat == "FEES"){
    state = initLoRa(param, paramsFEES, myRadio, isPassing);
  }else if(orderSat == "SATLLA-2B"){
    state = initLoRa(param, paramsSATLLA2B, myRadio, isPassing);
  }else{
    state = initLoRa(param, paramsSapling2, myRadio, isPassing);
  }
  return state;
}
bool initLoRa(Status& param, float* paramsSat, SX1278& myRadio, bool isPassing){
  param.modeminfo.frequency = paramsSat[0]; 
  param.modeminfo.bw = paramsSat[1];
  param.modeminfo.sf = paramsSat[2];
  param.modeminfo.cr = paramsSat[3];
  param.modeminfo.sw = paramsSat[4];
  param.modeminfo.preambleLength = paramsSat[5];
  param.stateLoRa = beginLoRa(myRadio, isPassing);
  if(!param.stateLoRa){
    return false;
  }
  return true;
}
bool initFirebase(FirebaseAuth& authFirebase, FirebaseConfig& configFirebase)
{
  Serial.println("in shit");
  configFirebase.api_key = "AIzaSyAbrEhpaVJVDAyvv8ZVH7f3A6DnYXKM2hY";
  configFirebase.database_url = "https://tinygs1-default-rtdb.asia-southeast1.firebasedatabase.app/";
  Serial.println("before shit");
  if (Firebase.signUp(&configFirebase, &authFirebase, "", "")){
    signupOK = true;
  }
  else{
    return false;
  }
  Serial.println("after shit");
  configFirebase.token_status_callback = tokenStatusCallback; 
  Firebase.begin(&configFirebase, &authFirebase);
  return true;
}
bool sendPacketToDatabase()
{
  if(Firebase.ready() && signupOK){
    json.set(SAT_PATH, status.modeminfo.satellite);
    json.set(EPOCH_PATH, getTimeDate());
    json.set(RSSI_PATH, String(status.lastPacketInfo.rssi));
    json.set(SNR_PATH, String(status.lastPacketInfo.snr));
    json.set(PACKET_PATH, String(status.lastPacketInfo.packet));
    json.set(LON_PATH, String(status.lastPacketInfo.lon));
    json.set(LAT_PATH, String(status.lastPacketInfo.lat));
    String databasePath = "/UsersData/packages";
    String parentPath = databasePath + "/" + String(status.lastPacketInfo.id);
    if (fbdo.errorReason().c_str() != "connection lost"){
      return (Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? true : false);
    }
    return false;
  }  
}
