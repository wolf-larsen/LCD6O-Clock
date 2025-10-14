#include "serialCom.h"
#include "Weather.h"

bool commChecked = false;

#define MAX_MESSAGES 100
struct message {
  uint32_t id;
  char type;      // Order/Response
  bool free;      // true: can be used/reused , false:can't be used
  char direction; // Sent/Received
  unsigned long int sentrcvdAt;
  char order[20];
  char text[80];
};
message messagesRecords[MAX_MESSAGES];
uint32_t ordersCount=0x00000100;

int8_t attempts = 3;

//-----------------------------------------------------------
void tryToConnect() {
  if ( ! WiFiIsOK()) {
    if (attempts>=0) {
      attempts--;
      if (wifi_SSID[0]!='?' && wifi_password[0]!='?') {
        WiFiSetup();
        if (WiFiIsOK()) {
          getWeather();
        }
        else {
          Serial.println("WiFi problem.");
        }
      }
      else {
        Serial.println("SSID not valid");
      }
    }
  }
}
//-----------------------------------------------------------
void afterCommChecked() {
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
  orderSend("GETBLINK", "", true);
  scheduler.addOneShotEvent(tryToConnect, 2000);
}


//-----------------------------------------------------------
void serialBufferInit() {
  Serial1.begin(115200,SERIAL_8N1,COM_RX,COM_TX);
  for (uint16_t idx=0; idx<MAX_MESSAGES; idx++) {
    messagesRecords[idx].free = true;
  }
  Serial1.flush();
}

