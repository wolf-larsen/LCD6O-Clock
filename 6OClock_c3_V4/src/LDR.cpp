#include <LDR.h>

void LDRSetup() {
  pinMode(LDR_PIN, INPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  digitalWrite(BACKLIGHT_PIN,128);
}

uint16_t LDRGetValue() {
  return analogRead(LDR_PIN);
}

void setBacklightValue(int blv) {
  if (backlightON) {
    analogWrite(BACKLIGHT_PIN, blv);
  }
  else {
    analogWrite(BACKLIGHT_PIN, 0);
  }
}

