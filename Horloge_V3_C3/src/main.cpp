#include <main.h>

//bool startNewSecond=false;
int ledsLevel = 0;
int ticks=255;
bool internetOK = false;

WiFiClientSecure client;

//-----------------------------------------------------------
// interruptions de comptage des tours crémaillère pompe
//-----------------------------------------------------------
ICACHE_RAM_ATTR void signal32kHz() {
  if (ledsLevel > 0 && ticks % 350 == 0) {
    ledsLevel--;
  }
  ticks--;
}

void bip() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(50);
  digitalWrite(BUZZER_PIN, LOW);
}

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

  pinMode(BUZZER_PIN, OUTPUT);
  bip();

  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  Serial.print("Init. LittleFS: ");
  if( ! LittleFS.begin(true)){
    Serial.print("!! Error");
    while(true);
  }
  
  else { Serial.print("OK"); }
  Serial.print("\n");

  ledDotsSetup();
  
  serialBufferInit();

  LDRSetup();

  digitalWrite(BUZZER_PIN, LOW);

//  i2cScan();
  getInternalWeather();

  attachInterrupt(CLOCKOUT_PIN, signal32kHz, FALLING);
  
  bip(); delay(100); bip();
  
}

void loop() {


  getAndProcessSerialInput();

  uint32_t currentMillis = millis();

  static uint32_t delayExecOrder = currentMillis;
  if (currentMillis-delayExecOrder>1) {
    delayExecOrder = currentMillis;
    executeOrdersReceived();
  }

  static bool runOnce=true;
  if (runOnce) {
    // put here what you want to be run only one time after setup
    runOnce = false;
  }

  static int16_t bl = 100;
  static uint32_t delayGetLight = currentMillis;
  if (backlightON && currentMillis-delayGetLight > 1234) {
    delayGetLight = currentMillis;
    uint16_t ldr = LDRGetValue();
//Serial.printf("ldr:%d  prevldr:%d  hyst:%d  offv:%d  divider:%d\n",ldr,previousLDRValue,display_backlightLDRHysteresis,display_backlightOffLDRValue,display_backlightLDRDivider);
//char buf[30];
//sprintf(buf,"10|20|%d",ldr);
//orderSend("SHOWVALUE",buf,false);
    if (display_backlightLDRHysteresis>0 && (abs(ldr-previousLDRValue)>display_backlightLDRHysteresis || bl < 0)) {
      previousLDRValue = ldr;
      if (ldr < display_backlightOffLDRValue) {
         ldr=0;
      }
      if (display_backlightLDRDivider > 0) {
        bl = min(int(ldr/display_backlightLDRDivider),255);

      }
      if (bl < display_backlightValueMin) bl = display_backlightValueMin;
      if (bl > display_backlightValueMax) bl = display_backlightValueMax;
//Serial.printf("bl:%d \n",bl);
//sprintf(buf,"10|40|%d",bl);
//orderSend("SHOWVALUE",buf,false);
      setBacklightValue(bl);
    }
  }

  static uint32_t delayLeds = currentMillis;
  static int previousLedLevel = 0;
  if (backlightON && currentMillis-delayLeds>1) {
    delayLeds = currentMillis;
//Serial.printf("ledlevel=%d ticks=%d tops=%d\n",ledsLevel,ticks,tops);
    if (ledsLevel > 0 && ledsLevel != previousLedLevel) {
//    if (ledsLevel > 0) {
      previousLedLevel = ledsLevel;
//      ledsLevel--;
      changeDotsLevel(bl, ledsLevel);
    }
  }


  if ( ! isCommChecked()) {
    //------- si les comm serie ne sont pas établies, revérifier
    static uint32_t delayCheckComm = currentMillis;
    if (currentMillis-delayCheckComm>1333) {
//Serial.printf("main::commchecked:%d\n",isCommChecked());
      delayCheckComm = currentMillis;
      orderSend("COMMCHECK", "", true);
    }
  }
  else {
    static uint32_t delayInit = currentMillis;
    static int8_t Attempts = 3;
    if (millis()-delayInit>1000) {
      delayInit = currentMillis;
//Serial.printf("SSID=%s\n",wifi_SSID);
      if ( ! WiFiIsOK()) {
        if (wifi_SSID[0]!='?' && wifi_password[0]!='?') {
         if (Attempts>0) {
//orderSend("SHOWVALUE","0|80|WiFiSetup",false);
            WiFiSetup();
            Attempts--;
//orderSend("SHOWVALUE","0|80|Fin/WiFiSetup",false);
            if (WiFiIsOK()) {
              getWeather();
            }
          }
        }
        else {
          orderSend("GETCONFIG", "WIFI.SSID", true);
          orderSend("GETCONFIG", "WIFI.PASSWORD", true);
          orderSend("GETCONFIG", "WEATHER.GPSLAT", true);
          orderSend("GETCONFIG", "WEATHER.GPSLON", true);
          orderSend("GETCONFIG", "WEATHER.TIMEZONE", true);
          orderSend("GETCONFIG", "BL.LDRHYST", true);
          orderSend("GETCONFIG", "BL.LDRDIVIDER", true);
          orderSend("GETCONFIG", "BL.MIN", true);
          orderSend("GETCONFIG", "BL.MAX", true);
          orderSend("GETCONFIG", "BL.OFFLDRV", true);
          orderSend("GETDISPLAYMODE", "", true);
        }
      }
    }
  }

  static uint32_t delayGetIndoorWeather = currentMillis;
//  if (weather_IndoorWeatherDelaySec>0 && (millis()-delayGetIndoorWeather)>weather_IndoorWeatherDelaySec*1000) {
  if (currentMillis-delayGetIndoorWeather > 10000) {
    if (displayMode==DISPLAYMODE_WITH_WEATHER && ledsLevel == 0) {
      delayGetIndoorWeather = currentMillis;
      getInternalWeather();
    }
  }

  static uint32_t delayGetWeather = currentMillis;
//  if (weather_WeatherDelaySec>0 && (millis()-delayGetWeather)>weather_WeatherDelaySec*1000) {
  if (currentMillis-delayGetWeather > 600000) {
    if (displayMode==DISPLAYMODE_WITH_WEATHER && internetOK && ledsLevel == 0) {
      delayGetWeather = currentMillis;
      getWeather();
    }
  }
  
  static uint32_t delayButtonTest = 0;
  static uint16_t pressedDuration = 0;
  if (currentMillis-delayButtonTest>10) {
    delayButtonTest = currentMillis;
    if (digitalRead(BUTTON1_PIN)==HIGH && pressedDuration>0) {
      if (alarmON) {
        alarmON=false;
        digitalWrite(BUZZER_PIN, LOW);
      }
      else if (backlightON==false) {
          backlightON=true;
          previousLDRValue=-1;
//          setBacklightValue(1);
      }
      else if (pressedDuration >= VERYLONG_PRESS) {
        // very long press
        orderSend("RESET","", false);
        delay(100);
        ESP.restart();
      }
      else if (pressedDuration >= LONG_PRESS) {
        // long press
        orderSend("DISPLAYIP","", false);
      }
      else if (pressedDuration >= MEDIUM_PRESS) {
        // long press
        orderSend("CHANGEFACE","", false);
      }
      else if (pressedDuration >= SHORT_PRESS) {
        // short press
        displayMode = 3-displayMode;
        orderSend("SETDISPLAYMODE",displayMode, false);
        dotsOff();
      }
      pressedDuration = 0;
    }
    else if (digitalRead(BUTTON1_PIN)==LOW){
      //--- button pressed
      pressedDuration++;
      if (pressedDuration==SHORT_PRESS) {
        bip();
      }
      if (pressedDuration==MEDIUM_PRESS) {
        bip(); delay(50); bip();
      }
      if (pressedDuration==LONG_PRESS) {
        bip(); delay(50); bip(); delay(50); bip();
      }
      if (pressedDuration==VERYLONG_PRESS) {
        bip(); delay(50); bip(); delay(50); bip(); delay(50); bip();
      }
    }
  }

  //  Alarme
  static uint32_t delayBip = currentMillis;
  if (alarmON) {
    if (currentMillis-delayAlarm >= totalAlarmDuration) {
      alarmON = false;
      digitalWrite(BUZZER_PIN, LOW);
    }
    else if (isBipping) {
      if (buzzerState && currentMillis-delayBip >= bipDuration) {
        digitalWrite(BUZZER_PIN, LOW);
        buzzerState = false;
        delayBip = currentMillis;
      } else if (!buzzerState && currentMillis-delayBip >= pauseBetweenBips) {
        bipCount++;
        if (bipCount <= bipsQty) {
          digitalWrite(BUZZER_PIN, HIGH);
          buzzerState = true;
          delayBip = currentMillis;
        } else {
          isBipping = false;
          bipCount = 0;
          delayBip = currentMillis;
        }
      }
    } else {
      if (currentMillis-delayBip >= pauseBetweenSalves) {
        isBipping = true;
        buzzerState = true;
        digitalWrite(BUZZER_PIN, HIGH);
        bipCount = 1;
        delayBip = currentMillis;
      }
    }
  }

}

//