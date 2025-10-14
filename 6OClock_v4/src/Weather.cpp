#include <Weather.h>
#include <display.h>
#include <hc138.h>
#include <WiFiSetup.h>
#include <configuration.h>
#include <serialCom.h>

//float intWeatherTemp = 0;
//float intWeatherHum = 0;
int OutdoorWeatherParametersExpected = 0;

//------------------------------------------------------------------------------------------------
void getInternalWeather() {
//Serial.printf("(%ld) getInternalWeather \n",millis());
  orderSend("GETW", "INDOOR", true);
}

//------------------------------------------------------------------------------------------------
void displayInternalWeather() {
  #define HEIGHT_TEMPHUM 290
  if ((int)config["display"]["displayMode"]==DISPLAYMODE_WITH_WEATHER) {
//Serial.printf("(%ld) displayInternalWeather T=%3.1f  H=%2.0f\n",millis(),currentIndoorTemp,currentIndoorRH);
    hc138SetAdress(TFT_FIRST);
    tft.fillRect(0,HEIGHT_TEMPHUM-42,170,43,TFT_BLACK);
    tft.setTextSize(1);
    if (currentIndoorTemp!=0.0 || currentIndoorRH!=0.0) {
      tft.setTextColor(TFT_ORANGE);
      tft.setFreeFont(MICRO_IFONT);
      tft.setCursor(50,HEIGHT_TEMPHUM-30);
      const char * ilib=(const char *)locales["weather"]["indoor"];
      if (ilib) {
        tft.print(ilib);
      }
      tft.setFreeFont(MEDIUM_FONT);
      tft.setCursor(0,HEIGHT_TEMPHUM);
      tft.printf("%4.1f",currentIndoorTemp);
      tft.setFreeFont(MICRO_FONT);
//      tft.setCursor(72,HEIGHT_TEMPHUM-10);
      tft.print(" 'C");
      tft.setFreeFont(MEDIUM_FONT);
      tft.setCursor(90,HEIGHT_TEMPHUM);
      tft.printf("%3.0f",currentIndoorRH);
      tft.setFreeFont(MICRO_FONT);
      tft.print("%");
    }
//Serial.printf("(%ld) FIN/displayInternalWeather \n",millis());
  }
}

//------------------------------------------------------------------------------------------------
void degToArrow(long deg, uint16_t xpos, uint16_t ypos, uint8_t radius) {
//return;
  float angleRad = deg * PI / 180.0;
 
  int y1 = ypos + (radius/2) * cos(angleRad);
  int x1 = xpos - (radius/2) * sin(angleRad);
  int y2 = ypos - (radius/2) * cos(angleRad);
  int x2 = xpos + (radius/2) * sin(angleRad);
 
  tft.drawLine(x1, y1, x2, y2, TFT_GOLD);
 
  int wingLength = radius/2;
  float wingAngle = 20 * PI / 180;  // 20 degrés
  int yWing1 = y2 + wingLength * cos(angleRad - wingAngle);
  int xWing1 = x2 - wingLength * sin(angleRad - wingAngle);
  int yWing2 = y2 + wingLength * cos(angleRad + wingAngle);
  int xWing2 = x2 - wingLength * sin(angleRad + wingAngle);
 
  tft.fillTriangle(x2, y2, xWing1, yWing1, xWing2, yWing2, TFT_GOLD);
}

//------------------------------------------------------------------------------------------------
void getWeather() {
//Serial.printf("(%ld) getOutdoorWeather \n",millis());
  OutdoorWeatherParametersExpected = 4;
  orderSend("GETW","CUR.WEATHER", true);
  orderSend("GETW","FC.TEMPMINMAX", true);
  orderSend("GETW","FC.PREC_UV", true);
  orderSend("GETW","FC.WIND", true);
}

