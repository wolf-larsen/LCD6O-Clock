#include <Arduino.h>
#include <WiFiClientSecure.h>
#include "defines.h"
#include "serialCom.h"
#include "WiFiSecondarySetup.h"
#include "LDR.h"
#include "Weather.h"
#include "dots.h"
#include "eventScheduler.h"

uint8_t displayMode = DISPLAYMODE_WITH_SECONDS;

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

//extern bool CommChecked;
//bool CommChecked = false;

char wifi_SSID[64] = "?";
char wifi_password[64] = "?";

JsonDocument weatherInfos;

bool alarmON = false;
bool isBipping = false;

void bip();
void backlightOff();
void commCheck();
void checkLeds();
void checkLight();
void alarmBeep();
void checkButton();
void checkIndoorWeather();
void checkWeather();
