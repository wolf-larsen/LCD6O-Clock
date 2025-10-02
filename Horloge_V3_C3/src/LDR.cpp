#include <LDR.h>

bool backlightON = true;

void LDRSetup() {
  pinMode(LDR_PIN, INPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN,128);
}

uint16_t LDRGetValue() {
  return analogRead(LDR_PIN);
}

void setBacklightValue(uint16_t blv) {
  analogWrite(BACKLIGHT_PIN, blv);
}

