#ifndef __LDR_H__
#define __LDR_H__

#include <defines.h>
#include <Arduino.h>
#define MIN_BACKLIGHT  1  // lowest possible backlight PWM value (0-255)

void LDRSetup();
uint16_t LDRGetValue();
void setBacklightValue(uint16_t blv);

#endif
