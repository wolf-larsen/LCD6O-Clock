#include "defines.h"
#include <LittleFS.h>
#include "SDaccess.h"
#include "WiFiSetup.h"
#include "configuration.h"
#include "display.h"
#include "hc138.h"
#include "serialCom.h"

IPAddress ip;

//AsyncWebServer server(80);
WebServer server(80);

bool STAMode = false;
bool APMode = false;
#define APNAME_PREFIX "6oclock_"
char APName[30] = APNAME_PREFIX;
String WiFiScanResult;
String themesList = "";

//-----------------------------------------------
void handleRoot() {
  if (LittleFS.exists("/start.html")) {
    File file = LittleFS.open("/start.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  } else {
    server.send(404, "text/plain", "start.html introuvable");
  }
}
//-----------------------------------------------
void handleCSS() {
  if (LittleFS.exists("/style.css")) {
    File file = LittleFS.open("/style.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  } else {
    server.send(404, "text/plain", "style.css introuvable");
  }
}
//-----------------------------------------------
void handleJS() {
  const char *fname = "/start.js";
  if (LittleFS.exists(fname)) {
    File file = LittleFS.open(fname, "r");
    server.streamFile(file, "text/javascript");
    file.close();
  } else {
    server.send(404, "text/plain", "introuvable");
  }
}
//-----------------------------------------------
void handleBG() {
  const char *fname = "/bg.webp";
  if (LittleFS.exists(fname)) {
    File file = LittleFS.open(fname, "r");
    server.streamFile(file, "image/webp");
    file.close();
  } else {
    server.send(404, "text/plain", "introuvable");
  }
}
//-----------------------------------------------
void handleGetParamData() {
  String response = "";
  response += (const char *)config["wifi"]["ssid"];
  response += "|";
  response += (const char *)config["wifi"]["password"];
  response += "|";
  response += WiFiScanResult;
  response += "|";
  response += themesList;
  response += "|";
  response += (int)config["display"]["displayMode"];
  response += "|";
  response += (int)config["display"]["faceNumber"];
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
  response += (bool)config["display"]["blinkSecondsLeds"] ? "Y":"";
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
  for (int id=0; id<=3; id++) {
    response += (const char *)config["splash"]["l1"][id];
    response += "|";
  }
  for (int id=0; id<=3; id++) {
    response += (const char *)config["splash"]["l2"][id];
    response += "|";
  }
  for (int id=0; id<=3; id++) {
    response += (const char *)config["splash"]["l3"][id];
    response += "|";
  }
//Serial.print("params=");
//Serial.println(response);
  server.send(200, "text/plain", response);
}

//-----------------------------------------------
void handleNotFound() {
  server.send(404, "text/plain", "Page non trouvee");
}

//-----------------------------------------------
void handleForm() {
#ifdef DEBUG
Serial.println("form 1");
#endif

  config["wifi"]["ssid"] = server.hasArg("ssid") ? server.arg("ssid") : "?";
  config["wifi"]["password"] = server.hasArg("password") ? server.arg("password") : "?";
#ifdef DEBUG
Serial.println("form 2");
#endif
  if (server.hasArg("displayMode")) {
    int dpmode=0;
    if (server.arg("displayMode")!="") dpmode = server.arg("displayMode").toInt();
    if (dpmode > 0 && dpmode != config["display"]["displayMode"]) {
      config["display"]["displayMode"] = dpmode;
      orderSend("SETDISPLAYMODE",(long)dpmode, false);
    }
  }
  config["display"]["faceNumber"] = server.hasArg("facenum") ? server.arg("facenum").toInt() : 0;
  config["display"]["backlightOffLDRValue"] = server.hasArg("ldroffv") ? server.arg("ldroffv").toInt() : 0;
  config["display"]["backlightLDRDivider"] = server.hasArg("ldrfrac") ? server.arg("ldrfrac").toInt() : 0;
  config["display"]["backlightLDRHysteresis"] = server.hasArg("ldrhyst") ? server.arg("ldrhyst").toInt() : 0;
  config["display"]["backlightValueMin"] = server.hasArg("blmin") ? server.arg("blmin").toInt() : 0;
  config["display"]["backlightValueMax"] = server.hasArg("blmax") ? server.arg("blmax").toInt() : 0;
  config["display"]["backlightOffTime"] = server.hasArg("bltoff") ? server.arg("bltoff") : "";
  config["display"]["backlightOnTime"] = server.hasArg("blton") ? server.arg("blton") : "";
  bool prevBlink=(bool)config["display"]["blinkSecondsLeds"];
  config["display"]["blinkSecondsLeds"] = server.hasArg("blinkleds") ? true : false;
  if (prevBlink != (bool)config["display"]["blinkSecondsLeds"]) {
    orderSend("SETBLINK",prevBlink ? "OFF" : "ON", false);
  }
#ifdef DEBUG
Serial.println("form 4");
#endif
  config["weather"]["WeatherDelaySec"] = server.hasArg("WeatherDelaySec") ? server.arg("WeatherDelaySec").toInt() : 0;
  config["weather"]["IndoorWeatherDelaySec"] = server.hasArg("IndoorWeatherDelaySec") ? server.arg("IndoorWeatherDelaySec").toInt() : 0;
  config["weather"]["GPSlat"] = server.hasArg("GPSlat") ? server.arg("GPSlat").toFloat() : 0;
  config["weather"]["GPSlon"] = server.hasArg("GPSlon") ? server.arg("GPSlon").toFloat() : 0;
  config["weather"]["timezone"] = server.hasArg("wtimezone") ? server.arg("wtimezone") : "";
  config["time"]["NTPAdjustDelaySec"] = server.hasArg("NTPAdjustDelaySec") ? server.arg("NTPAdjustDelaySec").toInt() : 0;
  config["time"]["NTPServer"] = server.hasArg("NTPServer") ? server.arg("NTPServer") : "";
#ifdef DEBUG
Serial.print("ntptimezone=");
Serial.println(server.arg("ntptimezone"));
#endif
  config["time"]["timezone"] = server.hasArg("ntptimezone") ? server.arg("ntptimezone") : "";

#ifdef DEBUG
Serial.println("form 5");
#endif
  config["alarm"]["Alarm1Active"] = server.hasArg("al1c") ? true : false;
  config["alarm"]["Alarm1"] = server.hasArg("al1") ? server.arg("al1") : "";
  config["alarm"]["Alarm2Active"] = server.hasArg("al2c") ? true : false;
  config["alarm"]["Alarm2"] = server.hasArg("al2") ? server.arg("al2") : "";

#ifdef DEBUG
Serial.println("form 6");
#endif
  config["splash"]["l1"][0] = server.arg("ss10");
  config["splash"]["l1"][1] = server.arg("ss11");
  config["splash"]["l1"][2] = server.arg("ss12");
  config["splash"]["l1"][3] = server.arg("ss13");
  config["splash"]["l2"][0] = server.arg("ss20");
  config["splash"]["l2"][1] = server.arg("ss21");
  config["splash"]["l2"][2] = server.arg("ss22");
  config["splash"]["l2"][3] = server.arg("ss23");
  config["splash"]["l3"][0] = server.arg("ss30");
  config["splash"]["l3"][1] = server.arg("ss31");
  config["splash"]["l3"][2] = server.arg("ss32");
  config["splash"]["l3"][3] = server.arg("ss33");
//Serial.println("SVG config");
#ifdef DEBUG
Serial.println("form 7");
#endif
  saveConfigdata();
//Serial.println("SVG faite");
  forceTimeUpdate = true;
  forceDateUpdate = true;
  forceWeatherUpdate = true;
//Serial.println("redirect");
//      clearTFT(TFT_FIRST,TFT_BLACK);
//      clearTFT(TFT_LAST,TFT_BLACK);
//Serial.println("Fin post");
#ifdef DEBUG
Serial.println("form 8");
#endif
  server.send(200, "text/html", "OK");
}

//-----------------------------------------------
void handleSendOrder() {
  String order = server.hasArg("order") ? server.arg("order") : "inconnu";
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
    orderSend("RESET","", false);
    delay(100);
    server.send(200, "text/html", "Reset effectue");
    delay(500);
    ESP.restart();
  }
          
  server.send(200, "text/plain", "OK");
}


////-----------------------------------------------
//void notFound(AsyncWebServerRequest *request) {
//    request->send(404, "text/plain", "Not found");
//}

//-----------------------------------------------
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

//-----------------------------------------------
String WiFiScan() {
#ifdef DEBUG
Serial.println("setup 62a");
#endif
  String WSResult = "";
//return WSResult;
  int n = WiFi.scanNetworks();
#ifdef DEBUG
Serial.println("setup 62b");
#endif
  if (n>0) {
    for (int i = 0; i < n; ++i) {
        WSResult += WiFi.SSID(i).c_str();
        WSResult += "#";
        switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:
            WSResult += "open";
            break;
        case WIFI_AUTH_WEP:
            WSResult += "WEP";
            break;
        case WIFI_AUTH_WPA_PSK:
            WSResult += "WPA";
            break;
        case WIFI_AUTH_WPA2_PSK:
            WSResult += "WPA2";
            break;
        case WIFI_AUTH_WPA_WPA2_PSK:
            WSResult += "WPA+WPA2";
            break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
            WSResult += "WPA2-EAP";
            break;
        case WIFI_AUTH_WPA3_PSK:
            WSResult += "WPA3";
            break;
        case WIFI_AUTH_WPA2_WPA3_PSK:
            WSResult += "WPA2+WPA3";
            break;
        case WIFI_AUTH_WAPI_PSK:
            WSResult += "WAPI";
            break;
        default:
            WSResult += "unknown";
        }
        if (i < n-1) WSResult += "$";
    }
  }
#ifdef DEBUG
Serial.println("setup 62c");
#endif
  WiFi.scanDelete();
#ifdef DEBUG
Serial.println("setup 62d");
#endif
  return WSResult;
}