//------------------------------------------------------------------------------------------------
void displayWeather() {
#ifdef DEBUG
Serial.printf("(%ld) displayOutdoorWeather \n",millis());
#endif
  uint16_t ypos=100;
  #define YLIBOFFSET 3
  #define XPOSTEMP2 104
  #define XPOSTEMP3 110
  #define XPOSRH2 40
  #define XPOSRH3 44
  #define XPOSPREC2 122
  #define XPOSPREC3 127
  #define XPOSWIND1 60
  #define XPOSWIND2 122
  #define XPOSWIND3 127
  char buf[33];

  if (STAMode && (int)config["display"]["displayMode"]==DISPLAYMODE_WITH_WEATHER) {
    hc138SetAdress(TFT_FIRST);
    tft.fillRect(0,ypos,tft.width(), 100, TFT_BLACK);
#ifdef DEBUG
tft.drawRoundRect(0, ypos, tft.width(), 100, 4, TFT_BLUE);
delay(2000);
#endif
    tft.setTextSize(1);
    //---- temp min/max
    tft.setFreeFont(MICRO_FONT);
    tft.setTextDatum(TL_DATUM);
    snprintf(buf,32, "%4.1f", daily_temperature_2m_min[0]);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.drawString(buf, 0, ypos+6, GFXFF);
    tft.setTextDatum(TR_DATUM);
    snprintf(buf,32, "%4.1f", daily_temperature_2m_max[0]);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.drawString(buf, tft.width(), ypos+6, GFXFF);
    //---- temp actuelle
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setFreeFont(MICRO_FONT);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("'C", XPOSTEMP3, ypos+YLIBOFFSET, GFXFF);
    tft.setFreeFont(MEDIUM_FONT);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TR_DATUM);
    snprintf(buf,32, "%4.1f", current_temperature_2m);
    tft.drawString(buf, XPOSTEMP2, ypos, GFXFF);
    ypos += tft.fontHeight(GFXFF);

    tft.setFreeFont(MICRO_FONT);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(current_units_precipitation, XPOSPREC3, ypos, GFXFF);
    tft.setFreeFont(SMALL_FONT);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TR_DATUM);
    snprintf(buf,32, "%5.1f", current_precipitation);
    tft.drawString(buf, XPOSPREC2, ypos, GFXFF);
//    ypos += tft.fontHeight(GFXFF);

    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setFreeFont(NANO_FONT);
    tft.setTextDatum(TL_DATUM);
    snprintf(buf,32, "%s rh", current_units_relative_humidity_2m);
    tft.drawString(buf, XPOSRH3, ypos+YLIBOFFSET, GFXFF);
    tft.setFreeFont(SMALL_FONT);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TR_DATUM);
    snprintf(buf,32, "%2.0f", current_relative_humidity_2m);
    tft.drawString(buf, XPOSRH2, ypos, GFXFF);
    ypos += tft.fontHeight(GFXFF);

//Serial.printf("Wind dir.=%d\n",current_wind_direction_10m);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(MICRO_FONT);
    tft.drawString(current_units_wind_speed_10m, XPOSWIND3, ypos, GFXFF);
    int wdindex = (int)(((float)current_wind_direction_10m + 11.25f) / 22.5f) % 16;
//Serial.printf("wdindex=%d\n",wdindex);
    tft.drawString((const char *)locales["weather"]["winddir"][wdindex], 0, ypos, GFXFF);
    degToArrow(current_wind_direction_10m, XPOSWIND1, ypos+8, 16);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(SMALL_FONT);
    tft.setTextDatum(TR_DATUM);
    snprintf(buf,32, "%4.0f", current_wind_speed_10m);
    tft.drawString(buf, XPOSWIND2, ypos, GFXFF);
    ypos += tft.fontHeight(GFXFF);

