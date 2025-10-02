#ifndef __WIFISETUP_H__
#define __WIFISETUP_H__

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Wire.h>

//#define WIFISCAN

extern JsonDocument config;
extern bool forceTimeUpdate;
extern bool forceDateUpdate;
extern bool forceWeatherUpdate;

extern bool facesUpdate;
extern String facesSourcePath;
extern String facesDestinationPath;

extern bool STAMode;
extern bool APMode;

bool WiFiSetup();
bool WiFiIsOK();
void displayIP();
void downloadFile(const char * URL, fs::FS &fs, const char * fileName);

#endif
