#include "defines.h"
#include <LittleFS.h>
#include "SDaccess.h"
#include "WiFiSetup.h"
#include "configuration.h"
#include "display.h"
#include "hc138.h"
#include "serialCom.h"

IPAddress ip;

AsyncWebServer server(80);

bool STAMode = false;
bool APMode = false;
#define APNAME_PREFIX "6oclock_"
char APName[30] = APNAME_PREFIX;

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void getMacString(char *out) {
  uint64_t mac = ESP.getEfuseMac();
  sprintf(out, "%02llX%02llX%02llX%02llX%02llX%02llX",
          (mac >> 40) & 0xFF,
          (mac >> 32) & 0xFF,
          (mac >> 24) & 0xFF,
          (mac >> 16) & 0xFF,
          (mac >> 8)  & 0xFF,
          mac & 0xFF);
}

bool WiFiSetup() {
  APMode = false;
  STAMode = false;

#ifdef DEBUG
Serial.println(ESP.getEfuseMac());
#endif

  WiFi.setMinSecurity(WIFI_AUTH_WPA_PSK);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  debugTFT("Connecting to\n");
  debugTFT((const char *)config["wifi"]["ssid"]);
  debugTFT("\n");
  const char* ssid = (const char *)config["wifi"]["ssid"];
  const char* password = (const char *)config["wifi"]["password"];
  WiFi.setMinSecurity(WIFI_AUTH_WPA_PSK);
  WiFi.begin(ssid, password);
  int8_t nbRetry=10;
  while (WiFi.status() != WL_CONNECTED && nbRetry-->=0) {
    delay(1000);
    debugTFT(".");
  }
  debugTFT("\n");
  if (WiFi.status() != WL_CONNECTED) {
    //---- Passage en AP autonome
    WiFi.disconnect();
    delay(100);
    WiFi.setMinSecurity(WIFI_AUTH_OPEN);
    WiFi.mode(WIFI_AP);
    getMacString(APName+8);
    WiFi.softAP(APName, "");
    if( ! WiFi.softAPConfig(IPAddress(192, 168, 4, 4), IPAddress(192, 168, 4, 4), IPAddress(255, 255, 255, 0))) {
      debugTFT("Unable to set AP\n");
      return false;
    }
    ip = WiFi.softAPIP();
    APMode = true;
  }
  else {
    STAMode = true;
    debugTFT("OK\n");
    ip = WiFi.localIP();
  }
  Serial.printf("IP: %d.%d.%d.%d \n",ip[0],ip[1],ip[2],ip[3]);
  debugTFT("MAC:");
  debugTFT(WiFi.macAddress().c_str());
  debugTFT("\n");
  delay(3000);

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "6o'Clock : Page introuvable");
  });
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
#ifdef DEBUG
Serial.println("acces /");
#endif
      request->send(LittleFS, "/start.html", "text/html");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/style.css", "text/css");
  });
  server.on("/start.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/start.js", "text/javascript");
  });
  server.on("/bg.webp", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/bg.webp", "image/webp");
  });

  server.on("/getParamData", HTTP_GET, [](AsyncWebServerRequest * request){
      String response = "";
      response += (const char *)config["wifi"]["ssid"];
      response += "|";
      response += (const char *)config["wifi"]["password"];
      response += "|";

      int n = WiFi.scanNetworks();
      if (n>0) {
        for (int i = 0; i < n; ++i) {
            response += WiFi.SSID(i).c_str();
            response += "#";
            switch (WiFi.encryptionType(i)) {
            case WIFI_AUTH_OPEN:
                response += "open";
                break;
            case WIFI_AUTH_WEP:
                response += "WEP";
                break;
            case WIFI_AUTH_WPA_PSK:
                response += "WPA";
                break;
            case WIFI_AUTH_WPA2_PSK:
                response += "WPA2";
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                response += "WPA+WPA2";
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                response += "WPA2-EAP";
                break;
            case WIFI_AUTH_WPA3_PSK:
                response += "WPA3";
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
                response += "WPA2+WPA3";
                break;
            case WIFI_AUTH_WAPI_PSK:
                response += "WAPI";
                break;
            default:
                response += "unknown";
            }
            if (i < n-1) response += "$";
//            delay(10);
        }
      }
      WiFi.scanDelete();

      response += "|";
      response += getThemesList();

      response += "|";
      response += (int)config["display"]["displayMode"];
      response += "|";
      response += (long)config["display"]["backlightOffLDRValue"];
      response += "|";
      response += (long)config["display"]["backlightLDRDivider"];
      response += "|";
      response += (long)config["display"]["backlightLDRHysteresis"];
      response += "|";
      response += (long)config["display"]["backlightValueMin"];
      response += "|";
      response += (long)config["display"]["backlightValueMax"];
      response += "|";
      response += (const char *)config["display"]["backlightOffTime"];
      response += "|";
      response += (const char *)config["display"]["backlightOnTime"];
      response += "|";
      response += (long)config["weather"]["WeatherDelaySec"];
      response += "|";
      response += (long)config["weather"]["IndoorWeatherDelaySec"];
      response += "|";
      response += (float)config["weather"]["GPSlat"];
      response += "|";
      response += (float)config["weather"]["GPSlon"];
      response += "|";
      response += (const char *)config["weather"]["timezone"];
      response += "|";
      response += (long)config["time"]["NTPAdjustDelaySec"];
      response += "|";
      response += (const char *)config["time"]["NTPServer"];
      response += "|";
//      response += (long)config["time"]["GMTOffsetSec"];
//      response += "|";
//      response += (long)config["time"]["summerTimeOffsetSec"];
//      response += "|";
      response += (const char *)config["time"]["timezone"];
      response += "|";
      response += (bool)config["alarm"]["Alarm1Active"] ? "Y":"";
      response += "|";
      response += (const char *)config["alarm"]["Alarm1"];
      response += "|";
      response += (bool)config["alarm"]["Alarm2Active"] ? "Y":"";
      response += "|";
      response += (const char *)config["alarm"]["Alarm2"];
      response += "|";
//Serial.print("splash\n");
      for (int id=0; id<=3; id++) {
//Serial.printf("splash l1%d \n",id);
        response += (const char *)config["splash"]["l1"][id];
        response += "|";
      }
      for (int id=0; id<=3; id++) {
//Serial.printf("splash l2%d \n",id);
        response += (const char *)config["splash"]["l2"][id];
        response += "|";
      }
      for (int id=0; id<=3; id++) {
//Serial.printf("splash l3%d \n",id);
        response += (const char *)config["splash"]["l3"][id];
        response += "|";
      }
//Serial.print("params=");
//Serial.println(response);
Serial.print("fin constit\n");
      request->send(200, "text/plain", response);
    }
  );

  server.on("/mainpost",HTTP_POST,[](AsyncWebServerRequest * request){
      config["alarm"]["Alarm1Active"] = false;
      config["alarm"]["Alarm2Active"] = false;

      int paramsNr = request->params();
      for (int i=0; i < paramsNr; i++) {
        AsyncWebParameter* p = request->getParam(i);
//Serial.printf("param:%s=%s\n",p->name().c_str(),p->value().c_str());
        if (p->name() == "ssid") {
          if (p->value()!="") config["wifi"]["ssid"] = p->value().c_str();
//Serial.printf("new ssid=%s\n",(const char *)config["wifi"]["ssid"]);
        }
        else if (p->name() == "password") {
          if (p->value()!="") config["wifi"]["password"] = p->value().c_str();
//Serial.printf("new pw=%s\n",(const char *)config["wifi"]["password"]);
        }

        else if (p->name() == "displayMode") {
          int dpmode=0;
          if (p->value()!="") dpmode = p->value().toInt();
          if (dpmode > 0 && dpmode != config["display"]["displayMode"]) {
            config["display"]["displayMode"] = dpmode;
            orderSend("SETDISPLAYMODE",(long)dpmode, false);
          }
        }
        else if (p->name() == "ldroffv") {
          if (p->value()!="") config["display"]["backlightOffLDRValue"] = p->value().toInt();
        }
        else if (p->name() == "ldrfrac") {
          if (p->value()!="") config["display"]["backlightLDRDivider"] = p->value().toInt();
        }
        else if (p->name() == "ldrhyst") {
          if (p->value()!="") config["display"]["backlightLDRHysteresis"] = p->value().toInt();
        }
        else if (p->name() == "blmin") {
          if (p->value()!="") config["display"]["backlightValueMin"] = p->value().toInt();
        }
        else if (p->name() == "blmax") {
          if (p->value()!="") config["display"]["backlightValueMax"] = p->value().toInt();
        }
        else if (p->name() == "bltoff") {
          if (p->value()!="") config["display"]["backlightOffTime"] = p->value().c_str();
        }
        else if (p->name() == "blton") {
          if (p->value()!="") config["display"]["backlightOnTime"] = p->value().c_str();
        }

        else if (p->name() == "WeatherDelaySec") {
          if (p->value()!="") config["weather"]["WeatherDelaySec"] = p->value().toInt();
        }
        else if (p->name() == "IndoorWeatherDelaySec") {
          if (p->value()!="") config["weather"]["IndoorWeatherDelaySec"] = p->value().toInt();
        }
        else if (p->name() == "GPSlat") {
          if (p->value()!="") config["weather"]["GPSlat"] = p->value().toFloat();
        }
        else if (p->name() == "GPSlon") {
          if (p->value()!="") config["weather"]["GPSlon"] = p->value().toFloat();
        }
        else if (p->name() == "timezone") {
          if (p->value()!="") config["weather"]["timezone"] = p->value().c_str();
        }

        else if (p->name() == "NTPAdjustDelaySec") {
          if (p->value()!="") config["time"]["NTPAdjustDelaySec"] = p->value().toInt();
        }
        else if (p->name() == "NTPServer") {
          if (p->value()!="") config["time"]["NTPServer"] = p->value().c_str();
        }
//        else if (p->name() == "GMTOffsetSec") {
//          if (p->value()!="") config["time"]["GMTOffsetSec"] = p->value().toInt();
//        }
//        else if (p->name() == "summerTimeOffsetSec") {
//          if (p->value()!="") config["time"]["summerTimeOffsetSec"] = p->value().toInt();
//        }
        else if (p->name() == "timezone") {
          if (p->value()!="") config["time"]["timezone"] = p->value().c_str();
        }
      
        else if (p->name() == "al1c") {
          if (p->value()!="") config["alarm"]["Alarm1Active"] = true;
        }
        else if (p->name() == "al1") {
          if (p->value()!="") config["alarm"]["Alarm1"] = p->value().c_str();
        }
        else if (p->name() == "al2c") {
          if (p->value()!="") config["alarm"]["Alarm2Active"] = true;
        }
        else if (p->name() == "al2") {
          if (p->value()!="") config["alarm"]["Alarm2"] = p->value().c_str();
        }
        else if (p->name().startsWith("ss")) {
//Serial.print("Splash ! :");
//Serial.print(p->name());
//Serial.print("=");
//Serial.println(p->value());
          char il[]="lX";
          il[1]=p->name().charAt(2);
//Serial.print("il=");
//Serial.print(il);
          int id=(int)p->name().charAt(3) - '0';
//Serial.print("  id=");
//Serial.println(id);
          config["splash"][il][id] = p->value();
        }
      }
//Serial.println("SVG config");
      saveConfigdata();
//Serial.println("SVG faite");
      forceTimeUpdate = true;
      forceDateUpdate = true;
      forceWeatherUpdate = true;
//Serial.println("redirect");
      request->redirect("/");
//      clearTFT(TFT_FIRST,TFT_BLACK);
//      clearTFT(TFT_LAST,TFT_BLACK);
//Serial.println("Fin post");
    }
  );

