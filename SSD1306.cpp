#include <cstring>
#include <Arduino.h>
#include "SSD1306.h"
#include <SPI.h>


#ifndef swap16
#define swap16(a, b) \
  { \
    int16_t t = a; \
    a = b; \
    b = t; \
  }
#endif

const uint8_t initData[] = {
  SSD1306_SETLOWCOLUMN,
  SSD1306_DISPLAYOFF,
  SSD1306_SETMULTIPLEX, 0x3f,
  SSD1306_SETDISPLAYOFFSET, 0x00,  //-not offset
  SSD1306_SETSTARTLINE,
  SSD1306_SETCOMPINS, 0x12,
  SSD1306_SETCONTRAST, 0xff,
  SSD1306_DISPLAYALLON_RESUME,
  SSD1306_NORMALDISPLAY,
  SSD1306_SETDISPLAYCLOCKDIV, 0x80,
  SSD1306_CHARGEPUMP, 0x14,
  SSD1306_DISPLAYON,
  SSD1306_MEMORYMODE, 0x10,  // 00,Horizontal Addressing Mode; 01,Vertical Addressing Mode; 10,Page Addressing Mode (RESET); 11,Invalid
  SSD1306_SEGREMAP,
  SSD1306_COMSCANINC
};

SSD1306::SSD1306(int8_t mosi_pin, int8_t sclk_pin, int8_t dc_pin, int8_t rst_pin, int8_t cs_pin) {
  mosiPin = mosi_pin;
  clkPin = sclk_pin;
  dcPin = dc_pin;
  rstPin = rst_pin;
  csPin = cs_pin;
  spi = new SPIClass(HSPI);
}

void SSD1306::init() {

  pinMode(dcPin, OUTPUT);
  pinMode(rstPin, OUTPUT);
  pinMode(csPin, OUTPUT);

  // Initialize SPI
  spi->begin(clkPin, -1, mosiPin);
  spi->setBitOrder(SPI_MSBFIRST);
  spi->setDataMode(SPI_MODE0);
  spi->setClockDivider(SPI_CLOCK_DIV2);

  // Reset OLED
  reset();

  // Wait for the screen to boot
  delay(100);

  for (uint8_t i = 0; i < sizeof(initData); i++)
    writeCommand(initData[i]);

  // Clear screen
  clear();
  dim(false);

  // Flush buffer to screen
  display();

  // Set default values for screen object
  this->CurrentX = 0;
  this->CurrentY = 0;

  this->Initialized = 1;
  delay(100);
}

void SSD1306::display() {
  uint8_t x = 2;

  for (uint8_t i = 0; i < (SSD1306_HEIGHT >> 3); i++) {
    // Set Position
    writeCommand(0xB0 | i);                 // go to page Y
    writeCommand(0x00 | (x & 0xf));         // // lower col addr
    writeCommand(0x10 | ((x >> 4) & 0xf));  // upper col addr
                                            // Write Data
    writeData(&SSD1306_Buffer[SSD1306_WIDTH * i], SSD1306_WIDTH);
  }
}

void SSD1306::clear(SSD1306_COLOR color) {
  uint8_t c = (color == SSD1306_BLACK) ? 0x00 : 0xFF;
  memset(SSD1306_Buffer, c, sizeof(SSD1306_Buffer));
}

void SSD1306::reset() {
  // CS = High (not selected)
  digitalWrite(csPin, HIGH);

  // Reset the OLED
  digitalWrite(rstPin, LOW);
  delay(10);
  digitalWrite(rstPin, HIGH);
  delay(10);
}

// Dim the display
// true: display is dimmed
// false: display is normal
void SSD1306::dim(bool dim) {
  uint8_t contrast;

  if (dim) {
    contrast = 0;  // Dimmed display
  } else {
    if (vccstate == SSD1306_EXTERNALVCC) {
      contrast = 0x9F;
    } else {
      contrast = 0xCF;
    }
  }
  // the range of contrast to too small to be really useful
  // it is useful to dim the display
  writeCommand(SSD1306_SETCONTRAST);
  writeCommand(contrast);
}

void SSD1306::writeCommand(uint8_t data) {
  digitalWrite(csPin, LOW);  // select OLED
  digitalWrite(dcPin, LOW);  // command
  (void)spi->transfer(data);
  digitalWrite(csPin, HIGH);  // un-select OLED
}

void SSD1306::writeData(uint8_t* buffer, size_t buff_size) {
  uint8_t* ptr = buffer;
  digitalWrite(csPin, LOW);   // select OLED
  digitalWrite(dcPin, HIGH);  // data
  while (buff_size--) (void)spi->transfer(*ptr++);
  digitalWrite(csPin, HIGH);  // un-select OLED
}

