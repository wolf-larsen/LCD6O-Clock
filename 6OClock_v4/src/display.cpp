#include <display.h>
#include <hc138.h>

TFT_eSPI tft = TFT_eSPI();

int16_t xpos = 0;
int16_t ypos = 0;
#define MAX_IMAGE_WIDTH 170
PNG png;
uint8_t currentTFT=0;

//bool nodebug=false;

void initDisplay(uint8_t d) {
  Serial.printf("Init.display %d\n",d);
  hc138SetAdress(d);
  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  // --- Splasch display
  if (d>TFT_FIRST && d<TFT_LAST) {
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(BIG_BFONT);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);
    tft.drawString((const char *)config["splash"]["l1"][d-1], tft.width()/2,  30, GFXFF);
    tft.drawString((const char *)config["splash"]["l2"][d-1], tft.width()/2, 120, GFXFF);
    tft.drawString((const char *)config["splash"]["l3"][d-1], tft.width()/2, 190, GFXFF);
  }
}


void intro() {
  #define MAX_COORD 40
  uint16_t coord[MAX_COORD][2];
  uint8_t pos = 0;
  for (uint8_t i = 0; i < MAX_COORD; coord[i][0]=0,coord[i++][1]=0);
  for (uint16_t col = 0; col < (170 * 6); col++) {
    if (coord[pos][0]!=0 || coord[pos][1]!=0) {
      hc138SetAdress(coord[pos][0] / 170);
      tft.fillCircle(coord[pos][0] % 170,coord[pos][1], 10, TFT_BLUE);
    }
    coord[pos][0]=col;
    float h = abs(sin(((float)col)/50))*(280-col/4);
    coord[pos][1]=319-h;
    hc138SetAdress(col / 170);
    tft.fillCircle(coord[pos][0] % 170,coord[pos][1], 10, TFT_CYAN);
    pos++;
    if (pos >= MAX_COORD) pos=0;
    delay(2);
  }
  for (uint8_t i = 0; i < MAX_COORD; i++) {
      hc138SetAdress(coord[pos][0] / 170);
      tft.fillCircle(coord[pos][0] % 170,coord[pos][1], 10, TFT_BLUE);
      pos++;
      if (pos >= MAX_COORD) pos=0;
      delay(2);
  }
}


void displaySetup() {
  for (uint8_t d=0; d<6; d++) {
    initDisplay(d);
  }
//  intro();
}

void debugTFT(const char* txt) {
  hc138SetAdress(TFT_LAST);
  tft.setTextSize(1);
  tft.setFreeFont(NANO_FONT);
  tft.setTextColor(TFT_GREEN);
  tft.print(txt);
  Serial.print(txt);
}


fs::File myfile;

// ---- Callback display PNGs
void * myOpen(const char *filename, int32_t *size) {
#ifdef FS_LITTLEFS
  myfile = LittleFS.open(filename);
#endif
#ifdef FS_SPIFFS
  myfile = SPIFFS.open(filename);
#endif
  *size = myfile.size();
  return &myfile;
}
//-----------------------------------
void myClose(void *handle) {
  if (myfile) myfile.close();
}
//-----------------------------------
int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length) {
  if (!myfile) return 0;
  return myfile.read(buffer, length);
}
//-----------------------------------
int32_t mySeek(PNGFILE *handle, int32_t position) {
  if (!myfile) return 0;
  return myfile.seek(position);
}
//-----------------------------------
void PNGDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  hc138SetAdress(currentTFT);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}
// argument of type "void (*)(PNGDRAW *pDraw)" is incompatible with parameter of type "PNG_DRAW_CALLBACK *" (aka "int (*)(png_draw_tag *)")

//-----------------------------------
void transparentPNGDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  uint8_t  maskBuffer[1 + MAX_IMAGE_WIDTH / 8];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  hc138SetAdress(currentTFT);
  if (png.getAlphaMask(pDraw, maskBuffer, 255)) {
    pushMaskedImage(xpos, ypos + pDraw->y, pDraw->iWidth, lineBuffer, maskBuffer);
  }
}

