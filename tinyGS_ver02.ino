#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "TLEFunction.h"
#include "predictSat.h"

#define SSID_WIFI                             "Thanh Tai"
#define PASSWORD_WIFI                         "123456789"

#define SERVER_TLE_TINYGS                     "https://api.tinygs.com/v1/tinygs_supported.txt"
#define SERVER_NTP                            "pool.ntp.org"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
HTTPClient http;


//String payload;
static char tleLine1[70];
static char tleLine2[70];

unsigned long epochNow = 1660138928;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFi.begin(SSID_WIFI, PASSWORD_WIFI);
  while (WiFi.status() != WL_CONNECTED);
  Serial.println("WiFi connected.");
  configTime(GMT_OFFSET_SECOND, DAYLIGHT_OFFSET_SECOND, SERVER_NTP);
  getEpochTimeNow(epochNow);
  //Serial.println(epochNow);
  //updateTleData(&http, payload, SERVER_TLE_TINYGS);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("Hello world");
  delay(1000);
}