void SSD1306::setCursor(uint8_t x, uint8_t y) {
  this->CurrentX = x;
  this->CurrentY = y;
}

char SSD1306::writeString(const char* str, FontDef Font, SSD1306_COLOR color) {
  while (*str) {  // Write until null-byte
    if (writeChar(*str, Font, color) != *str) {
      return *str;  // Char could not be written
    }

    str++;  // Next char
  }

  // Everything ok
  return *str;
}

char SSD1306::writeChar(char ch, FontDef Font, SSD1306_COLOR color) {
  uint32_t i, b, j;
  char c = ch;
  // Check if character is valid
  if (ch < 32) return 0;
  if (ch > 126) {
    if (Font.FontHeight != 10 && Font.FontWidth != 7) return 0;

    switch ((uint8_t)ch) {
      case 199: ch = 127; break;
      case 208: ch = 128; break;
      case 214: ch = 129; break;
      case 218: ch = 130; break;
      case 220: ch = 131; break;
      case 222: ch = 132; break;
      case 231: ch = 133; break;

      case 240: ch = 134; break;
      case 243: ch = 135; break;
      case 246: ch = 136; break;
      case 250: ch = 137; break;
      case 252: ch = 138; break;
      case 253: ch = 139; break;
      case 254: ch = 140; break;
    }
  }
  // Check remaining space on current line
  if (SSD1306_WIDTH < (this->CurrentX + Font.FontWidth) || SSD1306_HEIGHT < (this->CurrentY + Font.FontHeight)) {
    // Not enough space on current line
    return 0;
  }

  // Use the font to write
  for (i = 0; i < Font.FontHeight; i++) {
    b = Font.data[(ch - 32) * Font.FontHeight + i];
    for (j = 0; j < Font.FontWidth; j++) {
      if ((b << j) & 0x8000) {
        drawPixel(this->CurrentX + j, (this->CurrentY + i), (SSD1306_COLOR)color);
      } else {
        drawPixel(this->CurrentX + j, (this->CurrentY + i), (SSD1306_COLOR)!color);
      }
    }
  }

  // The current space is now taken
  this->CurrentX += Font.FontWidth;

  // Return written char for validation
  return c;
}

void SSD1306::drawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color) {
  if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
    // Don't write outside the buffer
    return;
  }

  // Check if pixel should be inverted
  if (this->Inverted) {
    color = (SSD1306_COLOR)!color;
  }

  // Draw in the right color
  if (color == SSD1306_WHITE) {
    SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y & 7));  //1 << (y % 8);
  } else {
    SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y & 7));  //~(1 << (y % 8));
  }
}

void SSD1306::drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
  if ((x >= SSD1306_WIDTH) || (y >= SSD1306_HEIGHT)) return;
  if ((x + w - 1) >= SSD1306_WIDTH) w = SSD1306_WIDTH - x;
  if ((y + h - 1) >= SSD1306_HEIGHT) h = SSD1306_HEIGHT - y;

  uint8_t x2 = x + w - 1;
  uint8_t y2 = y + h - 1;

  drawHLine(x, y, w, SSD1306_WHITE);
  drawHLine(x, y2, w, SSD1306_WHITE);
  drawVLine(x, y, h, SSD1306_WHITE);
  drawVLine(x2, y, h, SSD1306_WHITE);
}

void SSD1306::fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, SSD1306_COLOR color) {
  if ((x >= SSD1306_WIDTH) || (y >= SSD1306_HEIGHT)) return;
  if ((x + w - 1) >= SSD1306_WIDTH) w = SSD1306_WIDTH - x;
  if ((y + h - 1) >= SSD1306_HEIGHT) h = SSD1306_HEIGHT - y;

  uint8_t n = 0;

  for (uint8_t j = h; j > 0; j--) {
    drawHLine(x, y + n, w, color);
    n++;
  }
}

void SSD1306::drawHLine(int16_t x, int16_t y, int16_t w, SSD1306_COLOR color) {
  // Do bounds/limit checks
  if (y < 0 || y >= SSD1306_HEIGHT) { return; }

  // make sure we don't try to draw below 0
  if (x < 0) {
    w += x;
    x = 0;
  }

  // make sure we don't go off the edge of the display
  if ((x + w) > SSD1306_WIDTH) {
    w = (SSD1306_WIDTH - x);
  }

  // if our width is now negative, punt
  if (w <= 0) { return; }

  // set up the pointer for  movement through the buffer
  register uint8_t* pBuf = SSD1306_Buffer;
  // adjust the buffer pointer for the current row
  pBuf += ((y / 8) * SSD1306_WIDTH);
  // and offset x columns in
  pBuf += x;
  register uint8_t mask = 1 << (y & 7);

  switch (color) {
    case SSD1306_WHITE:
      while (w--) { *pBuf++ |= mask; };
      break;
    case SSD1306_BLACK:
      mask = ~mask;
      while (w--) { *pBuf++ &= mask; };
      break;
    case SSD1306_INVERSE:
      while (w--) { *pBuf++ ^= mask; };
      break;
  }
}