//-----------------------------------------------------------
void executeOrdersReceived() {
  for (uint16_t idx=0; idx<MAX_MESSAGES; idx++) {
    if ( ! messagesRecords[idx].free && messagesRecords[idx].type=='O' && messagesRecords[idx].direction=='R') {
#ifdef DEBUGPROTO
if (strcmp(messagesRecords[idx].order, "TOP")!=0) {Serial.printf("(%lu) Ordre recu %lu %s/%s\n",millis(),messagesRecords[idx].id,messagesRecords[idx].order,messagesRecords[idx].text);}
#endif
      // actions effectuées selon l'ordre reçu
      if (strcmp(messagesRecords[idx].order, "COMMCHECK")==0) {
        responseSend(messagesRecords[idx].id, "OK");
      }
      //------- top a chaque seconde
      else if (strcmp(messagesRecords[idx].order, "TOP")==0) {
        ledsLevel = DOTS_START_LEVEL_MAX;
        ticks = 32768;
      }
      //------- Reset
      else if (strcmp(messagesRecords[idx].order, "RESET")==0) {
        ESP.restart();
      }
//      //------- go to sleep
//      else if (strcmp(messagesRecords[idx].order, "SLEEP")==0) {
//        sleeping = true;
//        WiFi.mode(WIFI_OFF);
//#ifdef DEBUGPROTO
//Serial.printf("ordre SLEEP traite");
//#endif
//      }
//      else if (strcmp(messagesRecords[idx].order, "WAKEUP")==0) {
//        sleeping = false;
////        WiFiSetup();
//      }
      //------- présence internet
      else if (strcmp(messagesRecords[idx].order, "INTERNET")==0) {
        if (strcmp(messagesRecords[idx].text, "OK")==0) {
          internetOK = true;
        }
        else {
          internetOK = false;
        }
      }
      // ---- getweather
      else if (strcmp(messagesRecords[idx].order, "GETW")==0) {
        if (strcmp(messagesRecords[idx].text, "INDOOR")==0) {
//          getInternalWeather();
          String r=String(intWeatherTemp,1);
          r+=','+String(intWeatherHum,0);
          responseSend(messagesRecords[idx].id, r.c_str());
//Serial.printf("ordre GETW/INDOOR traite t=%f h=%f \n",intWeatherTemp,intWeatherHum);
        }
//        else if (strcmp(messagesRecords[idx].text, "CUR.WEATHER")==0 && isWeatherOK()) {
        else if (strcmp(messagesRecords[idx].text, "CUR.WEATHER")==0) {
          String r=String((float)weatherInfos["current"]["temperature_2m"],1);
          r+=','+String((float)weatherInfos["current"]["relative_humidity_2m"],1);
          r+=','+String((const char *)weatherInfos["current_units"]["relative_humidity_2m"]);
          r+=','+String((float)weatherInfos["current"]["precipitation"],1);
          r+=','+String((const char *)weatherInfos["current_units"]["precipitation"]);
          r+=','+String((float)weatherInfos["current"]["wind_speed_10m"],1);
          r+=','+String((const char *)weatherInfos["current_units"]["wind_speed_10m"]);
          r+=','+String((float)weatherInfos["current"]["wind_direction_10m"],1);
          responseSend(messagesRecords[idx].id, r.c_str());
//Serial.printf("ordre GETW/CUR.WEATHER t=%f h=%f \n",(float)weatherInfos["current"]["temperature_2m"],(float)weatherInfos["current"]["relative_humidity_2m"]);
        }
//        else if (strcmp(messagesRecords[idx].text, "FC.TEMPMINMAX")==0 && isWeatherOK()) {
        else if (strcmp(messagesRecords[idx].text, "FC.TEMPMINMAX")==0) {
          String r=String(NBDAYS_FORECAST+1);
          r+='#';
          for (uint8_t fday=0; fday<NBDAYS_FORECAST+1; fday++) {
            r+=String((float)weatherInfos["daily"]["temperature_2m_min"][fday],1);
            r+=','+String((float)weatherInfos["daily"]["temperature_2m_max"][fday],1);
            r+='$';
          }
          responseSend(messagesRecords[idx].id, r.c_str());
        }
//        else if (strcmp(messagesRecords[idx].text, "FC.PREC_UV")==0 && isWeatherOK()) {
        else if (strcmp(messagesRecords[idx].text, "FC.PREC_UV")==0) {
          String r=String(NBDAYS_FORECAST+1);
          r+='#';
          for (uint8_t fday=0; fday<NBDAYS_FORECAST+1; fday++) {
            r+=String((float)weatherInfos["daily"]["precipitation_sum"][fday],1);
            r+=','+String((float)weatherInfos["daily"]["uv_index_max"][fday],1);
            r+='$';
          }
          responseSend(messagesRecords[idx].id, r.c_str());
        }
//        else if (strcmp(messagesRecords[idx].text, "FC.WIND")==0 && isWeatherOK()) {
        else if (strcmp(messagesRecords[idx].text, "FC.WIND")==0) {
          String r=String(NBDAYS_FORECAST+1);
          r+='#';
          for (uint8_t fday=0; fday<NBDAYS_FORECAST+1; fday++) {
            r+=String((float)weatherInfos["daily"]["wind_direction_10m_dominant"][fday],1);
            r+=','+String((float)weatherInfos["daily"]["wind_speed_10m_max"][fday],1);
            r+='$';
          }
          responseSend(messagesRecords[idx].id, r.c_str());
        }
      }
      // ---- setdisplaymode
      else if (strcmp(messagesRecords[idx].order, "SETDISPLAYMODE")==0) {
        long dp = atol(messagesRecords[idx].text);
        if (dp==1 || dp==2) {
          displayMode = dp;
          dotsOff();
        }
        else {
          Serial.printf("SETDISPLAYMODE invalid:%d\n", dp);
        }
      }
      // ---- Bip
      else if (strcmp(messagesRecords[idx].order, "BIP")==0) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(50);
        digitalWrite(BUZZER_PIN, LOW);
      }
      // ---- Alarm start/stop
      else if (strcmp(messagesRecords[idx].order, "ALARM")==0) {
        if (strcmp(messagesRecords[idx].text, "START")==0) {
          alarmON = true;
//          alarmNbSalves = 10;
          for (int salve=0; salve<=10; salve++){
            scheduler.addOneShotEvent(alarmBeep, 100+salve*1000);
          }
        }
        else if (strcmp(messagesRecords[idx].text, "STOP")==0) {
          alarmON=false;
          digitalWrite(BUZZER_PIN, LOW);
        }
        else {
          Serial.print("ALARM mode invalid\n");
        }
      }
      // ---- Backlight ON/OFF
      else if (strcmp(messagesRecords[idx].order, "BL")==0) {
        if (strcmp(messagesRecords[idx].text, "ON")==0) {
          backlightON=true;
          previousLDRValue=-1;
        }
        else if (strcmp(messagesRecords[idx].text, "OFF")==0) {
          backlightOff();
        }
        else {
          Serial.print("BL backlight mode invalid\n");
        }
      }
      // ---- Backlight ON/OFF
      else if (strcmp(messagesRecords[idx].order, "SETBLINK")==0) {
        if (strcmp(messagesRecords[idx].text, "ON")==0) {
          blinkEnable=true;
        }
        else if (strcmp(messagesRecords[idx].text, "OFF")==0) {
          blinkEnable=false;
          changeDotsLevel(0, 0);
        }
        else {
          Serial.print("BL backlight mode invalid\n");
        }
      }
      else {
        Serial.printf("ordre inconnu:%s/%s\n",messagesRecords[idx].order,messagesRecords[idx].text);
      }
      freeMessage(idx);
    }
    else if ( ! messagesRecords[idx].free && messagesRecords[idx].type=='O' && messagesRecords[idx].direction=='S') {
      if (millis()-messagesRecords[idx].sentrcvdAt > 30000) {
        // order aged more than 30sec is deleted
        Serial.printf("ordre %ld (%s:%s) deleted: too old.\n",messagesRecords[idx].id,messagesRecords[idx].order,messagesRecords[idx].text);
        freeMessage(idx);
      }
    }
  }
}

