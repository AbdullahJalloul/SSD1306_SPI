#ifndef SSD1306_H
#define SSD1306_H

#include "SSD1306_Fonts.h"
#include <SPI.h>

// SSD1306 OLED height in pixels
#ifndef SSD1306_HEIGHT
#define SSD1306_HEIGHT 64
#endif

// SSD1306 width in pixels
#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH 128
#endif

enum {
  SSD1306_SETLOWCOLUMN = 0x00,
  SSD1306_SETHIGHCOLUMN = 0x10,
  SSD1306_MEMORYMODE = 0x20,
  SSD1306_COLUMNADDR = 0x21,
  SSD1306_PAGEADDR = 0x22,
  SSD1306_SETSTARTLINE = 0x40,
  SSD1306_DEFAULT_ADDRESS = 0x78,
  SSD1306_SETCONTRAST = 0x81,
  SSD1306_CHARGEPUMP = 0x8D,
  SSD1306_SEGREMAP = 0xA0,
  SSD1306_DISPLAYALLON_RESUME = 0xA4,
  SSD1306_DISPLAYALLON = 0xA5,
  SSD1306_NORMALDISPLAY = 0xA6,
  SSD1306_INVERTDISPLAY = 0xA7,
  SSD1306_SETMULTIPLEX = 0xA8,
  SSD1306_DISPLAYOFF = 0xAE,
  SSD1306_DISPLAYON = 0xAF,
  SSD1306_SETPAGE = 0xB0,
  SSD1306_COMSCANINC = 0xC0,
  SSD1306_COMSCANDEC = 0xC8,
  SSD1306_SETDISPLAYOFFSET = 0xD3,
  SSD1306_SETDISPLAYCLOCKDIV = 0xD5,
  SSD1306_SETPRECHARGE = 0xD9,
  SSD1306_SETCOMPINS = 0xDA,
  SSD1306_SETVCOMDETECT = 0xDB,
  SSD1306_EXTERNALVCC = 0x01,
  SSD1306_SWITCHCAPVCC = 0x02,
  SSD1306_NOP = 0xE3
};

enum MemoryMode {
  HORIZONTAL_ADDRESSING_MODE = 0x00,
  VERTICAL_ADDRESSING_MODE = 0x01,
  PAGE_ADDRESSING_MODE = 0x02
};

enum ScrollMode {
  ACTIVATE_SCROLL = 0x2F,
  DEACTIVATE_SCROLL = 0x2E,
  SET_VERTICAL_SCROLL_AREA = 0xA3,
  RIGHT_HORIZONTAL_SCROLL = 0x26,
  LEFT_HORIZONTAL_SCROLL = 0x27,
  VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL = 0x29,
  VERTICAL_AND_LEFT_HORIZONTAL_SCROLL = 0x2A
};

enum SSD1306_COLOR {
  SSD1306_BLACK = 0,
  SSD1306_WHITE = 1,
  SSD1306_INVERSE = 2
};

class SSD1306 {
public:
  SSD1306(int8_t mosi_pin, int8_t sclk_pin, int8_t dc_pin, int8_t rst_pin, int8_t cs_pin);
  void init();
  void display();
  void clear(SSD1306_COLOR color = SSD1306_BLACK);

  // Low-level procedures
  void reset();
  void dim(bool dim);
  void writeCommand(uint8_t data);
  void writeData(uint8_t* buffer, size_t buff_size);

  void setCursor(uint8_t x, uint8_t y);
  char writeString(const char* str, FontDef Font, SSD1306_COLOR color);
  char writeChar(char ch, FontDef Font, SSD1306_COLOR color);
  void drawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color);
  void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
  void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, SSD1306_COLOR color);
  void drawHLine(int16_t x, int16_t y, int16_t w, SSD1306_COLOR color);
  void drawVLine(int16_t x, int16_t y, int16_t h, SSD1306_COLOR color);
  void drawImage(uint8_t* img, uint8_t x, uint8_t y, uint8_t w, uint8_t h, SSD1306_COLOR color, SSD1306_COLOR bg);

  void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, SSD1306_COLOR color);
  void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, SSD1306_COLOR color);
  void drawCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR color);
  void fillCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR color);
  void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, SSD1306_COLOR color);
  void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t corners, int16_t delta, SSD1306_COLOR color);
  void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, SSD1306_COLOR color);
  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, SSD1306_COLOR color);
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, SSD1306_COLOR color);
  void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, SSD1306_COLOR color);

private:
  uint16_t CurrentX;
  uint16_t CurrentY;
  uint8_t Inverted;
  uint8_t Initialized;
  uint8_t SSD1306_Buffer[SSD1306_HEIGHT * SSD1306_WIDTH / 8];
  uint8_t vccstate = SSD1306_SWITCHCAPVCC;

  SPIClass* spi;
  int8_t mosiPin;
  int8_t clkPin;
  int8_t dcPin;
  int8_t csPin;
  int8_t rstPin;
};

#endif  // SSD1306_H
