#include <configuration.h>

//-----------------------------------------------------------
//====== Lire les donnees de config
//-----------------------------------------------------------
void getConfigdata() {
#ifdef FS_LITTLEFS
  File configFile = LittleFS.open("/param.json");
#endif
#ifdef FS_SPIFFS
  File configFile = SPIFFS.open("/param.json");
#endif
  if ( ! configFile) {
    Serial.println("Error opening FLASH:/param.json");
    return;
  }
  String cfg=configFile.readString();
  configFile.close();
Serial.println("Config:");
Serial.println(cfg);
  deserializeJson(config, cfg);
}

//-----------------------------------------------------------
//====== Lire les donnees de config
//-----------------------------------------------------------
void getLocales() {
  //---- locales
#ifdef FS_LITTLEFS
  File localesFile = LittleFS.open("/locales.json");
#endif
#ifdef FS_SPIFFS
  File localesFile = SPIFFS.open("/locales.json");
#endif
  if ( ! localesFile) {
    Serial.println("Error opening FLASH:/locales.json");
    return;
  }
  String loc=localesFile.readString();
  localesFile.close();
//Serial.println("locales:");
//Serial.println(loc);
  deserializeJson(locales, loc);
}

//-----------------------------------------------------------
//====== Ecrire les donnees de config
//-----------------------------------------------------------
void saveConfigdata() {
#ifdef FS_LITTLEFS
  File configFile = LittleFS.open("/param.json", FILE_WRITE);
#endif
#ifdef FS_SPIFFS
  File configFile = SPIFFS.open("/param.json", FILE_WRITE);
#endif
//  String cfg;
  serializeJson(config, configFile);
//  configFile.print(cfg);
  configFile.close();
//getConfigdata();
}