//-----------------------------------
void pushMaskedImage(int32_t x, int32_t y, int32_t w, uint16_t *img, uint8_t *mask) {
  // Add glyph validity checks here?
  uint8_t  *mptr = mask;
  uint8_t  *eptr = mask + ((w + 7) >> 3);
  uint16_t *iptr = img;
  // Do RLE for each line
  //while (h--) {
  //  int32_t  wb = ((w + 7) >> 3); // Width of mask line in bytes
    uint32_t xp = 0;
    uint32_t setCount = 0;
    uint32_t clearCount = 0;
    uint8_t  mbyte = 0;
    uint8_t  bits  = 0;

    // Scan through each bit of the mask
    while (mptr < eptr) {
      if (bits == 0) {
        mbyte = *mptr++;
        bits  = 8;
      }
      // Likely image lines starts with transparent pixels
      while ((mbyte & 0x80) == 0) {
        // Deal with case where remaining bits in byte are clear
        if (mbyte == 0x00) {
          clearCount += bits;
          if (mptr >= eptr) break;
          mbyte = *mptr++;
          bits  = 8;
          continue;
        }
        mbyte += mbyte;
        clearCount ++;
        if (--bits) continue;;
        if (mptr >= eptr) break;
        mbyte = *mptr++;
        bits  = 8;
      }

      //Get run length for set bits
      while (mbyte & 0x80) {
        // Deal with case where all bits are set
        if (mbyte == 0xFF) {
          setCount += bits;
          if (mptr >= eptr)  break;
          mbyte = *mptr++;
          continue;
        }
        mbyte += mbyte;
        setCount ++;
        if (--bits) continue;
        if (mptr >= eptr) break;
        mbyte = *mptr++;
        bits  = 8;
      }

      // Dump the pixels
      if (setCount) {
        xp += clearCount;
        tft.pushImage(x + xp, y, setCount, 1, iptr + xp);
        xp += setCount;
        clearCount = 0;
        setCount = 0;
      }
    }

  //  y++;
  //  iptr += w;
  //  mptr += ((w + 7) >> 3);
  //}
}

//-----------------------------------
void displayFile(uint8_t tftNumber, const char * fileName, uint16_t xPosition, uint16_t yPosition) {
//Serial.printf("dispFile %s \n", fileName);
  int16_t rc;
  xpos = xPosition;
  ypos = yPosition;
  hc138SetAdress(tftNumber);
uint32_t timerElapse=millis();

  currentTFT = tftNumber;
//  rc = png.open(fileName, myOpen, myClose, myRead, mySeek, transparentPNGDraw);
//  rc = png.open(fileName, myOpen, myClose, myRead, mySeek, PNGDraw );
//  rc = png.open(fileName, myOpen, myClose, myRead, mySeek, PNGDraw );
  rc = png.open(fileName, myOpen, myClose, myRead, mySeek, (PNG_DRAW_CALLBACK *)(PNGDraw) );
  if (rc == PNG_SUCCESS) {

    tft.startWrite();
//Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
    if (png.getWidth() > MAX_IMAGE_WIDTH) {
      Serial.println("Image too wide for allocated line buffer size!");
    }
    else {
      rc = png.decode(NULL, 0);
    }
    png.close();
    tft.endWrite();
  }
//Serial.printf("%s at tft %d in %d ms\n", fileName, tftNumber, millis()-timerElapse);
}

//-----------------------------------
void displayNumber(uint8_t tftNumber, uint8_t numberToDisplay) {
  int rc;
//  bipPossible=false;
//nodebug=true;
  hc138SetAdress(tftNumber);
  uint32_t timerElapse=millis();

  char file[]="/numbersX/Y.png";
//  file[8]=displayFaceIndex+48;
  file[8]=(long)config["display"]["faceNumber"]+48;
  file[10]=numberToDisplay+48;
  displayFile(tftNumber, file, 0, 0);
//nodebug=false;
//  bipPossible=true;
}

//-----------------------------------
void clearTFT(uint8_t d, uint32_t color) {
  hc138SetAdress(d);
//Serial.printf("(%ld) clearTFT %d\n",millis(),d);
  tft.fillScreen(color);
  tft.setCursor(0,0);
}
//
