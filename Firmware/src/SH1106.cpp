/**The MIT License (MIT)

Copyright (c) 2015 by Daniel Eichhorn

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

See more at http://blog.squix.ch

Credits for parts of this code go to Mike Rankin. Thank you so much for sharing!
*/

#include "SH1106.h"
#include <Wire.h>


SH1106::SH1106(int i2cAddress, int sda, int sdc) {
  myI2cAddress = i2cAddress;
  mySda = sda;
  mySdc = sdc;
}

void SH1106::init() {
  Wire.begin(mySda, mySdc);
  Wire.setClock(400000);
  sendInitCommands();
  resetDisplay();
}

void SH1106::resetDisplay(void) {
  displayOff();
  clear();
  display();
  displayOn();
}

void SH1106::reconnect() {
  Wire.begin(mySda, mySdc);
}

void SH1106::displayOn(void) {
    sendCommand(0xaf);        //display on
}

void SH1106::displayOff(void) {
  sendCommand(0xae);          //display off
}

void SH1106::setContrast(char contrast) {
  sendCommand(0x81);
  sendCommand(contrast);
}

void SH1106::flipScreenVertically() {
  sendCommand(0xA0 | 0x1);      //SEGREMAP   //Rotate screen 180 deg
  sendCommand(0xC8);            //COMSCANDEC  Rotate screen 180 Deg
}

void SH1106::clear(void) {
    memset(buffer, 0, (128 * 64 / 8));
}

void SH1106::display(void) {

 for (uint8_t k=0; k<8; k++) 
    {
      sendCommand(0xB0+k);//set page addressSSD_Data_Mode;
      sendCommand(0x02) ;//set lower column address
      sendCommand(0x10) ;//set higher column address

       for (uint16_t i=0; i< 128; i++) {
          // send a bunch of data in one xmission
          //Wire.begin(mySda, mySdc);
          Wire.beginTransmission(myI2cAddress);
          Wire.write(0x40);
          for (uint8_t x=0; x<16; x++) {
          Wire.write(buffer[128*k+i]);
          i++;
          }
        i--;
        yield();
        Wire.endTransmission();
        }
      }
}

void SH1106::setPixel(int x, int y) {
  if (x >= 0 && x < 128 && y >= 0 && y < 64) {

     switch (myColor) {
      case WHITE:   buffer[x + (y/8)*128] |=  (1 << (y&7)); break;
      case BLACK:   buffer[x + (y/8)*128] &= ~(1 << (y&7)); break;
      case INVERSE: buffer[x + (y/8)*128] ^=  (1 << (y&7)); break;
    }
  }
}

void SH1106::setChar(int x, int y, unsigned char data) {
  for (int i = 0; i < 8; i++) {
    if (bitRead(data, i)) {
     setPixel(x,y + i);
    }
  }
}

// Code form http://playground.arduino.cc/Main/Utf8ascii
byte SH1106::utf8ascii(byte ascii) {
    if ( ascii<128 ) {   // Standard ASCII-set 0..0x7F handling
       lastChar=0;
       return( ascii );
    }

    // get previous input
    byte last = lastChar;   // get last char
    lastChar=ascii;         // remember actual character

    switch (last)     // conversion depnding on first UTF8-character
    {   case 0xC2: return  (ascii);  break;
        case 0xC3: return  (ascii | 0xC0);  break;
        case 0x82: if(ascii==0xAC) return(0x80);       // special case Euro-symbol
    }

    return  (0);                                     // otherwise: return zero, if character has to be ignored
}

// Code form http://playground.arduino.cc/Main/Utf8ascii
String SH1106::utf8ascii(String s) {
        String r= "";
        char c;
        for (int i=0; i<s.length(); i++)
        {
                c = utf8ascii(s.charAt(i));
                if (c!=0) r+=c;
        }
        return r;
}

void SH1106::drawString(int x, int y, String text) {
  text = utf8ascii(text);
  unsigned char currentByte;
  int charX, charY;
  int currentBitCount;
  int charCode;
  int currentCharWidth;
  int currentCharStartPos;
  int cursorX = 0;
  int numberOfChars = pgm_read_byte(myFontData + CHAR_NUM_POS);
  // iterate over string
  int firstChar = pgm_read_byte(myFontData + FIRST_CHAR_POS);
  int charHeight = pgm_read_byte(myFontData + HEIGHT_POS);
  int currentCharByteNum = 0;
  int startX = 0;
  int startY = y;

  if (myTextAlignment == TEXT_ALIGN_LEFT) {
    startX = x;
  } else if (myTextAlignment == TEXT_ALIGN_CENTER) {
    int width = getStringWidth(text);
    startX = x - width / 2;
  } else if (myTextAlignment == TEXT_ALIGN_RIGHT) {
    int width = getStringWidth(text);
    startX = x - width;
  }

  for (int j=0; j < text.length(); j++) {

    charCode = text.charAt(j)-0x20;

    currentCharWidth = pgm_read_byte(myFontData + CHAR_WIDTH_START_POS + charCode);
    // Jump to font data beginning
    currentCharStartPos = CHAR_WIDTH_START_POS + numberOfChars;

    for (int m = 0; m < charCode; m++) {

      currentCharStartPos += pgm_read_byte(myFontData + CHAR_WIDTH_START_POS + m)  * charHeight / 8 + 1;
    }

    currentCharByteNum = ((charHeight * currentCharWidth) / 8) + 1;
    // iterate over all bytes of character
    for (int i = 0; i < currentCharByteNum; i++) {

      currentByte = pgm_read_byte(myFontData + currentCharStartPos + i);
      //Serial.println(String(charCode) + ", " + String(currentCharWidth) + ", " + String(currentByte));
      // iterate over all bytes of character
      for(int bit = 0; bit < 8; bit++) {
         //int currentBit = bitRead(currentByte, bit);

         currentBitCount = i * 8 + bit;

         charX = currentBitCount % currentCharWidth;
         charY = currentBitCount / currentCharWidth;

         if (bitRead(currentByte, bit)) {
          setPixel(startX + cursorX + charX, startY + charY);
         }

      }
      yield();
    }
    cursorX += currentCharWidth;

  }
}

