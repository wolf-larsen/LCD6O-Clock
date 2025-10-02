#ifndef __MAIN_H__
#define __MAIN_H__

#include <Arduino.h>
#include <ArduinoJson.h>
#include <defines.h>

bool forceTimeUpdate = false;
bool forceDateUpdate = false;
bool forceWeatherUpdate = false;

bool CommChecked=false;

JsonDocument config;
JsonDocument locales;

float currentIndoorTemp=0.0;
float currentIndoorRH=0.0;

float current_temperature_2m;
float current_precipitation;
char current_units_precipitation[10];
char current_units_relative_humidity_2m[10];
float current_relative_humidity_2m;
long current_wind_direction_10m;
float current_wind_speed_10m;
char current_units_wind_speed_10m[10];
float daily_temperature_2m_min[NBDAYS_FORECAST+1];
float daily_temperature_2m_max[NBDAYS_FORECAST+1];
float daily_precipitation_sum[NBDAYS_FORECAST+1];
char daily_units_precipitation_sum[10];
float daily_uv_index_max[NBDAYS_FORECAST+1];
long daily_wind_direction_10m_dominant[NBDAYS_FORECAST+1];
float daily_wind_speed_10m_max[NBDAYS_FORECAST+1];

uint8_t displayFaceIndex=0;
bool facesUpdate=false;
uint8_t facesNum=0;
String facesSourcePath;
String facesDestinationPath;

#endif
