#ifndef __SDACCESS_H__
#define __SDACCESS_H__

#include "defines.h"
#include <FS.h>
#include <SD.h>
#include "configuration.h"

extern bool forceTimeUpdate;

extern bool facesUpdate;
extern String facesSourcePath;
extern String facesDestinationPath;
extern uint8_t facesNum;

bool SDSetup();
//void refreshImages(bool forceCopy);
//void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void deleteFile(fs::FS &fs, const char * path);
String getThemesList();
void printDirectory(File dir, int numTabs);
void updateFaces();

#endif
