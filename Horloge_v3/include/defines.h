#ifndef __DEFINES__
#define __DEFINES__

#define VERSION "1.0 20250925"
#define DEBUG

// -------- Uncomment one and only one of 2 next lines, depending on chosen FS por reading images to TFT
//# define FS_SPIFFS
# define FS_LITTLEFS

#ifdef FS_LITTLEFS
#include <LittleFS.h>
#endif
#ifdef FS_SPIFFS
#include <SPIFFS.h>
#endif

#define WITH_AHT20

#define NBDAYS_FORECAST 2

#define SD_CS           5   // SPI CS, go to 74HC138 E1/
#define HC138_SD_CS     7   // 74HC138 output pin to SD module's CS
#define HC138_A0_PIN    25  // 74HC138 A0 adress select line
#define HC138_A1_PIN    26  // 74HC138 A1 adress select line
#define HC138_A2_PIN    27  // 74HC138 A2 adress select line

#define CS              5   // Fake SPI CS to sd card/
#define TFT_RST_FAKE    -1
#define TFT_DC          12

#define COM_RX          16
#define COM_TX          17

//#define SPI_CLK         18  //reserved, used
//#define SPI_MOSI        23  //reserved, used
//#define SPI_MISO        19  //reserved, used
//#define I2C_SDA_ESP32   21  //reserved, used
//#define I2C_SCL_ESP32   22  //reserved, used

//#define UART2_RX_ESP32  16  //reserved
//#define UART2_TX_ESP32  17  //reserved
//#define UART0_RX_ESP32  1   //reserved
//#define UART0_TX_ESP32  3   //reserved


//----libres
//  GPIO2
//  GPIO4
//  GPIO13
//  GPIO14
//  GPIO15
//  GPIO32
//  GPIO33
//  GPIO34  input only no internal pullup !
//  GPIO35  input only no internal pullup !
//  GPIO36  input only no internal pullup !
//  


#define TFT_FIRST 0
#define TFT_LAST  5

#define DISPLAYMODE_WITH_SECONDS 1
#define DISPLAYMODE_WITH_WEATHER 2

#define DISPLAY_FACE_MAX 2

#endif
