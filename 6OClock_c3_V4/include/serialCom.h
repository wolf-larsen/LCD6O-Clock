#ifndef __SERIALCOM_H__
#define __SERIALCOM_H__

#include <Arduino.h>
#include "defines.h"
#include <ArduinoJson.h>
#include "LDR.h"
#include "dots.h"
#include "eventScheduler.h"

extern EventScheduler scheduler;

extern uint8_t displayMode;

extern bool internetOK;

//extern bool sleeping;

extern uint16_t currentYear;
extern uint16_t currentMonth;
extern uint16_t currentDay;

extern float intWeatherTemp;
extern float intWeatherHum;

extern char wifi_SSID[64];
extern char wifi_password[64];
extern float weather_GPSlat;
extern float weather_GPSlon;
extern char weather_timezone[64];
extern long weather_IndoorWeatherDelaySec;
extern long weather_WeatherDelaySec;
extern long display_backlightLDRHysteresis;
extern long display_backlightLDRDivider;
extern long display_backlightValueMin;
extern long display_backlightValueMax;
extern long display_backlightOffLDRValue;

extern int ledsLevel;
extern int ticks;
//extern CRGB leds[NUM_LEDS];
extern CRGB leds;

extern JsonDocument weatherInfos;

extern bool backlightON;
extern int16_t previousLDRValue;

extern bool alarmON;
//extern int alarmNbSalves;
//extern bool isBipping;

extern bool forceTimeUpdate;
extern bool forceDateUpdate;
extern bool forceWeatherUpdate;

extern bool blinkEnable;

void tryToConnect();
void afterCommChecked();
void serialBufferInit();
void getAndProcessSerialInput();
bool orderReceived(char * command);
bool responseReceived(char * command);
uint32_t orderSend(const char * command, const char * text, bool responseExpected);
uint32_t orderSend(const char * command, long value, bool responseExpected);
bool responseSend(uint32_t orderid, const char * responsetext);
bool responseSendLong(uint32_t id, long lval);
bool responseSendDouble(uint32_t id, float fval);
void executeOrdersReceived();
void freeMessage(uint16_t idx);
bool isCommChecked();

void alarmBeep();
void backlightOff();

#endif
