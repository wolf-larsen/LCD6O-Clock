#ifndef __DEFINES__
#define __DEFINES__

#define DEBUG
#define DEBUGPROTO
//#define DEBUGBL

// -------- Uncomment one and only one of 2 next lines, depending on chosen FS por reading images to TFT
//# define FS_SPIFFS
# define FS_LITTLEFS
#include <LittleFS.h>
#include <FastLED.h>

#define LDR_PIN         4 // LDR input pin
#define BACKLIGHT_PIN   5 // backlight PWM pin
#define BUTTON1_PIN     9 // action button
#define CLOCKOUT_PIN    10 // pin receiving 1Hz clock

#define I2C_SDA_PIN         6
#define I2C_SCL_PIN         7

#define LEDS_DATA_PIN   0

#define COM_RX          3
#define COM_TX          2

#define BUZZER_PIN      1

#define DISPLAYMODE_WITH_SECONDS 1
#define DISPLAYMODE_WITH_WEATHER 2

#define NUM_LEDS 6

#define LED_LEFT_U 5
#define LED_LEFT_D 4
#define LED_MIDDLE_U 3
#define LED_MIDDLE_D 2
#define LED_RIGHT_U 1
#define LED_RIGHT_D 0

#define SHORT_PRESS 1
#define MEDIUM_PRESS 70
#define LONG_PRESS 140
#define VERYLONG_PRESS 300

#define WITH_AHT20

#define NBDAYS_FORECAST 2

//--------- dots
#define DOTS_START_LEVEL_MAX          30
#define DOTS_LEVEL_RED    230
#define DOTS_LEVEL_GREEN  50
#define DOTS_LEVEL_BLUE   240

#endif

