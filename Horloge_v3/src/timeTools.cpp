#include <timeTools.h>
#include <display.h>
#include <hc138.h>
#include <WiFiSetup.h>
#include <serialCom.h>

RTC_DS3231 rtc;

bool RTCModuleOK = false;

JsonDocument saints;

uint16_t yPosWeather[2]={0,0};

bool timeSetup() {
//Serial.println("TimeSetup (RTC+NTP)"); delay(1000);

  RTCModuleOK = rtc.begin();
//DateTime currentDateTime = rtc.now();
//Serial.printf("(%ld) timesetup hh:mm:ss=%02d:%02d:%02d \n",millis(),currentDateTime.hour(),currentDateTime.minute(),currentDateTime.second());

  debugTFT("RTC module : ");
  debugTFT(RTCModuleOK ? "OK\n" : "not found\n");
//  rtc.writeSqwPinMode(DS3231_SquareWave1kHz); // non fonctionnel sur DS3231M
  if (WiFiIsOK() && STAMode) {
    const char * ntps = (const char *)config["time"]["NTPServer"];
    const char * timezone = (const char *)config["time"]["timezone"];
//Serial.printf("NTP server, timezone :%s, %s\n",ntps,timezone);
    if (ntps && strlen(ntps)>0 && timezone && strlen(timezone)>0) {
//        configTime((long)config["time"]["GMTOffsetSec"], (long)config["time"]["summerTimeOffsetSec"], ntps);
        configTzTime(timezone, ntps);
    }
//    delay(1000);
    adjustRTCTime();
  }
  return true;
}

void adjustRTCTime() {
//Serial.println("Compare RTC time with NTP");
  DateTime currentDateTime = rtc.now();
  if (RTCModuleOK) {
    currentDateTime = rtc.now();
  }
  if (WiFiIsOK() && STAMode) {
//    if (STAMode && getLocalTime(&timeinfo)) {
    time_t now;
    struct tm *timeinfo;
    time(&now);
    timeinfo = localtime(&now);
    if (RTCModuleOK) {
      currentDateTime = rtc.now();
//Serial.printf("RTC time: %d:%d:%d\n",currentDateTime.hour(),currentDateTime.minute(),currentDateTime.second());
    }
//Serial.printf("NTP time: %02d/%02d/%04d %02d:%02d:%02d\n",timeinfo->tm_mday,timeinfo->tm_mon+1,timeinfo->tm_year+1900,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
    if (RTCModuleOK) {
      if ( timeinfo->tm_year>100 &&
          (currentDateTime.year()!=timeinfo->tm_year+1900
        || currentDateTime.month()!=timeinfo->tm_mon+1
        || currentDateTime.day()!=timeinfo->tm_mday
        || currentDateTime.hour()!=timeinfo->tm_hour
        || currentDateTime.minute()!=timeinfo->tm_min
        || currentDateTime.second()!=timeinfo->tm_sec)
         ) {
Serial.println("Ajust. RTC <- NTP");
        rtc.adjust(DateTime(timeinfo->tm_year+1900,timeinfo->tm_mon+1,timeinfo->tm_mday,timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec));
        currentDateTime = rtc.now();
        displayTime();
      }
    }
  }
}

