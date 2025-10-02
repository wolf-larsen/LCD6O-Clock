#include <Arduino.h>
#include <WiFiClientSecure.h>
#include "defines.h"
#include "serialCom.h"
#include "WiFiSecondarySetup.h"
#include "LDR.h"
#include "Weather.h"
#include "dots.h"

uint8_t displayMode = DISPLAYMODE_WITH_SECONDS;

extern bool backlightON;
int16_t previousLDRValue=0;

extern float intWeatherTemp;
extern float intWeatherHum;
float weather_GPSlat=0;
float weather_GPSlon=0;
char weather_timezone[64]="?";
long weather_IndoorWeatherDelaySec=0;
long weather_WeatherDelaySec=0;
long display_backlightLDRHysteresis=0;
long display_backlightLDRDivider=0;
long display_backlightValueMin=0;
long display_backlightValueMax=0;
long display_backlightOffLDRValue=0;
extern bool backlightON;

char wifi_SSID[64] = "?";
char wifi_password[64] = "?";
//char wifi_SSID[64] = "freebox_LVHOME";
//char wifi_password[64] = "5073171234";
//char wifi_SSID[64] = "SFR_E2FF";
//char wifi_password[64] = "1d5k5pk1jd3c8wp7ti5y";

JsonDocument weatherInfos;

bool alarmON = false;
int bipCount = 0;
bool isBipping = false;
bool buzzerState = false;
int bipsQty            =5;
int bipDuration        =80;
int pauseBetweenBips   =60;
int pauseBetweenSalves =600;
int salvesQty          =10;
long totalAlarmDuration =(pauseBetweenSalves+(pauseBetweenBips+bipDuration)*bipsQty)*salvesQty;
uint32_t delayAlarm = 0;

