#ifndef __TIMETOOLS_H__
#define __TIMETOOLS_H__

#include "defines.h"
#include <RTClib.h>
#include <WiFi.h>
#include <time.h>
#include <ArduinoJson.h>
#include <SDaccess.h>

extern RTC_DS3231 rtc;
extern bool forceTimeUpdate;
extern bool forceDateUpdate;
extern JsonDocument config;
extern JsonDocument locales;

bool timeSetup();
void adjustRTCTime();
void displayTime();
uint16_t hhmmToMinutes(const char * hhmm);
bool inInterval(const char *heure_test, const char *heure_debut, const char *heure_fin);

#endif
