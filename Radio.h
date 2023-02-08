#include <Arduino.h>
#include <RadioLib.h>
#include <base64.h>
#include "Status.h"

extern Status status;

void beginLoRa(SX1278& radio);
void setFlag(void);
void listenRadio(SX1278& radio);