void SH1106::drawStringMaxWidth(int x, int y, int maxLineWidth, String text) {
  int currentLineWidth = 0;
  int startsAt = 0;
  int endsAt = 0;
  int lineNumber = 0;
  char currentChar = ' ';
  int lineHeight = pgm_read_byte(myFontData + HEIGHT_POS);
  String currentLine = "";
  for (int i = 0; i < text.length(); i++) {
    currentChar = text.charAt(i);
    if (currentChar == ' ' || currentChar == '-') {
      String lineCandidate = text.substring(startsAt, i);
      if (getStringWidth(lineCandidate) <= maxLineWidth) {
        endsAt = i;
      } else {

        drawString(x, y + lineNumber * lineHeight, text.substring(startsAt, endsAt));
        lineNumber++;
        startsAt = endsAt + 1;
      }
    }

  }
  drawString(x, y + lineNumber * lineHeight, text.substring(startsAt));
}

int SH1106::getStringWidth(String text) {
  text = utf8ascii(text);
  int stringWidth = 0;
  char charCode;
  for (int j=0; j < text.length(); j++) {
    charCode = text.charAt(j)-0x20;
    stringWidth += pgm_read_byte(myFontData + CHAR_WIDTH_START_POS + charCode);
  }
  return stringWidth;
}

void SH1106::setTextAlignment(int textAlignment) {
  myTextAlignment = textAlignment;
}

void SH1106::setFont(const char *fontData) {
  myFontData = fontData;
}

void SH1106::drawBitmap(int x, int y, int width, int height, const char *bitmap) {
  for (int i = 0; i < width * height / 8; i++ ){
    unsigned char charColumn = 255 - pgm_read_byte(bitmap + i);
    for (int j = 0; j < 8; j++) {
      int targetX = i % width + x;
      int targetY = (i / (width)) * 8 + j + y;
      if (bitRead(charColumn, j)) {
        setPixel(targetX, targetY);
      }
    }
  }
}

void SH1106::setColor(int color) {
  myColor = color;
}

void SH1106::drawRect(int x, int y, int width, int height) {
  for (int i = x; i < x + width; i++) {
    setPixel(i, y);
    setPixel(i, y + height);
  }
  for (int i = y; i < y + height; i++) {
    setPixel(x, i);
    setPixel(x + width, i);
  }
}

void SH1106::fillRect(int x, int y, int width, int height) {
  for (int i = x; i < x + width; i++) {
    for (int j = y; j < y + height; j++) {
      setPixel(i, j);
    }
  }
}

void SH1106::drawXbm(int x, int y, int width, int height, const char *xbm) {
  if (width % 8 != 0) {
    width =  ((width / 8) + 1) * 8;
  }
  for (int i = 0; i < width * height / 8; i++ ){
    unsigned char charColumn = pgm_read_byte(xbm + i);
    for (int j = 0; j < 8; j++) {
      int targetX = (i * 8 + j) % width + x;
      int targetY = (8 * i / (width)) + y;
      if (bitRead(charColumn, j)) {
        setPixel(targetX, targetY);
      }
    }
  }
}

void SH1106::sendCommand(unsigned char com) {
  Wire.beginTransmission(myI2cAddress);     //begin transmitting
  Wire.write(0x80);                          //command mode
  Wire.write(com);
  Wire.endTransmission();                    // stop transmitting
}

void SH1106::sendInitCommands(void) {
  sendCommand(DISPLAYOFF);
  sendCommand(NORMALDISPLAY);
  sendCommand(SETDISPLAYCLOCKDIV);
  sendCommand(0x80);
  sendCommand(SETMULTIPLEX);
  sendCommand(0x3F);
  sendCommand(SETDISPLAYOFFSET);
  sendCommand(0x00);
  sendCommand(SETSTARTLINE | 0x00);
  sendCommand(CHARGEPUMP);
  sendCommand(0x14);
  sendCommand(MEMORYMODE);
  sendCommand(0x00);
  sendCommand(SEGREMAP);
  sendCommand(COMSCANINC);
  sendCommand(SETCOMPINS);
  sendCommand(0x12);
  sendCommand(SETCONTRAST);
  sendCommand(0xCF);
  sendCommand(SETPRECHARGE);
  sendCommand(0xF1);
  sendCommand(SETVCOMDETECT);
  sendCommand(0x40);
  sendCommand(DISPLAYALLON_RESUME);
  sendCommand(NORMALDISPLAY);
  sendCommand(0x2e);            // stop scroll
  sendCommand(DISPLAYON);
}
