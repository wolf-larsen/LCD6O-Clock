#include "main.h"
#include "WiFiSetup.h"
#include "SDaccess.h"
#include "timeTools.h"
#include "hc138.h"
#include "display.h"
#include "Weather.h"
#include "configuration.h"
#include "serialCom.h"

void setup() {
  Serial.begin(115200);
  Serial.flush();
  delay(500);
#ifdef DEBUG
delay(2000);
#endif
  Serial.print("6 O'Clock V");
  Serial.println(VERSION);

#ifdef DEBUG
Serial.println("setup 10");
#endif
  serialBufferInit();

#ifdef DEBUG
Serial.println("setup 11");
#endif
  hc138Setup();

  Serial.print("Init. LittleFS: ");
  if( ! LittleFS.begin(true)){
    Serial.print("!! Error");
    while(true);
  }
  else { Serial.print("OK"); }
  Serial.print("\n");

#ifdef DEBUG
Serial.println("setup 20");
#endif
  getConfigdata();
#ifdef DEBUG
Serial.println("setup 21");
#endif
  getLocales();
//  config["wifi"]["password"]="1d5k5pk1jd3c8wp7ti5y";
  
#ifdef DEBUG
Serial.println("setup 30");
#endif
  Serial1.flush();
  uint32_t delayCheckComm = millis();
  int nbRetry=10;
  while ( ! CommChecked && nbRetry > 0) {
    getAndProcessSerialInput();
    executeOrdersReceived();
    if (millis()-delayCheckComm>1000) {
      delayCheckComm = millis();
      orderSend("COMMCHECK", "", true);
      nbRetry--;
    }
  }

#ifdef DEBUG
Serial.println("setup 40");
#endif
  displaySetup();

#ifdef DEBUG
Serial.println("setup 50");
#endif
  getAndProcessSerialInput();

#ifdef DEBUG
Serial.println("setup 60");
#endif
  WiFiSetup();

#ifdef DEBUG
Serial.println("setup 62");
#endif
  displayIP();

#ifdef DEBUG
Serial.println("setup 64");
#endif
  SDSetup();

#ifdef DEBUG
Serial.println("setup 66");
#endif
  getAndProcessSerialInput();
  
#ifdef DEBUG
Serial.println("setup 70");
#endif
  timeSetup();

#ifdef DEBUG
Serial.println("setup 80");
#endif
  for (int ol=0; ol<1000; ol++) {
//Serial.printf("ser %d \n",ol);
    getAndProcessSerialInput();
    executeOrdersReceived();
    delay(2);
  }

#ifdef DEBUG
Serial.println("setup 90");
#endif
  if ((int)config["display"]["displayMode"]==DISPLAYMODE_WITH_WEATHER) {
    clearTFT(TFT_FIRST,TFT_BLACK);
    clearTFT(TFT_LAST,TFT_BLACK);
  }

}

void loop() {

  uint32_t currentMillis = millis();
  long IndoorWeatherDelayMillis = (long)config["weather"]["IndoorWeatherDelaySec"]*1000;
  long WeatherDelayMillis = (long)config["weather"]["WeatherDelaySec"]*1000;
  static uint32_t delayGetInternaWeather = currentMillis;
  static uint32_t delayGetWeather = currentMillis;
  static uint32_t delayCheckComm = currentMillis;
  
  getAndProcessSerialInput();

//  static uint32_t delayExecOrders = currentMillis;
//  if (currentMillis-delayExecOrders>1) {
//    delayExecOrders = currentMillis;
    executeOrdersReceived();
//  }

  static uint32_t delayRefreshTime = currentMillis;
  if (currentMillis-delayRefreshTime>100 || forceTimeUpdate) {
    delayRefreshTime = currentMillis;
    displayTime();
  }

  if ( ! CommChecked) {
    if (currentMillis - delayCheckComm > 1000) {
      delayCheckComm = currentMillis;
//Serial.printf("Appel COMMCHECK::millis=%lu \n",millis());
      orderSend("COMMCHECK", "", true);
    }
  }
  else {
    static bool oneshotAterCommChecked = true;
    if (oneshotAterCommChecked) {
      oneshotAterCommChecked=false;
      orderSend("SETDISPLAYMODE", (long)config["display"]["displayMode"], false);
      forceTimeUpdate = true;
      forceDateUpdate = true;
      displayTime();
      if ((int)config["display"]["displayMode"]==DISPLAYMODE_WITH_WEATHER) {
        getInternalWeather();
        getWeather();
      }
    }
    if ((int)config["display"]["displayMode"]==DISPLAYMODE_WITH_WEATHER) {
      if (IndoorWeatherDelayMillis>0 && (currentMillis - delayGetInternaWeather > IndoorWeatherDelayMillis)) {
//Serial.printf("Appel GETINTGW millis normal::millis=%lu  dlygetint=%lu\n",millis(),delayGetInternaWeather);
        delayGetInternaWeather = currentMillis;
        getInternalWeather();
      }

      if (WeatherDelayMillis>0 && (currentMillis - delayGetWeather > WeatherDelayMillis)) {
//      if (currentMillis-delayGetWeather > 60000) {
//Serial.printf("Appel GETOUTW millis normal::millis=%lu  dlygetout=%lu\n",millis(),delayGetWeather);
        delayGetWeather = currentMillis;
        getWeather();
      }
    }
  }

  static uint32_t delayAdjustTime = currentMillis;
  long NTPAdjustDelaySec = (long)config["time"]["NTPAdjustDelaySec"];
//NTPAdjustDelaySec = 15;
  if (NTPAdjustDelaySec>0 && currentMillis-delayAdjustTime>NTPAdjustDelaySec*1000) {
//Serial.printf("AdjustRTCtime millis normal::miillis=%lu  dlygetout=%lu\n",millis(),delayAdjustTime);
    delayAdjustTime = currentMillis;
    adjustRTCTime();
  }

  static uint32_t delayUpdateFaces = currentMillis;
  if (currentMillis-delayUpdateFaces>1000) {
    delayUpdateFaces = currentMillis;
    if (facesUpdate) {
      updateFaces();
      facesUpdate=false;
    }
  }

}