//-----------------------------------------------------------
// Transmet les ordres
//-----------------------------------------------------------
  server.on("/sendOrder", HTTP_GET, [](AsyncWebServerRequest * request){
      int paramsNr = request->params();
      for (int i=0; i < paramsNr; i++) {
        AsyncWebParameter* p = request->getParam(i);
        if (p->name() == "order") {
          String order = p->value();
      
//Serial.print("Order:");
//Serial.println(order);
          //-------------------------------
          if (order=="i2cscan") {
          
          }

          //-------------------------------
          else if (order.startsWith("TH|")) {
            if ( ! facesUpdate) {
              uint8_t pos1=order.indexOf("|");
              uint8_t pos2=order.lastIndexOf("|");
              uint8_t faceNum = order.substring(pos1+1,pos2).toInt();
              String faceName = order.substring(pos2+1);
              facesSourcePath="/themes/"+faceName+"/";
              facesDestinationPath="/numbers"+String(faceNum)+"/";
              facesUpdate = true;
            }
          }
          
          //-------------------------------
          else if (order=="reset") {
            ESP.restart();
          }
          
          request->send(200, "text/plain", "OK");
        }
      }
      
    }
  );

  server.begin();
/*
    // Send a POST request to <IP>/post with a form field message set to <message>
    server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
        String message;
        if (request->hasParam(PARAM_MESSAGE, true)) {
            message = request->getParam(PARAM_MESSAGE, true)->value();
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, POST: " + message);
    });

    server.onNotFound(notFound);

        
        HTTPserver->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(SPIFFS, "/mainpage.html", "text/html");
        });
        HTTPserver->on("/param", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/parampage.html", "text/html");
        });
        HTTPserver->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/style.css", "text/css");
        });
        HTTPserver->on("/mainpage.js", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/mainpage.js", "text/javascript");
        });
        HTTPserver->on("/parampage.js", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/parampage.js", "text/javascript");
        });

        HTTPserver->onNotFound([](AsyncWebServerRequest *request){
            request->send(404, "text/plain", "AUTOBARGE : Page introuvable");
        });
*/

  debugTFT("Ok\n");

  return true;
}

