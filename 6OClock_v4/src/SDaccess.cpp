#include <defines.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <display.h>
#include <hc138.h>
#include <SDaccess.h>
#ifdef FS_LITTLEFS
#include <LittleFS.h>
#endif
#ifdef FS_SPIFFS
#include <SPIFFS.h>
#endif

bool SDSetup() {
  char buf[10];
  hc138SetAdress(HC138_SD_CS);
  if(!SD.begin(CS)){
    debugTFT("Card Mount Failed\n");
    return false;
  }
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
    debugTFT("No SD card attached\n");
    return false;
  }
  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.print("MMC\n");
  } else if(cardType == CARD_SD){
    Serial.print("SDSC\n");
  } else if(cardType == CARD_SDHC){
    Serial.print("SDHC\n");
  } else {
    Serial.print("UNKNOWN\n");
    return false;
  }

uint64_t cardSize = SD.cardSize() / (1024 * 1024);
Serial.printf("Card size:%9d Mb\n",cardSize);

//File root = SD.open("/");
//printDirectory(root, 0);
//root.close();

  debugTFT("SD Card OK\n");

  return true;
}

//void refreshImages(bool forceCopy) {
//  File sourceFile;
//  File destFile;
//  uint8_t buf[512];
//  char sfilename[]="/themes/default/X.png";
//  char dfilename[]="/numbers/X.png";
//  hc138SetAdress(HC138_SD_CS);
//  for (uint8_t num=0;num<=9;num++) {
//    sfilename[16]=num+48;
//    sourceFile = SD.open(sfilename);
//    dfilename[9]=num+48;
//#ifdef FS_LITTLEFS
//    if ( ! LittleFS.exists(dfilename)) {
//      deleteFile(LittleFS, dfilename);
//      destFile = LittleFS.open(dfilename, FILE_WRITE);
//#endif
//#ifdef FS_SPIFFS
//    if (forceCopy || ! SPIFFS.exists(dfilename)) {
//      deleteFile(SPIFFS, dfilename);
//      destFile = SPIFFS.open(dfilename, FILE_WRITE);
//#endif
//      Serial.printf("Copying SD:%s to FLASH:%s\n",sfilename,dfilename);
//      while (sourceFile.read(buf, 512) ) {
//        destFile.write(buf, 512);
//      }
//      destFile.close();
//      sourceFile.close();
//    }
//  }
//
//  char sbgname[]="/bg/earthbg0X.png";
//  char dbgname[]="/bg/earthbg0X.png";
//  hc138SetAdress(HC138_SD_CS);
//  for (uint8_t num=1;num<=6;num++) {
//    sbgname[12]=num+48;
//    sourceFile = SD.open(sbgname);
//    dbgname[12]=num+48;
//#ifdef FS_LITTLEFS
//    if ( ! LittleFS.exists(dbgname)) {
//      deleteFile(LittleFS, dbgname);
//      destFile = LittleFS.open(dbgname, FILE_WRITE);
//#endif
//#ifdef FS_SPIFFS
//    if (forceCopy || ! SPIFFS.exists(dbgname)) {
//      deleteFile(SPIFFS, dbgname);
//      destFile = SPIFFS.open(dbgname, FILE_WRITE);
//#endif
//      Serial.printf("Copying SD:%s to FLASH:%s\n",sbgname,dbgname);
//      while (sourceFile.read(buf, 512) ) {
//        destFile.write(buf, 512);
//      }
//      destFile.close();
//      sourceFile.close();
//    }
//  }
//}

String getThemesList() {
  hc138SetAdress(HC138_SD_CS);
  String themes = "";
  uint16_t nbThemes=0;
  File root = SD.open("/themes");
  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()) {
      if (nbThemes>0) {
        themes += "#";
      }
      themes += file.name();
      nbThemes++;
    }
    file = root.openNextFile();
  }
  return themes;
}


void printDirectory(File dir, int numTabs) {
  hc138SetAdress(HC138_SD_CS);
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

//void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
//  hc138SetAdress(HC138_SD_CS);
//  Serial.printf("Listing directory: %s\n", dirname);
//
//  File root = fs.open(dirname);
//  if(!root){
//    Serial.println("Failed to open directory");
//    return;
//  }
//  if(!root.isDirectory()){
//    Serial.println("Not a directory");
//    return;
//  }
//
//  File file = root.openNextFile();
//  while(file){
//    if(file.isDirectory()){
//      Serial.print("  DIR : ");
//      Serial.println(file.name());
//      if(levels){
//        listDir(fs, file.name(), levels -1);
//      }
//    } else {
//      Serial.print("  FILE: ");
//      Serial.print(file.name());
//      Serial.print("  SIZE: ");
//      Serial.println(file.size());
//    }
//    file = root.openNextFile();
//  }
//}

void deleteFile(fs::FS &fs, const char * path){
  hc138SetAdress(HC138_SD_CS);
  Serial.printf("Deleting file: %s\n", path);
  if(fs.remove(path)){
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void updateFaces() {
  uint8_t buf[512];
  hc138SetAdress(HC138_SD_CS);
  for (uint8_t numberImage = 0; numberImage <=9; numberImage++) {
Serial.println(numberImage);
    String sFileName=facesSourcePath+String(numberImage)+".png";
    String dFileName=facesDestinationPath+String(numberImage)+".png";
Serial.println(sFileName);
Serial.println(dFileName);
    if (LittleFS.exists(dFileName)) {
Serial.println("suppression");
      LittleFS.remove(dFileName);
    }
Serial.println("ouv source/dest");
    File sourceFile = SD.open(sFileName,"r");
    File destFile = LittleFS.open(dFileName, FILE_WRITE);
    while (sourceFile.read(buf, 512) ) {
//Serial.print('.');
      destFile.write(buf, 512);
    }
Serial.println("fermetures");
    sourceFile.close();
    destFile.close();
  }
  int displayFaceIndex = facesNum;
  if (displayFaceIndex > DISPLAY_FACE_MAX) {
    displayFaceIndex = 0;
  }
  config["display"]["faceNumber"] = displayFaceIndex;
  saveConfigdata();
  forceTimeUpdate = true;

}