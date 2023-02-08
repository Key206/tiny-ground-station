#include "Radio.h"

volatile bool receivedFlag = false;
volatile bool enableInterrupt = true;

void setFlag(void) {
  if(!enableInterrupt) {
    return;
  }
  receivedFlag = true;
}

void beginLoRa(SX1278& radio)
{
  ModemInfo &m = status.modeminfo;
  int state = radio.begin(m.frequency, m.bw, m.sf, m.cr, m.sw, m.power, m.preambleLength, m.gain);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    return;
  }
  radio.setCRC(m.crc);
  radio.forceLDRO(m.fldro);
  if (radio.setCurrentLimit(120) == RADIOLIB_ERR_INVALID_CURRENT_LIMIT) {
    Serial.println(F("Selected current limit is invalid for this module!"));
    //return;
  }
  radio.setDio0Action(setFlag);

  // start listening for LoRa packets
  Serial.print(F("[SX1278] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    return;
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
    status.lastPacketInfo.frequencyerror = radio.getFrequencyError();
   
    if (state == RADIOLIB_ERR_NONE) {
      // packet was successfully received
      Serial.println(F("[SX1278] Received packet!"));

      // print data of the packet
      Serial.print(F("[SX1278] Data:\t\t"));
      for(int i = 0; i < respLen; i++){
        Serial.print(*(respFrame+i));
      }
      Serial.println();

      // print RSSI (Received Signal Strength Indicator)
      Serial.print(F("[SX1278] RSSI:\t\t"));
      Serial.print(status.lastPacketInfo.rssi);
      Serial.println(F(" dBm"));

      // print SNR (Signal-to-Noise Ratio)
      Serial.print(F("[SX1278] SNR:\t\t"));
      Serial.print(status.lastPacketInfo.snr);
      Serial.println(F(" dB"));

      // print frequency error
      Serial.print(F("[SX1278] Frequency error:\t"));
      Serial.print(status.lastPacketInfo.frequencyerror);
      Serial.println(F(" Hz"));

    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      // packet was received, but is malformed
      Serial.println(F("[SX1278] CRC error!"));

    } else {
      // some other error occurred
      Serial.print(F("[SX1278] Failed, code "));
      Serial.println(state);

    }
    radio.startReceive();
    enableInterrupt = true;
    String encoded = base64::encode(respFrame, respLen);
  }
} 