//-----------------------------------------------------------
bool responseReceived(char * command) {
//Serial.printf("reponse recue:%s\n",command);
    char *cmd = strtok(command, "|");
    if (cmd != NULL) {
      uint32_t orderNumber = atol(cmd+1);
      cmd = strtok(NULL, "");
#ifdef DEBUGPROTO
Serial.printf("(%lu) reponse a ma requete %lu:%s \n",millis(),orderNumber,cmd);
#endif
      if (cmd != NULL) {
        int16_t orderFound = -1;
        for (uint16_t idx=0; idx<MAX_MESSAGES; idx++) {
          if (messagesRecords[idx].id == orderNumber && ! messagesRecords[idx].free) {
            orderFound = idx;
            break;
          }
        }
        if (orderFound >= 0) {
//Serial.printf(" (rqt=%s|%s)\n",messagesRecords[orderFound].order,messagesRecords[orderFound].text);
          if (strcmp(messagesRecords[orderFound].order, "COMMCHECK")==0) {
            commChecked = true;
            scheduler.addOneShotEvent(afterCommChecked, 10);
//Serial.printf("comm::commchecked:%d\n",isCommChecked());
          }
          else if (strcmp(messagesRecords[orderFound].order, "GETDISPLAYMODE")==0) {
            displayMode = atol(cmd);
            dotsOff();
          }
          else if (strcmp(messagesRecords[orderFound].order, "GETBLINK")==0) {
            if (strcmp(messagesRecords[orderFound].text, "ON")==0) {
              blinkEnable = true;
            }
            else if (strcmp(messagesRecords[orderFound].text, "OFF")==0) {
              blinkEnable = false;
              dotsOff();
            }
          }
          else if (strcmp(messagesRecords[orderFound].order, "GETCONFIG")==0) {
            if (strcmp(messagesRecords[orderFound].text, "WIFI.SSID")==0) {
              strcpy(wifi_SSID, cmd);
            }
            else if (strcmp(messagesRecords[orderFound].text, "WIFI.PASSWORD")==0) {
              strcpy(wifi_password, cmd);
            }
            else if (strcmp(messagesRecords[orderFound].text, "WEATHER.GPSLAT")==0) {
              weather_GPSlat = atof(cmd);
            }
            else if (strcmp(messagesRecords[orderFound].text, "WEATHER.GPSLON")==0) {
              weather_GPSlon = atof(cmd);
            }
            else if (strcmp(messagesRecords[orderFound].text, "WEATHER.TIMEZONE")==0) {
              strcpy(weather_timezone, cmd);
            }
            else if (strcmp(messagesRecords[orderFound].text, "WEATHER.INDDLY")==0) {
              weather_IndoorWeatherDelaySec = atol(cmd);
            }
            else if (strcmp(messagesRecords[orderFound].text, "WEATHER.OUTDLY")==0) {
              weather_WeatherDelaySec = atol(cmd);
            }
            else if (strcmp(messagesRecords[orderFound].text, "BL.LDRHYST")==0) {
              display_backlightLDRHysteresis = atol(cmd);
            }
            else if (strcmp(messagesRecords[orderFound].text, "BL.LDRDIVIDER")==0) {
              display_backlightLDRDivider = atol(cmd);
            }
            else if (strcmp(messagesRecords[orderFound].text, "BL.MIN")==0) {
              display_backlightValueMin = atol(cmd);
            }
            else if (strcmp(messagesRecords[orderFound].text, "BL.MAX")==0) {
              display_backlightValueMax = atol(cmd);
            }
            else if (strcmp(messagesRecords[orderFound].text, "BL.OFFLDRV")==0) {
              display_backlightOffLDRValue = atol(cmd);
            }
          }
          freeMessage(orderFound);
          return true;
        }
        else {
//Serial.println();
          return false;
        }
      }
      else {
        return false;
      }
    }
    else {
      return false;
    }
  }
  
  //-----------------------------------------------------------
