#ifndef __DOTS_H__
#define __DOTS_H__

#include <defines.h>
#include <ArduinoJson.h>
//#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

extern uint8_t displayMode;

void ledDotsSetup();
void changeDotsLevel(uint8_t ambient, int8_t level);
void dotsOff();

#endif
