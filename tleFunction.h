#include <Arduino.h>
#include <HTTPClient.h>

#define STEP_JUMP_TLE          140
#define NUM_OF_SAT             13
#define OFFSET_TIME_PREDICT    600 // in second

void updateTleData(HTTPClient* http, String& payload, String serverName);
bool checkNameSat(String nameSat, String payload, uint16_t& posStartTLEsLine1);
bool getTLE(String nameSat, char* tle_line1, char* tle_line2, String payload);
uint8_t getAmountOfSat(String payload);
void getAllSatName(String* arraySatNames, String payload);
