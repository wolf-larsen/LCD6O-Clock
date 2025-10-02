#include "defines.h"
#include "serialCom.h"
#include "Weather.h"
#include "timeTools.h"
#include "display.h"
#include "WiFiSetup.h"

#define MAX_MESSAGES 20
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
uint32_t ordersCount=0x00001000;

//-----------------------------------------------------------
void serialBufferInit() {
  Serial1.begin(115200,SERIAL_8N1,COM_RX,COM_TX);
  for (uint16_t idx=0; idx<MAX_MESSAGES; idx++) {
    messagesRecords[idx].free = true;
  }
}

//-----------------------------------------------------------
void executeOrdersReceived() {
  for (uint16_t idx=0; idx<MAX_MESSAGES; idx++) {
    if ( ! messagesRecords[idx].free && messagesRecords[idx].type=='O' && messagesRecords[idx].direction=='R') {
#ifdef DEBUG
Serial.printf("(%lu) trait.ordre %lu %s/%s\n",millis(),messagesRecords[idx].id,messagesRecords[idx].order,messagesRecords[idx].text);
#endif
      // actions effectuées selon l'ordre reçu
      // ---- commcheck
      if (strcmp(messagesRecords[idx].order, "COMMCHECK")==0) {
        responseSend(messagesRecords[idx].id, "OK");
      }
      // ---- getconfig
      else if (strcmp(messagesRecords[idx].order, "GETCONFIG")==0) {
        if (strcmp(messagesRecords[idx].text, "WIFI.SSID")==0) {
          responseSend(messagesRecords[idx].id, (const char *)config["wifi"]["ssid"]);
        }
        else if (strcmp(messagesRecords[idx].text, "WIFI.PASSWORD")==0) {
          responseSend(messagesRecords[idx].id, (const char *)config["wifi"]["password"]);
        }
        else if (strcmp(messagesRecords[idx].text, "WEATHER.GPSLAT")==0) {
          responseSend(messagesRecords[idx].id, (double)config["weather"]["GPSlat"]);
        }
        else if (strcmp(messagesRecords[idx].text, "WEATHER.GPSLON")==0) {
          responseSend(messagesRecords[idx].id, (double)config["weather"]["GPSlon"]);
        }
        else if (strcmp(messagesRecords[idx].text, "WEATHER.TIMEZONE")==0) {
          responseSend(messagesRecords[idx].id, (const char *)config["weather"]["timezone"]);
        }
        else if (strcmp(messagesRecords[idx].text, "BL.LDRHYST")==0) {
          responseSend(messagesRecords[idx].id, (long)config["display"]["backlightLDRHysteresis"]);
        }
        else if (strcmp(messagesRecords[idx].text, "BL.LDRDIVIDER")==0) {
          responseSend(messagesRecords[idx].id, (long)config["display"]["backlightLDRDivider"]);
        }
        else if (strcmp(messagesRecords[idx].text, "BL.MIN")==0) {
          responseSend(messagesRecords[idx].id, (long)config["display"]["backlightValueMin"]);
        }
        else if (strcmp(messagesRecords[idx].text, "BL.MAX")==0) {
          responseSend(messagesRecords[idx].id, (long)config["display"]["backlightValueMax"]);
        }
        else if (strcmp(messagesRecords[idx].text, "BL.OFFLDRV")==0) {
          responseSend(messagesRecords[idx].id, (long)config["display"]["backlightOffLDRValue"]);
        }
      }
      // ---- getdisplaymode ---------------
      else if (strcmp(messagesRecords[idx].order, "GETDISPLAYMODE")==0) {
        responseSend(messagesRecords[idx].id, (long)config["display"]["displayMode"]);
        freeMessage(idx);
      }
      // ---- setdisplaymode ---------------
      else if (strcmp(messagesRecords[idx].order, "SETDISPLAYMODE")==0) {
        int dpmode = atol(messagesRecords[idx].text);
        if (dpmode != config["display"]["displayMode"]) {
          config["display"]["displayMode"] = dpmode;
          if (dpmode==DISPLAYMODE_WITH_WEATHER) {
            clearTFT(TFT_FIRST,TFT_BLACK);
            clearTFT(TFT_LAST,TFT_BLACK);
          }
          forceTimeUpdate = true;
          forceDateUpdate = true;
          displayTime();
          if (dpmode==DISPLAYMODE_WITH_WEATHER) {
            displayInternalWeather();
            displayWeather();
          }
        }
        responseSend(messagesRecords[idx].id, "OK");
        freeMessage(idx);
      }
      // ---- change numbers font ---------------
      else if (strcmp(messagesRecords[idx].order, "CHANGEFACE")==0) {
        displayFaceIndex++;
        if (displayFaceIndex > DISPLAY_FACE_MAX) {
          displayFaceIndex = 0;
        }
        forceTimeUpdate = true;
      }
      // ---- display IP ---------------
      else if (strcmp(messagesRecords[idx].order, "DISPLAYIP")==0) {
        clearTFT(TFT_FIRST,TFT_BLACK);
        displayIP();
        delay(5000);
        clearTFT(TFT_FIRST,TFT_BLACK);
        forceTimeUpdate = true;
        forceDateUpdate = true;
      }
      // ---- Reset ---------------
      else if (strcmp(messagesRecords[idx].order, "RESET")==0) {
        ESP.restart();
      }
      //-------------------------------
      else {
        Serial.printf("(%lu) Ordre %lu %s/%s inconnu !\n",millis(),messagesRecords[idx].id,messagesRecords[idx].order,messagesRecords[idx].text);
      }
      freeMessage(idx);
    }
    else if ( ! messagesRecords[idx].free && messagesRecords[idx].type=='O' && messagesRecords[idx].direction=='S') {
      if (millis()-messagesRecords[idx].sentrcvdAt > 10000) {
        // order aged more than 10sec are deleted
//Serial.printf("(%ld) ordre %ld efface car ancien\n",millis(),messagesRecords[idx].id);
        freeMessage(idx);
      }
    }
  }
}