void SSD1306::drawVLine(int16_t x, int16_t __y, int16_t __h, SSD1306_COLOR color) {
  // do nothing if we're off the left or right side of the screen
  if (x < 0 || x >= SSD1306_WIDTH) { return; }

  // make sure we don't try to draw below 0
  if (__y < 0) {
    // __y is negative, this will subtract enough from __h to account for __y being 0
    __h += __y;
    __y = 0;
  }

  // make sure we don't go past the height of the display
  if ((__y + __h) > SSD1306_HEIGHT) {
    __h = (SSD1306_HEIGHT - __y);
  }

  // if our height is now negative, punt
  if (__h <= 0) {
    return;
  }

  // this display doesn't need ints for coordinates, use local byte registers for faster juggling
  register uint8_t y = __y;
  register uint8_t h = __h;


  // set up the pointer for fast movement through the buffer
  register uint8_t* pBuf = SSD1306_Buffer;
  // adjust the buffer pointer for the current row
  if (y > 0)
    pBuf += ((y / 8) * SSD1306_WIDTH);
  // and offset x columns in
  pBuf += x;

  // do the first partial byte, if necessary - this requires some masking
  register uint8_t mod = (y & 7);
  if (mod) {
    // mask off the high n bits we want to set
    mod = 8 - mod;

    // note - lookup table results in a nearly 10% performance improvement in fill* functions
    // register uint8_t mask = ~(0xFF >> (mod));
    static uint8_t premask[8] = { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
    register uint8_t mask = premask[mod];

    // adjust the mask if we're not going to reach the end of this byte
    if (h < mod) {
      mask &= (0XFF >> (mod - h));
    }

    switch (color) {
      case SSD1306_WHITE: *pBuf |= mask; break;
      case SSD1306_BLACK: *pBuf &= ~mask; break;
      case SSD1306_INVERSE: *pBuf ^= mask; break;
    }

    // fast exit if we're done here!
    if (h < mod) { return; }

    h -= mod;

    pBuf += SSD1306_WIDTH;
  }


  // write solid bytes while we can - effectively doing 8 rows at a time
  if (h >= 8) {
    if (color == SSD1306_INVERSE) {  // separate copy of the code so we don't impact performance of the black/white write version with an extra comparison per loop
      do {
        *pBuf = ~(*pBuf);

        // adjust the buffer forward 8 rows worth of data
        pBuf += SSD1306_WIDTH;

        // adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
        h -= 8;
      } while (h >= 8);
    } else {
      // store a local value to work with
      register uint8_t val = (color == SSD1306_WHITE) ? 255 : 0;

      do {
        // write our value in
        *pBuf = val;

        // adjust the buffer forward 8 rows worth of data
        pBuf += SSD1306_WIDTH;

        // adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
        h -= 8;
      } while (h >= 8);
    }
  }

  // now do the final partial byte, if necessary
  if (h) {
    mod = h & 7;
    // this time we want to mask the low bits of the byte, vs the high bits we did above
    // register uint8_t mask = (1 << mod) - 1;
    // note - lookup table results in a nearly 10% performance improvement in fill* functions
    static uint8_t postmask[8] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
    register uint8_t mask = postmask[mod];
    switch (color) {
      case SSD1306_WHITE: *pBuf |= mask; break;
      case SSD1306_BLACK: *pBuf &= ~mask; break;
      case SSD1306_INVERSE: *pBuf ^= mask; break;
    }
  }
}

void SSD1306::drawImage(uint8_t* img, uint8_t x, uint8_t y, uint8_t w, uint8_t h, SSD1306_COLOR color, SSD1306_COLOR bg) {
  uint8_t bits = 0x80;
  uint8_t bw = (w + 7) / 8;  // Bitmask scanline pad = whole byte
  uint8_t data = img[0];

  for (uint8_t j = 0; j < h; j++) {
    bits = 0;
    y++;
    for (uint8_t i = 0; i < w; i++) {

      drawPixel(x + i, y, (data & bits) ? color : bg);
      //data <<= 1;
      bits = bits >> 1;

      if (bits == 0x00) {
        data = img[(j * bw) + (i / 8)];
        bits = 0x80;
      }
    }
  }
}


void SSD1306::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, SSD1306_COLOR color) {
  int16_t max_radius = ((w < h) ? w : h) / 2;  // 1/2 minor axis
  if (r > max_radius)
    r = max_radius;
  // smarter version
  drawHLine(x + r, y, w - 2 * r, color);          // Top
  drawHLine(x + r, y + h - 1, w - 2 * r, color);  // Bottom
  drawVLine(x, y + r, h - 2 * r, color);          // Left
  drawVLine(x + w - 1, y + r, h - 2 * r, color);  // Right
  // draw four corners
  drawCircleHelper(x + r, y + r, r, 1, color);
  drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
  drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
  drawCircleHelper(x + r, y + h - r - 1, r, 8, color);
}

