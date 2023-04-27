#include "Radio.h"
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;
FirebaseJson json;

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
    
    status.lastPacketInfo.id += 1;
    EEPROM.write(ADDR_ID_EEPROM, status.lastPacketInfo.id);
    EEPROM.commit();
    
    unsigned long unixt = 0;
    getEpochTimeNow(unixt);
    mySat.findsat(unixt);
    status.lastPacketInfo.lat = mySat.satLat;
    status.lastPacketInfo.lon = mySat.satLon;
    if(state == RADIOLIB_ERR_NONE){
      Serial.println("sended");
      String encoded = base64::encode(respFrame, respLen);
      status.lastPacketInfo.packet = encoded;
      sendPacketToDatabase();
      saveDataToSD(encoded);
    }
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
  char JSONmessageBuffer[300]; // be careful the size of buffer
  serializeJson(JSONbuffer, JSONmessageBuffer);
  status.stateSD = appendFile(SD, "/LoRa.txt", JSONmessageBuffer);
}
void configParamsLoRa(Status& param, SX1278& myRadio, String orderSat){
  if(orderSat[0] == 'G'){
    param.modeminfo.satellite = orderSat;
    initLoRa(param, paramsGaoFen,myRadio);
  }else if(orderSat[0] == 'F'){
    param.modeminfo.satellite = orderSat;
    initLoRa(param, paramsFossa,myRadio);
  }else{
    param.modeminfo.satellite = "Norbi"; 
    initLoRa(param, paramsNorbi,myRadio);
  }
}
void initLoRa(Status& param, float* arr, SX1278& myRadio){
  param.modeminfo.frequency = arr[0]; 
  param.modeminfo.bw = arr[1];
  param.modeminfo.sf = arr[2];
  param.modeminfo.cr = arr[3];
  param.stateLoRa = beginLoRa(myRadio);
  if(!param.stateLoRa){
    Serial.println("Fail init LoRa");
  }
}
bool initFirebase()
{
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")){
    signupOK = true;
  }
  else{
    //Serial.printf("%s\n", config.signer.signupError.message.c_str());
    return false;
  }
  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; 
  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);
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
  }  
}