//-----------------------------------------------------------
bool responseReceived(char * command) {
//Serial.printf("(%lu) reponse %s\n",millis(),command);
  char *cmd = strtok(command, "|");
  if (cmd != NULL) {
    uint32_t orderNumber = atol(cmd+1);
    cmd = strtok(NULL, "");
#ifdef DEBUG
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
#ifdef DEBUG
//Serial.printf(" (rqt=%s|%s)\n",messagesRecords[orderFound].order,messagesRecords[orderFound].text);
#endif
        if (strcmp(messagesRecords[orderFound].order, "COMMCHECK")==0) {
          CommChecked = true;
        }
        else if (strcmp(messagesRecords[orderFound].order, "GETW")==0) {
#ifdef DEBUG
//Serial.println("GETW");
#endif

          //-------------------------------------------------------------------------
          if (strcmp(messagesRecords[orderFound].text, "INDOOR")==0) {
            char *resp = strtok(cmd, ","); if (resp != NULL) {
              currentIndoorTemp = atof(resp);
//Serial.printf("(%ld) indoor temp %6.1f \n",millis(),currentIndoorTemp);
            }
            resp = strtok(NULL, ";,/ "); if (resp != NULL) {
              currentIndoorRH = atof(resp);
//Serial.printf("(%ld) indoor hum %6.1f \n",millis(),currentIndoorRH);
            }
            displayInternalWeather();
          }

          //-------------------------------------------------------------------------
          else if (strcmp(messagesRecords[orderFound].text, "CUR.WEATHER")==0) {
#ifdef DEBUG
//Serial.println("CURW");
#endif
            char *resp = strtok(cmd, ","); if (resp != NULL) {
              current_temperature_2m = atof(resp);
//Serial.printf("(%ld) outdoor temp %6.1f \n",millis(),current_temperature_2m);
            }
            resp = strtok(NULL, ";,"); if (resp != NULL) {
              current_relative_humidity_2m=atof(resp);
            }
            resp = strtok(NULL, ";,"); if (resp != NULL) {
              strncpy(current_units_relative_humidity_2m,resp,sizeof(current_units_relative_humidity_2m));
            }
            resp = strtok(NULL, ";,"); if (resp != NULL) {
              current_precipitation=atof(resp);
            }
            resp = strtok(NULL, ";,"); if (resp != NULL) {
              strncpy(current_units_precipitation,resp,sizeof(current_units_precipitation));
            }
            resp = strtok(NULL, ";,"); if (resp != NULL) {
              current_wind_speed_10m=atof(resp);
            }
            resp = strtok(NULL, ";,"); if (resp != NULL) {
              strncpy(current_units_wind_speed_10m,resp,sizeof(current_units_wind_speed_10m));
            }
            resp = strtok(NULL, ";,"); if (resp != NULL) {
              current_wind_direction_10m=atol(resp);
            }
            OutdoorWeatherParametersExpected--;
//Serial.printf("CURW OutdoorWeatherParametersExpected=%d\n",OutdoorWeatherParametersExpected);
            if (OutdoorWeatherParametersExpected <= 0) {
              displayWeather();
            }
          }
          
          //-------------------------------------------------------------------------
          else if (strcmp(messagesRecords[orderFound].text, "FC.TEMPMINMAX")==0) {
            char *resp = strtok(cmd, "#");
            int nbdays = atol(resp);
            char tval[nbdays][20];
            resp = strtok(NULL, "#");
            char *tbvaleurs = strtok(resp, "$");
            strcpy(tval[0],tbvaleurs);
            if (tbvaleurs != NULL) {
              for (int dayfc = 1; dayfc < nbdays; dayfc++) {
                // integration tbvaleurs
                //
                tbvaleurs = strtok(NULL, "$");
                if (tbvaleurs == NULL) {
                  break;
                }
                strcpy(tval[dayfc],tbvaleurs);
              }
            }
            for (int dayfc = 0; dayfc < nbdays; dayfc++) {
              char *tvw = strtok(tval[dayfc], ",;");
              daily_temperature_2m_min[dayfc] = atof(tvw);
              tvw = strtok(NULL, ",;");
              daily_temperature_2m_max[dayfc] = atof(tvw);
            }
            OutdoorWeatherParametersExpected--;
//Serial.printf("CURW OutdoorWeatherParametersExpected=%d\n",OutdoorWeatherParametersExpected);
            if (OutdoorWeatherParametersExpected <= 0) {
              displayWeather();
            }
          }

          //-------------------------------------------------------------------------
          else if (strcmp(messagesRecords[orderFound].text, "FC.PREC_UV")==0) {
            char *resp = strtok(cmd, "#");
            int nbdays = atol(resp);
            char tval[nbdays][20];
            resp = strtok(NULL, "#");
            char *tbvaleurs = strtok(resp, "$");
            strcpy(tval[0],tbvaleurs);
            if (tbvaleurs != NULL) {
              for (int dayfc = 1; dayfc < nbdays; dayfc++) {
                tbvaleurs = strtok(NULL, "$");
                if (tbvaleurs == NULL) {
                  break;
                }
                strcpy(tval[dayfc],tbvaleurs);
              }
            }
            for (int dayfc = 0; dayfc < nbdays; dayfc++) {
              char *tvw = strtok(tval[dayfc], ",;");
              daily_precipitation_sum[dayfc] = atof(tvw);
              tvw = strtok(NULL, ",;");
              daily_uv_index_max[dayfc] = atof(tvw);
            }
            OutdoorWeatherParametersExpected--;
//Serial.printf("CURW OutdoorWeatherParametersExpected=%d\n",OutdoorWeatherParametersExpected);
            if (OutdoorWeatherParametersExpected <= 0) {
              displayWeather();
            }
          }

          //-------------------------------------------------------------------------
          else if (strcmp(messagesRecords[orderFound].text, "FC.WIND")==0) {
            char *resp = strtok(cmd, "#");
            int nbdays = atol(resp);
            char tval[nbdays][20];
            resp = strtok(NULL, "#");
            char *tbvaleurs = strtok(resp, "$");
            strcpy(tval[0],tbvaleurs);
            if (tbvaleurs != NULL) {
              for (int dayfc = 1; dayfc < nbdays; dayfc++) {
                tbvaleurs = strtok(NULL, "$");
                if (tbvaleurs == NULL) {
                  break;
                }
                strcpy(tval[dayfc],tbvaleurs);
              }
            }
            for (int dayfc = 0; dayfc < nbdays; dayfc++) {
              char *tvw = strtok(tval[dayfc], ",;");
              daily_wind_direction_10m_dominant[dayfc] = atof(tvw);
              tvw = strtok(NULL, ",;");
              daily_wind_speed_10m_max[dayfc] = atof(tvw);
            }
            OutdoorWeatherParametersExpected--;
//Serial.printf("CURW OutdoorWeatherParametersExpected=%d\n",OutdoorWeatherParametersExpected);
            if (OutdoorWeatherParametersExpected <= 0) {
              displayWeather();
            }
          }

        }
        // ---- free 'order' message
        freeMessage(orderFound);
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
//Serial.printf("(%c)",(char)incomingByte);
    if (incomingByte=='#' && previousChar=='{') {
      if (incomingCommand) {
//Serial.println("etat:mode commande, sortie mode commande");
        if (inputPos > 0) {
          command[inputPos-1] = '\0';
        }
        inputPos = 0;
        incomingCommand = false;
//Serial.printf("cmd=%s\n",command);
        if (command[0]=='O') {
          //---- got order
//Serial.println("enreg ordre");
          orderReceived(command);
        }
        else if (command[0]=='R') {
          //---- got order
//Serial.println("trait. reponse");
          responseReceived(command);
        }
      }
      else {
        inputPos = 0;
        command[inputPos] = '\0';
        incomingCommand = true;
//Serial.println("entree mode commande");
      }
    }
    else if (incomingCommand && incomingByte=='}' && previousChar=='#') {
      if (incomingCommand) {
//Serial.println("etat:mode commande, sortie mode commande");
        //---- fin de commande
        if (inputPos > 0) {
          command[inputPos-1] = '\0';
        }
        inputPos = 0;
        incomingCommand = false;
//Serial.printf("cmd=%s\n",command);
        if (command[0]=='O') {
          //---- got order
//Serial.println("enreg ordre");
          orderReceived(command);
        }
        else if (command[0]=='R') {
          //---- got order
//Serial.println("trait. reponse");
          responseReceived(command);
        }
      }
    }
    else if (inputPos >= COMMAND_BUFFER_SIZE) {
      //---- overrun
      command[inputPos] = '\0';
      inputPos = 0;
      incomingCommand = false;
//Serial.println("overrun, sortie mode commande");
    }
    else if (incomingCommand) {
      command[inputPos++] = incomingByte;
    }
    previousChar = incomingByte;
  }
}

