#include <dots.h>

CRGB leds[NUM_LEDS];

bool blinkEnable = true;
uint8_t ledNumbersModeSeconds[4]={LED_RIGHT_D,LED_RIGHT_U,LED_LEFT_D,LED_LEFT_U};
uint8_t ledNumbersModeWeather[2]={LED_MIDDLE_D,LED_MIDDLE_U};

void ledDotsSetup() {
  FastLED.addLeds<NEOPIXEL, LEDS_DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  // verif chenillard
  for (int l=0;l<NUM_LEDS;l++) {
    leds[l] = CRGB::DarkGreen;
    FastLED.show();
    delay(50);
  }
  for (int l=0;l<NUM_LEDS;l++) {
    leds[l] = CRGB::Black;
    FastLED.show();
    delay(20);
  }
  // fin verif

}

void changeDotsLevel(uint8_t backLight, int8_t level) {
  if (! blinkEnable) {
    return;
  }
  uint8_t lvl = level * backLight / 256;

  uint8_t ledsLvlR = lvl*DOTS_LEVEL_RED/255;
  uint8_t ledsLvlG = lvl*DOTS_LEVEL_GREEN/255;
  uint8_t ledsLvlB = lvl*DOTS_LEVEL_BLUE/255;
  if (displayMode==DISPLAYMODE_WITH_SECONDS) {
//    for (uint8_t li=0; li < sizeof(ledNumbersModeWeather)/sizeof(uint8_t); li++) {
//      leds[ledNumbersModeWeather[li]] = CRGB::Black;
//    }
    for (uint8_t li=0; li < sizeof(ledNumbersModeSeconds)/sizeof(uint8_t); li++) {
      leds[ledNumbersModeSeconds[li]] = CRGB(ledsLvlR, ledsLvlG, ledsLvlB);
    }
  }
  else if (displayMode==DISPLAYMODE_WITH_WEATHER) {
//    for (uint8_t li=0; li < sizeof(ledNumbersModeSeconds)/sizeof(uint8_t); li++) {
//      leds[ledNumbersModeSeconds[li]] = CRGB::Black;
//    }
    for (uint8_t li=0; li < sizeof(ledNumbersModeWeather)/sizeof(uint8_t); li++) {
      leds[ledNumbersModeWeather[li]] = CRGB(ledsLvlR, ledsLvlG, ledsLvlB);
    }
  }
  FastLED.show();
}

void dotsOff() {
  for (uint8_t li=0; li < sizeof(ledNumbersModeWeather)/sizeof(uint8_t); li++) {
    leds[ledNumbersModeWeather[li]] = CRGB::Black;
  }
  for (uint8_t li=0; li < sizeof(ledNumbersModeSeconds)/sizeof(uint8_t); li++) {
    leds[ledNumbersModeSeconds[li]] = CRGB::Black;
  }
  FastLED.show();
}


