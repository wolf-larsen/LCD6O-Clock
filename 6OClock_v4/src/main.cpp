#include "main.h"
#include "WiFiSetup.h"
#include "SDaccess.h"
#include "timeTools.h"
#include "hc138.h"
#include "display.h"
#include "Weather.h"
#include "configuration.h"
#include "serialCom.h"
#include "radar.h"
#include "eventScheduler.h"

//  config["wifi"]["password"]="1d5k5pk1jd3c8wp7ti5y";

EventScheduler scheduler;

//-------------------------------
void humanDetection() {
  if (radarDetection() == 0) {
    if (humanPresence) {
      orderSend("BL","OFF",false);
    }
    humanPresence = false;
  }
  else if ( ! sleeping) {
    if ( ! humanPresence) {
      orderSend("BL","ON",false);
      humanPresence = true;
    }
  }
}

//-------------------------------
void commCheck() {
  if ( ! CommChecked) {
    orderSend("COMMCHECK", "", true);
  }
}

//-----------------------------------------------------------
void checkInternalWeather() {
  if (CommChecked) {
    if ((int)config["display"]["displayMode"]==DISPLAYMODE_WITH_WEATHER) {
      getInternalWeather();
    }
  }
}

//-----------------------------------------------------------
void checkWeather() {
#ifdef DEBUG
Serial.println("checkweather ?");
#endif
  if (CommChecked) {
    if ((int)config["display"]["displayMode"]==DISPLAYMODE_WITH_WEATHER) {
      getWeather();
    }
  }
}

//-----------------------------------------------------------
void checkfacesUpdate() {
  if (facesUpdate) {
    updateFaces();
    facesUpdate=false;
  }
}


//-----------------------------------------------------------
void i2cScan() {
  Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.begin();
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
}


void setup() {
  Serial.begin(115200);
  Serial.flush();
  delay(500);
#ifdef DEBUG
delay(2000);
#endif
  Serial.print("6 O'Clock V");
  Serial.println(VERSION);

//  i2cScan();

#ifdef DEBUG
Serial.println("setup 10");
#endif
  serialBufferInit();

#ifdef DEBUG
Serial.println("setup 12");
#endif
  radarSetup();

#ifdef DEBUG
Serial.println("setup 15");
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
  
#ifdef DEBUG
Serial.println("setup 30");
#endif
  Serial1.flush();
  uint32_t delayCheckComm = millis();
  int nbRetry=5;
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
Serial.println("setup 66");
#endif
  SDSetup();

#ifdef DEBUG
Serial.println("setup 68");
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

  scheduler.addEvent(displayTime, 100);
  scheduler.addEvent(humanDetection, 1000);
  scheduler.addEvent(commCheck, 1100);
  if ((long)config["weather"]["IndoorWeatherDelaySec"] > 0) {
    scheduler.addEvent(checkInternalWeather, (long)config["weather"]["IndoorWeatherDelaySec"]*1000);
  }
  if ((long)config["weather"]["WeatherDelayMillis"] > 0) {
    scheduler.addEvent(checkWeather, (long)config["weather"]["WeatherDelayMillis"]*1000);
  }
  scheduler.addOneShotEvent(checkInternalWeather, 5000);
  scheduler.addOneShotEvent(checkWeather, 5500);
  if ((long)config["time"]["NTPAdjustDelaySec"] > 0) {
    scheduler.addEvent(adjustRTCTime, (long)config["time"]["NTPAdjustDelaySec"]*1000);
  }
  scheduler.addEvent(checkfacesUpdate, 1300);

}

void loop() {
  
  getAndProcessSerialInput();
  executeOrdersReceived();

  scheduler.update();

  server.handleClient();

}

