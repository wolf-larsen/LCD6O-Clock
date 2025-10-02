#ifndef __WEATHER_H__
#define __WEATHER_H__

#include "defines.h"
#include <Wire.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#include <WiFiSecondarySetup.h>
#ifdef WITH_AHT20
#define AHT20_ADDRESS 0x38
#endif

extern JsonDocument weatherInfos;
extern uint8_t displayMode;
extern float weather_GPSlat;
extern float weather_GPSlon;
extern char weather_timezone[64];
extern WiFiClientSecure client;
extern bool internetOK;

void weatherSetup();
void getWeather();
#ifdef WITH_AHT20
void getInternalWeather();
#endif
String httpGETRequest(const char* serverName);
//void requestCB(void *optParm, AsyncHTTPRequest *request, int readyState);
//bool isWeatherOK();

#endif
