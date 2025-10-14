#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include "defines.h"
#include <ArduinoJson.h>
#include <FS.h>
#include <SD.h>

#ifdef FS_LITTLEFS
#include <LittleFS.h>
#endif
#ifdef FS_SPIFFS
#include <SPIFFS.h>
#endif

extern JsonDocument config;
extern JsonDocument locales;

void getConfigdata();
void saveConfigdata();
void getLocales();

#endif
