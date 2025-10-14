#ifndef __WIFISECNDSETUP_H__
#define __WIFISECNDSETUP_H__

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
//#include <ArduinoJson.h>
#include <Wire.h>

extern bool forceWeatherUpdate;

//extern bool facesUpdate;
//extern String facesSourcePath;
//extern String facesDestinationPath;

extern char wifi_SSID[64];
extern char wifi_password[64];

bool WiFiSetup();
bool WiFiIsOK();

#endif
