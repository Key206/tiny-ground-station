#include <RadioLib.h>
#include <base64.h>
#include <ArduinoJson.h>
#include "Status.h"
#include "SDcard.h"

#define BEGIN_LORA_OK           1
#define BEGIN_LORA_FAULT        0
extern Status status;

bool beginLoRa(SX1278& radio);
void setFlag(void);
void listenRadio(SX1278& radio);
void saveDataToSD(String packet);
