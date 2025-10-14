#include "main.h"

EventScheduler scheduler;

int ledsLevel = 0;
int ticks=255;
bool internetOK = false;
//bool sleeping = false;
int alarmNbSalves = 0;
uint16_t pressedDuration = 0;
bool backlightON = true;
int bl = 255;
int previousLedLevel = 0;

//-----------------------------------------------------------
void bip() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(10);
  digitalWrite(BUZZER_PIN, LOW);
}

//----------------------------------------
void commCheck() {
  if ( ! isCommChecked()) {
    orderSend("COMMCHECK", "", true);
  }
}

//----------------------------------------
void checkLeds() {
  if (backlightON) {
//Serial.printf("ledlevel=%d ticks=%d tops=%d\n",ledsLevel,ticks,tops);
    if (ledsLevel > 0 && ledsLevel != previousLedLevel) {
      previousLedLevel = ledsLevel;
      changeDotsLevel(bl, ledsLevel);
    }
  }
}

//----------------------------------------
void checkLight() {
#ifdef DEBUGBL
Serial.printf("blON=%d ldr:%d  prevldr:%d  hyst:%d  offv:%d  divider:%d\n",backlightON,previousLDRValue,display_backlightLDRHysteresis,display_backlightOffLDRValue,display_backlightLDRDivider);
#endif
  if (backlightON) {
    uint16_t ldr = LDRGetValue();
#ifdef DEBUGBL
//Serial.printf("ldr:%d  prevldr:%d  hyst:%d  offv:%d  divider:%d\n",ldr,previousLDRValue,display_backlightLDRHysteresis,display_backlightOffLDRValue,display_backlightLDRDivider);
Serial.printf("ldr:%d \n",ldr);
#endif
    if (display_backlightLDRHysteresis>0 && (abs(ldr-previousLDRValue)>display_backlightLDRHysteresis || previousLDRValue < 0)) {
#ifdef DEBUGBL
Serial.printf("variation LDR \n");
#endif
      previousLDRValue = ldr;
      if (ldr < display_backlightOffLDRValue) {
          ldr=0;
      }
      if (display_backlightLDRDivider > 0) {
        bl = min(int(ldr/display_backlightLDRDivider),255);

      }
      if (bl < display_backlightValueMin) bl = display_backlightValueMin;
      if (bl > display_backlightValueMax) bl = display_backlightValueMax;
#ifdef DEBUGBL
Serial.printf("bl:%d \n",bl);
//char buf[32];
//sprintf(buf,"10|40|%d",bl);
//orderSend("SHOWVALUE",buf,false);
#endif
      setBacklightValue(bl);
    }
  }
}

//----------------------------------------
void alarmBeep() {
  if (alarmON) {
    bip(); delay(50);
    bip(); delay(50);
    bip(); delay(50);
    bip();
  }
}

//----------------------------------------
void backlightOff() {
  backlightON = false;
  setBacklightValue(0);
  changeDotsLevel(0, 0);
}

//-----------------------------------------------------------
void checkButton() {
  if (digitalRead(BUTTON1_PIN)==HIGH && pressedDuration>0) {
    if (alarmON) {
      alarmON=false;
      digitalWrite(BUZZER_PIN, LOW);
    }
    else if (backlightON==false) {
        backlightON=true;
        previousLDRValue=-1;
        scheduler.addOneShotEvent(backlightOff, 5000);
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
//        orderSend("SLEEP","", false);
//        WiFi.mode(WIFI_OFF);
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
      bip(); delay(100); bip();
    }
    if (pressedDuration==LONG_PRESS) {
      bip(); delay(100); bip(); delay(100); bip();
    }
    if (pressedDuration==VERYLONG_PRESS) {
      bip(); delay(100); bip(); delay(100); bip(); delay(100); bip();
    }
  }
}

//-----------------------------------------------------------
void checkIndoorWeather() {
#ifdef DEBUG
//Serial.print("checkindoorw::");
//Serial.print("max free block:");
//Serial.println(heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
#endif
  if (displayMode==DISPLAYMODE_WITH_WEATHER && ledsLevel == 0) {
    getInternalWeather();
  }
}

//-----------------------------------------------------------
void checkWeather() {
  if (displayMode==DISPLAYMODE_WITH_WEATHER && internetOK && ledsLevel == 0) {
#ifdef DEBUG
//Serial.print("checkw::");
//Serial.print("max free block:");
//Serial.println(heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
#endif
    getWeather();
  }
}

//-----------------------------------------------------------
// interruptions de comptage des tours crémaillère pompe
//-----------------------------------------------------------
ICACHE_RAM_ATTR void signal32kHz() {
  if (ledsLevel > 0 && ticks % 350 == 0) {
    ledsLevel--;
  }
  ticks--;
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

  pinMode(BUZZER_PIN, OUTPUT);
  bip();

  i2cScan();

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

//  digitalWrite(BUZZER_PIN, LOW);

  getInternalWeather();

  attachInterrupt(CLOCKOUT_PIN, signal32kHz, FALLING);
  
  scheduler.addEvent(commCheck, 1100);
  scheduler.addEvent(checkButton, 10);
  scheduler.addEvent(checkIndoorWeather, 10000);
  scheduler.addEvent(checkWeather, 300000);
  scheduler.addEvent(checkLight, 1234);
  scheduler.addEvent(checkLeds, 1);

  bip(); delay(100); bip();
  
}

void loop() {

  getAndProcessSerialInput();

  executeOrdersReceived();

  scheduler.update();
}

//