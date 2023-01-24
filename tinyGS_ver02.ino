#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "tleFunction.h"
#include "predictSat.h"
#include "controlStepper.h"

#define SSID_WIFI                             "Thanh Tai"
#define PASSWORD_WIFI                         "123456789"

#define SERVER_TLE_TINYGS                     "https://api.tinygs.com/v1/tinygs_supported.txt"
#define SERVER_NTP                            "pool.ntp.org"

#define uS_TO_S_FACTOR                        1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_PREPARE_AFTER_WAKEUP             20          /* in second */

Sgp4 mySat;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
HTTPClient http;

AccelStepper motorAz(MOTOR_INTERFACE_TYPE, STEP_AZ, DIR_AZ);
AccelStepper motorEl(MOTOR_INTERFACE_TYPE, STEP_EL, DIR_EL);

String payload;
unsigned long epochNow = 1660138928;
String* upcomingSatList;

void setup() {
  Serial.begin(9600);
  WiFi.begin(SSID_WIFI, PASSWORD_WIFI);
  while (WiFi.status() != WL_CONNECTED);
  Serial.println("WiFi connected.");
  
  configTime(GMT_OFFSET_SECOND, DAYLIGHT_OFFSET_SECOND, SERVER_NTP);
  getEpochTimeNow(epochNow);
  Serial.println(epochNow);
  
  updateTleData(&http, payload, SERVER_TLE_TINYGS);
  mySat.site(10.954,106.852,18);
  
  upcomingSatList = new String[NUM_OF_SAT];
  getAllSatName(upcomingSatList, payload);
  createUpcomingList(upcomingSatList, mySat, payload, epochNow);
}

void loop() {
  Serial.println("Hello world");
  delay(1000);
}
void goToSleep(unsigned int timeToSleep)
{
  esp_sleep_enable_timer_wakeup(timeToSleep * uS_TO_S_FACTOR);
  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start();
}
unsigned int calculateSleepTime(unsigned long Now, unsigned long Start)
{
  return (unsigned int)(Start - Now - TIME_PREPARE_AFTER_WAKEUP);
}