void displayTime() {
  if (RTCModuleOK) {
//Serial.printf("(%ld) displayTime \n",millis());
    #define YPOS_SECONDS 0
    uint16_t ypos_date=0;
    uint16_t ypos_nextdays=0;
    int8_t hour,minute,second;
    int16_t year=0;
    int8_t month,day,dow;
    int8_t hourd,houru,minuted,minuteu,secondd,secondu;
    static int8_t p_houru=-1,p_hourd=-1,p_minuteu=-1,p_minuted=-1,p_secondu=-1,p_secondd=-1;
    static int8_t p_day=-1;
    char buf[33];

    DateTime currentDateTime = rtc.now();
    year=currentDateTime.year();
    month=currentDateTime.month();
    day=currentDateTime.day();
    dow=currentDateTime.dayOfTheWeek();
    hour=currentDateTime.hour();
    houru=hour%10;
    hourd=hour/10;
    minute=currentDateTime.minute();
    minuteu=minute%10;
    minuted=minute/10;
    second=currentDateTime.second();
    secondu=second%10;
    secondd=second/10;
//Serial.printf("(%ld) hh:mm=%02d:%02d \n",millis(),hour,minute);

    if (year!=0 || forceTimeUpdate || forceDateUpdate) {
        //----------------------------------------------------------------
        if (p_secondu!=secondu && second==0) {
          char currentHourMinute[6];
          sprintf(currentHourMinute,"%02d:%02d",hour,minute);
//Serial.printf("(%ld) currenthh:mm=%s \n",millis(),currentHourMinute);
//Serial.printf("(%ld) al1=%s bloff=%s \n",millis(),(const char *)config["alarm"]["Alarm1"],(const char *)config["display"]["backlightOffTime"]);
          //--- Test & declenchement alarme
          if ((bool)config["alarm"]["Alarm1Active"] && strcmp(currentHourMinute,(const char *)config["alarm"]["Alarm1"])==0) {
            orderSend("ALARM","START",false);
          }
          if ((bool)config["alarm"]["Alarm2Active"] && strcmp(currentHourMinute,(const char *)config["alarm"]["Alarm2"])==0) {
            orderSend("ALARM","START",false);
          }
          //--- Test & allumage/extinction backlight
//          if (strcmp(currentHourMinute,(const char *)config["display"]["backlightOffTime"])==0) {
          if (inInterval(currentHourMinute,(const char *)config["display"]["backlightOffTime"],(const char *)config["display"]["backlightOnTime"])) {
            orderSend("BL","OFF",false);
          }
//          if (strcmp(currentHourMinute,(const char *)config["display"]["backlightOnTime"])==0) {
          if (inInterval(currentHourMinute,(const char *)config["display"]["backlightOnTime"],(const char *)config["display"]["backlightOffTime"])) {
            orderSend("BL","ON",false);
          }
        }

      if ((int)config["display"]["displayMode"]==DISPLAYMODE_WITH_SECONDS) {
        if (p_secondu!=secondu || forceTimeUpdate) {
          //------------ Actions à faire a chaque changement de seconde -------------
          if (p_secondu!=secondu) {
            orderSend("TOP","",false);

          }
          displayNumber(5, secondu);
        }
        if (p_secondd!=secondd || forceTimeUpdate) {
          displayNumber(4, secondd);
          hc138SetAdress(4);
        }
        if (p_minuteu!=minuteu || forceTimeUpdate) {
          displayNumber(3, minuteu);
        }
        if (p_minuted!=minuted || forceTimeUpdate) {
          displayNumber(2, minuted);
        }
        if (p_houru!=houru || forceTimeUpdate) {
          displayNumber(1, houru);
        }
        if (p_hourd!=hourd || forceTimeUpdate) {
          displayNumber(0, hourd);
        }
      }
      //----------------------------------------------------------------
      if ((int)config["display"]["displayMode"]==DISPLAYMODE_WITH_WEATHER) {
        if (p_secondu!=secondu || forceTimeUpdate) {
          //------------ Actions à faire a chaque changement de seconde -------------
          if (p_secondu!=secondu) {
            orderSend("TOP","",false);
          }
          hc138SetAdress(TFT_LAST);
          tft.setTextDatum(TC_DATUM);
          tft.setFreeFont(BIG_BFONT);
          tft.fillRect(40, YPOS_SECONDS,tft.width()-80,tft.fontHeight(GFXFF),TFT_BLACK);
          ypos_nextdays = YPOS_SECONDS+tft.fontHeight(GFXFF);
          tft.setTextColor(TFT_DARKGREEN, TFT_BLACK);
          snprintf(buf,32, "%02d", second);
          tft.drawString(buf, tft.width()/2, YPOS_SECONDS, GFXFF);
        }
        if (p_minuteu!=minuteu || forceTimeUpdate) {
          displayNumber(4, minuteu);
        }
        if (p_minuted!=minuted || forceTimeUpdate) {
          displayNumber(3, minuted);
        }
        if (p_houru!=houru || forceTimeUpdate) {
          displayNumber(2, houru);
        }
        if (p_hourd!=hourd || forceTimeUpdate) {
          displayNumber(1, hourd);
        }
        if (p_day!=day || forceDateUpdate) {
          //------------ Actions à faire a chaque changement de date -----------
//Serial.printf("(%ld) displayTime::changement date ou forced (%d) \n",millis(),forceDateUpdate);
          hc138SetAdress(TFT_FIRST);
          tft.setFreeFont(SMALL_BFONT);
          tft.fillRect(0,0,tft.width(),80,TFT_BLACK);
//tft.drawRoundRect(     0,0,tft.width(),80, 4, TFT_PURPLE);
          tft.setTextColor(TFT_CYAN, TFT_BLACK);
          tft.setTextDatum(TC_DATUM);
          const char * sdow = (const char *)locales["dow"][dow];
          if (sdow) {
            snprintf(buf,32, "%s %d", sdow, day);
            tft.drawString(buf, tft.width()/2, ypos_date, GFXFF);
            ypos_date += tft.fontHeight(GFXFF);
          }
          const char * smonth = (const char *)locales["month"][month-1];
          if (smonth) {
            snprintf(buf,32, "%s %4d", smonth, year);
            tft.drawString(buf, tft.width()/2, ypos_date, GFXFF);
            ypos_date += tft.fontHeight(GFXFF);
          }

          tft.setFreeFont(MICRO_FONT);
          tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
          tft.setTextDatum(TC_DATUM);
//          tft.fillRect(0,ypos_date,tft.width(),tft.fontHeight(GFXFF),TFT_BLACK);
//tft.drawRoundRect(0,0,tft.width(), 35, 4, TFT_RED);
          //---- lecture table des saints
//          hc138SetAdress(HC138_SD_CS);
//          File fsaints=SD.open("/ephemeris.json");
          File fsaints=LittleFS.open("/ephemeris.json");
          bool saintsOK = fsaints;
          if (saintsOK) {
              deserializeJson(saints, fsaints.readString());
              fsaints.close();
              const char * saintdujour = saints[String(month)][day-1][0];
              hc138SetAdress(TFT_FIRST);
              tft.drawString(saintdujour, tft.width()/2, ypos_date, GFXFF);
            }
          //---- display next days
          #define SECONDS_IN_ONE_DAY 86400
          hc138SetAdress(TFT_LAST);
          DateTime day0 = DateTime(year,month,day,0,0,0);
          uint32_t ts = day0.unixtime();
          ypos_nextdays+=4;

          //------ next days loop
          for (uint8_t fday=0; fday<NBDAYS_FORECAST; fday++) {
            ts += SECONDS_IN_ONE_DAY;
            DateTime nextDay = DateTime(ts);
            tft.fillRect(0,ypos_nextdays,tft.width(), 35, TFT_BLACK);
//tft.drawRoundRect(       0,ypos_nextdays,tft.width(), 35, 4, TFT_GREEN);
            tft.drawFastHLine(0,ypos_nextdays+5,tft.width(), TFT_LIGHTGREY);
            tft.setFreeFont(MICRO_IFONT);
            tft.setTextDatum(TR_DATUM);
            tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
//Serial.printf("jour +%d\n",fday+1);
            const char * snextday = (const char *)locales["nextdays"][fday+1];
            if (snextday) {
              tft.drawString(snextday, tft.width(), ypos_nextdays, GFXFF);
              ypos_nextdays += tft.fontHeight(GFXFF);
            }
            tft.setTextColor(TFT_CYAN, TFT_BLACK);
            tft.setFreeFont(MICRO_FONT);
            tft.setTextDatum(TC_DATUM);
            const char * nddow = (const char *)locales["dow"][nextDay.dayOfTheWeek()];
            const char * ndsmonth = locales["shortmonth"][nextDay.month()-1];
            if (dow && smonth) {
              snprintf(buf,32, "%s %d %s",nddow, nextDay.day(),ndsmonth);
              tft.drawString(buf, tft.width()/2, ypos_nextdays, GFXFF);
              ypos_nextdays += tft.fontHeight(GFXFF);
            }
            if (saintsOK) {
              tft.setFreeFont(MICRO_FONT);
              tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
              const char * saintdujour = saints[String(nextDay.month())][nextDay.day()-1][0];
              if (saintdujour) {
                tft.drawString(saintdujour, tft.width()/2, ypos_nextdays, GFXFF);
                ypos_nextdays += tft.fontHeight(GFXFF);
              }
            }
            yPosWeather[fday] = ypos_nextdays;
            ypos_nextdays += 50;
//Serial.printf("fin jour +%d\n",fday+1);
          }
//delay(2000);
        }
      }
      forceTimeUpdate = false;
      forceDateUpdate = false;
    }
    p_day= day;
    p_hourd= hourd;
    p_houru= houru;
    p_minuted= minuted;
    p_minuteu= minuteu;
    p_secondd= secondd;
    p_secondu= secondu;
  }
}

// Convertit "HH:MM" en minutes depuis minuit
uint16_t hhmmToMinutes(const char * hhmm) {
    int heures=0, minutes=0;
//Serial.printf("hhmm=%s \n",hhmm);
    if (strlen(hhmm)==5) {
      sscanf(hhmm, "%02d:%02d",&heures,&minutes);
    }
//Serial.printf("h=%d m=%d \n",heures,minutes);
    return heures * 60 + minutes;
}

// Vérifie si heureTest est dans l'intervalle [heureDebut, heureFin)
bool inInterval(const char *heure_test, const char *heure_debut, const char *heure_fin) {
    uint16_t test = hhmmToMinutes(heure_test);
    uint16_t debut = hhmmToMinutes(heure_debut);
    uint16_t fin = hhmmToMinutes(heure_fin);

    if (debut < fin) {
        return test >= debut && test < fin;
    } else {
        // Passage à minuit
        return test >= debut || test < fin;
    }
}