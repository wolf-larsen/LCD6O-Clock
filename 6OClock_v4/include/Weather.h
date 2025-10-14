#ifndef __WEATHER_H__
#define __WEATHER_H__

#include "defines.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#ifdef WITH_AHT20
#define AHT20_ADDRESS 0x38
#endif

extern JsonDocument config;
extern JsonDocument weatherInfos;
extern uint16_t yPosWeather[];

extern float currentIndoorTemp;
extern float currentIndoorRH;
extern float current_temperature_2m;
extern float current_precipitation;
extern char current_units_precipitation[10];
extern char current_units_relative_humidity_2m[10];
extern float current_relative_humidity_2m;
extern long current_wind_direction_10m;
extern float current_wind_speed_10m;
extern char current_units_wind_speed_10m[10];
extern float daily_temperature_2m_min[NBDAYS_FORECAST+1];
extern float daily_temperature_2m_max[NBDAYS_FORECAST+1];
extern float daily_precipitation_sum[NBDAYS_FORECAST+1];
extern char daily_units_precipitation_sum[10];
extern float daily_uv_index_max[NBDAYS_FORECAST+1];
extern long daily_wind_direction_10m_dominant[NBDAYS_FORECAST+1];

void weatherSetup();
void getWeather();
void displayWeather();
#ifdef WITH_AHT20
void getInternalWeather();
void displayInternalWeather();
#endif

String httpGETRequest(const char* serverName);

#endif