// lit et gère les ordres de la liaison série
//-----------------------------------------------------------
void getAndProcessSerialInput() {
  #define COMMAND_BUFFER_SIZE 100
  static char command[COMMAND_BUFFER_SIZE] = "";
  static uint8_t inputPos = 0;
  static char previousChar=0;
  static bool incomingCommand = false;
  while (Serial1.available() > 0) {
    uint8_t incomingByte = Serial1.read();
    if (incomingByte=='#' && previousChar=='{') {
      if (incomingCommand) {
        if (inputPos > 0) {
          command[inputPos-1] = '\0';
        }
        inputPos = 0;
        incomingCommand = false;
        if (command[0]=='O') {
          //---- got order
          orderReceived(command);
        }
        else if (command[0]=='R') {
          //---- got reponse
          responseReceived(command);
        }
      }
      else {
        inputPos = 0;
        command[inputPos] = '\0';
        incomingCommand = true;
      }
    }
    else if (incomingCommand && incomingByte=='}' && previousChar=='#') {
      if (incomingCommand) {
        //---- fin de commande
        if (inputPos > 0) {
          command[inputPos-1] = '\0';
        }
        inputPos = 0;
        incomingCommand = false;
        if (command[0]=='O') {
          //---- got order
          orderReceived(command);
        }
        else if (command[0]=='R') {
          //---- got response
          responseReceived(command);
        }
      }
    }
    else if (inputPos >= COMMAND_BUFFER_SIZE) {
      //---- overrun
      command[inputPos] = '\0';
      inputPos = 0;
      incomingCommand = false;
    }
    else if (incomingCommand) {
      command[inputPos++] = incomingByte;
    }
    previousChar = incomingByte;
  }
}

