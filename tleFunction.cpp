#include "TLEFunction.h"


void updateTleData(String& payload, String serverName)
{
  uint8_t httpResponseCode = 0;
  HTTPClient http;
  http.begin(serverName);
  httpResponseCode = http.GET(); 
  if(httpResponseCode > 0){
    payload = http.getString();
  }
  else{
    Serial.println(httpResponseCode);
    Serial.println("Fail to get TLE!!!");
  }
  http.end();
}
bool checkNameSat(String nameSat, String payload, uint16_t& posStartTLEsLine1){
  String tempName;
  uint16_t i, j;
  uint16_t lengthOfPayload = payload.length();
  for(i = 0; i < lengthOfPayload; ++i){
    for(j = i; j < i + 50; ++j){
      if(payload[j] == '\n'){
        tempName = payload.substring(i,j);
        if(strcmp(tempName.c_str(), nameSat.c_str()) == 0){
          posStartTLEsLine1 = j + 1;
          return true;
        }
        else{
          break;
        }  
      }
    }
    i = j + STEP_JUMP_TLE;
  }
  return false;
}
bool getTLE(String nameSat, char* tle_line1, char* tle_line2, String payload){
  uint16_t posStartTLEsLine1 = 0;
  uint8_t count = 0;
  uint16_t i;
  if(checkNameSat(nameSat, payload, posStartTLEsLine1)){
    for(i = posStartTLEsLine1; i < posStartTLEsLine1 + 69; ++i){
      if(payload[i] != '\n'){
        tle_line1[count] = payload[i];
        tle_line2[count] = payload[i + 70];
        ++count;
      }
      else{
        break;
      }
    }
    tle_line1[i] = '\0';
    tle_line2[i] = '\0';
    return true;
  }  
  else{
    return false;
  }   
}
uint8_t getAmountOfSat(String payload)
{
  uint8_t amountOfSat = 0;
  uint16_t lengthOfPayload = payload.length();
  for(uint16_t index = 0; index < lengthOfPayload; ++index){
    if(payload[index] == '\n'){   
      index = index + STEP_JUMP_TLE;
      amountOfSat++;
    }
  }
  return amountOfSat;
}
void getAllSatName(String* arraySatNames, String payload)
{
  uint8_t element = 0;
  uint16_t indexStart, indexStop;
  uint8_t n = getAmountOfSat(payload);
  for(indexStart = 0; element < n; ++indexStart){
    indexStop = indexStart;
    while(payload[indexStop] != '\n'){
      ++indexStop;
    }
    arraySatNames[element++] = payload.substring(indexStart,indexStop);
    indexStart = indexStop + STEP_JUMP_TLE;
  }
}