bool WiFiSetup() {
  APMode = false;
  STAMode = false;

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
#ifdef DEBUG
Serial.println("setup 61");
#endif

  displayIP(ip);
#ifdef DEBUG
Serial.println("setup 62");
#endif

  WiFiScanResult = WiFiScan();
#ifdef DEBUG
Serial.println("setup 63");
#endif
  themesList = getThemesList();
#ifdef DEBUG
Serial.println("setup 64");
#endif

  server.on("/", handleRoot);
  server.on("/style.css", handleCSS);
  server.on("/start.js", handleJS);
  server.on("/get", handleForm);
  server.on("/bg.webp", handleBG);
  server.on("/getParamData", handleGetParamData);
  server.on("/sendOrder", handleSendOrder);
  server.onNotFound(handleNotFound);

  server.begin();

#ifdef DEBUG
Serial.println("setup 65");
#endif
  debugTFT("Ok\n");

  return true;
}


bool WiFiIsOK() {
  return (WiFi.status() == WL_CONNECTED);
}

void displayIP(IPAddress myIP) {
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
  tft.print(myIP[0]);  tft.print('.');
  tft.setCursor(40,160);
  tft.print(myIP[1]);  tft.print('.');
  tft.setCursor(40,190);
  tft.print(myIP[2]);  tft.print('.');
  tft.setCursor(40,220);
  tft.print(myIP[3]);
  tft.setCursor(0,0);
}