//-----------------------------------------------------------
bool orderReceived(char * command) {
//Serial.printf("(%ld) ordre recu:%s\n",millis(),command);
  char *cmd = strtok(command, "|");
  if (cmd != NULL) {
    message msg;
    msg.id = atol(cmd+1);
    if (msg.id == 0) {
      return false;
    }
//Serial.printf("order id=%d\n",msg.id);
    cmd = strtok(NULL, "|");
    if (cmd != NULL) {
      strcpy(msg.order, cmd);
//Serial.printf("order cmd=%s\n",msg.order);
      cmd = strtok(NULL, "");
      if (cmd != NULL) {
        strcpy(msg.text, cmd);
//Serial.printf("order text=%s\n",msg.text);
      }
      else {
        msg.text[0]='\0';
      }
      msg.type = 'O';
      msg.sentrcvdAt=millis();
      int16_t idxFree = -1;
      for (uint16_t idx=0; idx<MAX_MESSAGES; idx++) {
        if (messagesRecords[idx].free) {
          idxFree = idx;
          break;
        }
      }
      if (idxFree>=0) {
//Serial.printf("(%ld) orderrcvd #%d free prend id:%ld:%s/%s\n",millis(),idxFree,msg.id,msg.order,msg.text);
        messagesRecords[idxFree].id = msg.id;
        messagesRecords[idxFree].free = false;
        strncpy(messagesRecords[idxFree].order, msg.order, sizeof(msg.order));
        messagesRecords[idxFree].sentrcvdAt = millis();
        messagesRecords[idxFree].type = 'O';
        messagesRecords[idxFree].direction = 'R';
        strncpy(messagesRecords[idxFree].text, msg.text, sizeof(msg.text));
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
#ifdef DEBUG
if (strcmp(command, "TOP")!=0) {Serial.printf("(%ld) ordersend ordre=%s/%s ",millis(),command,text);}
#endif
  if (command != NULL) {
    int16_t idxFree = -1;
    for (uint16_t idx=0; idx<MAX_MESSAGES; idx++) {
//Serial.printf("%d/fr=%d, ",idx,messagesRecords[idx].free);
      if ( ! messagesRecords[idx].free
          && messagesRecords[idx].type=='O'
          && messagesRecords[idx].direction=='S'
          && strncmp(command,messagesRecords[idx].order,sizeof(messagesRecords[idx].order))==0
          && strncmp(text,messagesRecords[idx].text,sizeof(messagesRecords[idx].text))==0
         ) {
//Serial.printf("(%ld) %s/%s deja envoye (%ld)\n",millis(),command,text,command,messagesRecords[idx].id);
          return 0;
      }
      if (messagesRecords[idx].free) {
        idxFree = idx;
        break;
      }
    }
//Serial.printf("\n%d idxfree=%d \n",millis(),idxFree);
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
      Serial1.printf("{#O%d|%s|%s#}\n", ordersCount, command, text);
//Serial.printf("(%ld) ordre %ld:%s|%s envoye\n",millis(),ordersCount, command, text);
if (strcmp(command, "TOP")!=0) {Serial.printf("(%ld) \n",ordersCount);}
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
  if (id > 0) {
    Serial1.printf("{#R%d|%s#}", id, text);
#ifdef DEBUG
//Serial.printf("(%ld) reponse %d|%s envoyee\n",millis(), id, text);
#endif
    return true;
  }
  else {
    return false;
  }
}

//-----------------------------------------------------------
//bool responseSendLong(uint32_t id, long lval) {
bool responseSend(uint32_t id, long lval) {
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
//bool responseSendDouble(uint32_t id, float fval) {
bool responseSend(uint32_t id, double fval) {
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