//-----------------------------------------------------------
bool orderReceived(char * command) {
#ifdef DEBUGPROTO
//Serial.printf("(%ld) ordre recu:%s\n",millis(),command);
#endif
  char *cmd = strtok(command, "|");
  if (cmd != NULL) {
    message msg;
    msg.id = atol(cmd+1);
    if (msg.id == 0) {
      return false;
    }
    cmd = strtok(NULL, "|");
    if (cmd != NULL) {
      strcpy(msg.order, cmd);
      cmd = strtok(NULL, "");
      if (cmd != NULL) {
        strcpy(msg.text, cmd);
      }
      else {
        msg.text[0]='\0';
      }
//Serial.printf("orderrcvd id,cmd,text=%ld,%s,%s\n",msg.id,msg.order,msg.text);
      msg.type = 'O';
      msg.sentrcvdAt=millis();
      int16_t idxFree = -1;
      for (uint16_t idx=0; idx<MAX_MESSAGES; idx++) {
        if (messagesRecords[idx].free) {
          idxFree = idx;
          break;
        }
      }
//Serial.printf("ordrcv 1st free:%d\n",idxFree);
      if (idxFree>=0) {
//Serial.printf("(%ld) orderrcvd #%d free prend id:%ld:%s/%s\n",millis(),idxFree,msg.id,msg.order,msg.text);
        messagesRecords[idxFree].id = msg.id;
        messagesRecords[idxFree].free = false;
        strcpy(messagesRecords[idxFree].order, msg.order);
        messagesRecords[idxFree].sentrcvdAt = millis();
        messagesRecords[idxFree].type = 'O';
        messagesRecords[idxFree].direction = 'R';
        strcpy(messagesRecords[idxFree].text, msg.text);
//Serial.println("fin ordre");
        return true;
      }
      else {
        return false;
      }
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
}

//-----------------------------------------------------------
uint32_t orderSend(const char * command, long value, bool responseExpected) {
  char buf[20];
  ltoa(value,buf,DEC);
  return orderSend(command, buf, responseExpected);
}
//-----------------------------------------------------------
uint32_t orderSend(const char * command, const char * text, bool responseExpected) {
  if (command != NULL) {
    int16_t idxFree = -1;
    for (uint16_t idx=0; idx<MAX_MESSAGES; idx++) {
      if (messagesRecords[idx].free) {
        idxFree = idx;
        break;
      }
    }
    if (idxFree>=0 || ! responseExpected) {
      ordersCount++;
      if (responseExpected) {
        messagesRecords[idxFree].id = ordersCount;
        messagesRecords[idxFree].free = false;
        strncpy(messagesRecords[idxFree].order, command, sizeof(messagesRecords[idxFree].order));
        messagesRecords[idxFree].sentrcvdAt = millis();
        messagesRecords[idxFree].type = 'O';
        messagesRecords[idxFree].direction = 'S';
        strncpy(messagesRecords[idxFree].text, text, sizeof(messagesRecords[idxFree].text));
      }
//      SerialComm.printf("{#O%d|%s|%s#}\n", ordersCount, command, text);
      Serial1.printf("{#O%d|%s|%s#}\n", ordersCount, command, text);
#ifdef DEBUGPROTO
Serial.printf("(%ld) ordre %ld:%s|%s envoye\n", millis(),ordersCount, command, text);
#endif
      return ordersCount;
    }
    else {
      return 0;
    }
  }
  else {
    return 0;
  }
}

//-----------------------------------------------------------
bool responseSend(uint32_t id, const char * text) {
#ifdef DEBUGPROTO
//Serial.printf("envoi reponse %d,%s\n", id, text);
#endif
  if (id > 0) {
//    SerialComm.printf("{#R%d|%s#}", id, text);
    Serial1.printf("{#R%d|%s#}", id, text);
//Serial.printf("(%ld) reponse %d|%s envoyee\n",millis(), id, text);
    return true;
  }
  else {
    return false;
  }
}

//-----------------------------------------------------------
bool responseSendLong(uint32_t id, long lval) {
  if (id > 0) {
    Serial1.printf("{#R%d|%ld#}", id, lval);
//Serial.printf("(%ld) reponse %d|%ld envoyee\n",millis(), id, lval);
    return true;
  }
  else {
    return false;
  }
}

//-----------------------------------------------------------
bool responseSendDouble(uint32_t id, float fval) {
  if (id > 0) {
    Serial1.printf("{#R%d|%20.6f#}", id, fval);
//Serial.printf("(%ld) reponse %d|%20.6f envoyee\n",millis(), id, fval);
    return true;
  }
  else {
    return false;
  }
}

//-----------------------------------------------------------
void freeMessage(uint16_t idx) {
  messagesRecords[idx].free = true;
}

//-----------------------------------------------------------
bool isCommChecked() {
  return commChecked;
}

