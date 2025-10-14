#ifndef __WIFISETUP_H__
#define __WIFISETUP_H__

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>

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
void displayIP(IPAddress myIP);
//void downloadFile(const char * URL, fs::FS &fs, const char * fileName);

#endif
