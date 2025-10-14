#include <Arduino.h>
#include <ArduinoJson.h>
#include "defines.h"
#include "eventScheduler.h"
#include "configuration.h"

extern bool CommChecked;

extern uint16_t currentYear;
extern uint16_t currentMonth;
extern uint16_t currentDay;

extern JsonDocument config;
extern JsonDocument locales;

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
extern float daily_wind_speed_10m_max[NBDAYS_FORECAST+1];

extern int OutdoorWeatherParametersExpected;

extern IPAddress ip;

extern EventScheduler scheduler;

void serialBufferInit();
void getAndProcessSerialInput();
bool orderReceived(char * command);
bool responseReceived(char * command);
uint32_t orderSend(const char * command, const char * text, bool responseExpected);
uint32_t orderSend(const char * command, long valeur, bool responseExpected);
bool responseSend(uint32_t id, const char * text);
bool responseSend(uint32_t id, long lval);
bool responseSend(uint32_t id, double fval);
void executeOrdersReceived();
void freeMessage(uint16_t idx);