void SSD1306::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, SSD1306_COLOR color) {
  int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
  if (r > max_radius)
    r = max_radius;
  // smarter version
  fillRect(x + r, y, w - 2 * r, h, color);
  // draw four corners
  fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
  fillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
}

void SSD1306::drawCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR color) {
int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  drawPixel(x0, y0 + r, color);
  drawPixel(x0, y0 - r, color);
  drawPixel(x0 + r, y0, color);
  drawPixel(x0 - r, y0, color);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);
  }
}

void SSD1306::fillCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR color) {
  drawVLine(x0, y0 - r, 2 * r + 1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}

void SSD1306::drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, SSD1306_COLOR color) {
int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    if (cornername & 0x4) {
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 + y, y0 + x, color);
    }
    if (cornername & 0x2) {
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      drawPixel(x0 - y, y0 - x, color);
      drawPixel(x0 - x, y0 - y, color);
    }
  }
}

void SSD1306::fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, SSD1306_COLOR color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;
  int16_t px = x;
  int16_t py = y;

  delta++; // Avoid some +1's in the loop

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    // These checks avoid double-drawing certain lines, important
    // for the SSD1306 library which has an INVERT drawing mode.
    if (x < (y + 1)) {
      if (corners & 1)
        drawVLine(x0 + x, y0 - y, 2 * y + delta, color);
      if (corners & 2)
        drawVLine(x0 - x, y0 - y, 2 * y + delta, color);
    }
    if (y != py) {
      if (corners & 1)
        drawVLine(x0 + py, y0 - px, 2 * px + delta, color);
      if (corners & 2)
        drawVLine(x0 - py, y0 - px, 2 * px + delta, color);
      py = y;
    }
    px = x;
  }
}

void SSD1306::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, SSD1306_COLOR color) {
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}

void SSD1306::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, SSD1306_COLOR color) {

  int16_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    swap16(y0, y1);
    swap16(x0, x1);
  }
  if (y1 > y2) {
    swap16(y2, y1);
    swap16(x2, x1);
  }
  if (y0 > y1) {
    swap16(y0, y1);
    swap16(x0, x1);
  }

  if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if (x1 < a)
      a = x1;
    else if (x1 > b)
      b = x1;
    if (x2 < a)
      a = x2;
    else if (x2 > b)
      b = x2;
    drawHLine(a, y0, b - a + 1, color);
    return;
  }

  int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
          dx12 = x2 - x1, dy12 = y2 - y1;
  int32_t sa = 0, sb = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if (y1 == y2)
    last = y1; // Include y1 scanline
  else
    last = y1 - 1; // Skip it

  for (y = y0; y <= last; y++) {
    a = x0 + sa / dy01;
    b = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;
    /* longhand:
    a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if (a > b)
      swap16(a, b);
    drawHLine(a, y, b - a + 1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = (int32_t)dx12 * (y - y1);
  sb = (int32_t)dx02 * (y - y0);
  for (; y <= y2; y++) {
    a = x1 + sa / dy12;
    b = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;
    /* longhand:
    a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
    b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    */
    if (a > b)
      swap16(a, b);
    drawHLine(a, y, b - a + 1, color);
  }

}

void SSD1306::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, SSD1306_COLOR color) {
  // Update in subclasses if desired!
  if (x0 == x1) {
    if (y0 > y1)
      swap16(y0, y1);
    drawVLine(x0, y0, y1 - y0 + 1, color);
  } else if (y0 == y1) {
    if (x0 > x1)
      swap16(x0, x1);
    drawHLine(x0, y0, x1 - x0 + 1, color);
  } else {
    writeLine(x0, y0, x1, y1, color);
  }
}

void SSD1306::writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, SSD1306_COLOR color) {

  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap16(x0, y0);
    swap16(x1, y1);
  }

  if (x0 > x1) {
    swap16(x0, x1);
    swap16(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