#ifdef DEBUG
Serial.printf("(%ld) displayOutdoorWeather forecast\n",millis());
#endif
    //----- forecast 2 days
    hc138SetAdress(TFT_LAST);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    #define FC_TEMP_MIN_XPOS  40
    #define FC_TEMP_MAX_XPOS  110
    #define FC_UV_XPOS  152
    #define FC_PREC_XPOS  45
    #define FC_WINDS_XPOS  140
    for (uint8_t fday=0; fday<NBDAYS_FORECAST; fday++) {
//Serial.printf("meteo jour +%d ypos[%d]=%d\n",fday,fday,yPosWeather[fday]);
      tft.fillRect(0, yPosWeather[fday], tft.width(), 37, TFT_BLACK);
//tft.drawRoundRect( 0, yPosWeather[fday], tft.width(), 37, 4, TFT_CYAN);
      tft.setFreeFont(MICRO_FONT);
      tft.setTextDatum(TR_DATUM);
      snprintf(buf,32, "%+3.0f", daily_temperature_2m_min[fday+1]);
      tft.setTextColor(TFT_WHITE, TFT_BLUE);
      tft.drawString(buf, FC_TEMP_MIN_XPOS, yPosWeather[fday], GFXFF);
      snprintf(buf,32, "%+3.0f", daily_temperature_2m_max[fday+1]);
      tft.setTextColor(TFT_WHITE, TFT_RED);
      tft.drawString(buf, FC_TEMP_MAX_XPOS, yPosWeather[fday], GFXFF);
      uint8_t nextLine = tft.fontHeight(GFXFF);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setFreeFont(NANO_FONT);
      tft.drawString("min", FC_TEMP_MIN_XPOS-26, yPosWeather[fday]+5, GFXFF);
      tft.drawString("max", FC_TEMP_MAX_XPOS-30, yPosWeather[fday]+5, GFXFF);
      tft.setTextDatum(TL_DATUM);
      tft.drawString("'C", FC_TEMP_MIN_XPOS+2, yPosWeather[fday], GFXFF);
      tft.drawString("'C", FC_TEMP_MAX_XPOS+2, yPosWeather[fday], GFXFF);

      tft.setTextDatum(TL_DATUM);
      tft.setFreeFont(MICRO_FONT);
      snprintf(buf,32, "%2.0f", daily_uv_index_max[fday+1]);
      tft.drawString(buf, FC_UV_XPOS, yPosWeather[fday], GFXFF);
      tft.setTextDatum(TR_DATUM);
      tft.setFreeFont(NANO_FONT);
      tft.drawString("uv", FC_UV_XPOS-1, yPosWeather[fday]+5, GFXFF);

      tft.setFreeFont(MICRO_FONT);
      tft.setTextDatum(TR_DATUM);
      snprintf(buf,32, "%4.1f", daily_precipitation_sum[fday+1]);
      tft.drawString(buf, FC_PREC_XPOS, yPosWeather[fday]+nextLine, GFXFF);
      tft.setTextDatum(TL_DATUM);
//      snprintf(buf,32, "%3.1f", daily_uv_index_max[fday+1]);
//      tft.drawString(buf, FC_UV_XPOS, yPosWeather[fday]+nextLine, GFXFF);
      tft.setFreeFont(NANO_FONT);
      const char * sprec = (const char *)locales["weather"]["precipitation"];
      if (sprec) {
        tft.drawString(sprec, 0, yPosWeather[fday]+nextLine+5, GFXFF);
      }
      const char * unitprec = daily_units_precipitation_sum;
      if (unitprec) {
        tft.drawString(unitprec, FC_PREC_XPOS+2, yPosWeather[fday]+nextLine+5, GFXFF);
      }

//daily_wind_direction_10m_dominant[fday+1]=random(0,360);
//Serial.printf("Wind dir+%d=%d\n",fday+1,daily_wind_direction_10m_dominant[fday+1]);
      degToArrow(daily_wind_direction_10m_dominant[fday+1], (tft.width()/2)+5, yPosWeather[fday]+nextLine+6, 10);
      tft.setTextDatum(TR_DATUM);
      tft.setFreeFont(MICRO_FONT);
      snprintf(buf,32, "%3.0f", daily_wind_speed_10m_max[fday+1]);
      tft.drawString(buf, FC_WINDS_XPOS, yPosWeather[fday]+nextLine, GFXFF);
      tft.setTextDatum(TL_DATUM);
      tft.setFreeFont(NANO_FONT);
      tft.drawString(current_units_wind_speed_10m, FC_WINDS_XPOS+3, yPosWeather[fday]+nextLine+5, GFXFF);
      int wdindex = (int)(((float)daily_wind_direction_10m_dominant[fday+1] + 11.25f) / 22.5f) % 16;
      tft.setTextDatum(TR_DATUM);
      tft.drawString((const char *)locales["weather"]["winddir"][wdindex], (tft.width()/2)-5, yPosWeather[fday]+nextLine+6, GFXFF);
    }
  }
  forceWeatherUpdate = false;
}