void handle_paramPost() {
}


bool WiFiIsOK() {
  return (WiFi.status() == WL_CONNECTED);
}

void displayIP() {
  displayFile(0, "/bg/iphonebg.png", 0, 0);
  hc138SetAdress(0);
//  tft.setFont(&FreeSans24pt7b);
  tft.setTextSize(1);
  if (APMode) {
    tft.setFreeFont(MICRO_FONT);
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(21,60);
    tft.print(APNAME_PREFIX);
    tft.setCursor(21,80);
    tft.print(APName+8);
  }
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(40,100);
  tft.println("IP:");
  tft.setFreeFont(MEDIUM_FONT);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(40,130);
  tft.print(ip[0]);  tft.print('.');
  tft.setCursor(40,160);
  tft.print(ip[1]);  tft.print('.');
  tft.setCursor(40,190);
  tft.print(ip[2]);  tft.print('.');
  tft.setCursor(40,220);
  tft.print(ip[3]);
  tft.setCursor(0,0);
}

void downloadFile(const char * URL, fs::FS &fs, const char * fileName) {
//Serial.printf("DL:%s in %s\n",URL,fileName);
  WiFiClient client;
  HTTPClient http;
  http.begin(client, URL);
  int httpCode = http.GET();
//Serial.printf("Http resp:%d\n",httpCode);
  if (httpCode == HTTP_CODE_OK) {
    int len = http.getSize();

//Serial.printf("len:%d\n",len);
    if (fs.exists(fileName)) {
      fs.remove(fileName);
    }
    File file = fs.open(fileName, FILE_WRITE);
    // create buffer for read
    uint8_t buff[128] = { 0 };

    // get tcp stream
    WiFiClient * stream = http.getStreamPtr();

    // read all data from server
    while(http.connected() && (len > 0 || len == -1)) {
      // get available data size
      size_t size = stream->available();

      if(size) {
        // read up to 128 byte
        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

        // write it to Serial
        file.write(buff,c);
//        Serial.write(buff,c);
        if(len > 0) {
            len -= c;
        }
      }
      delay(1);
    }

//    Serial.print("[HTTP] connection closed or file end.\n");
    file.close();

    if (fs.exists(fileName)) {
//      Serial.printf("%s a bien ete cree\n",fileName);
    }
  }
}

