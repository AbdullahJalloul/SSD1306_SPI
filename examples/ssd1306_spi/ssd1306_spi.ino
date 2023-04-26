#include "SSD1306.h"

#define OLED_MOSI 13
#define OLED_CLK 14
#define OLED_DC 26
#define OLED_CS 33
#define OLED_RESET 25

SSD1306 oled(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

void setup() {
  oled.init();
  oled.drawRoundRect(0, 0, 127, 63, 8, SSD1306_WHITE);
  oled.setCursor(9, 10);
  oled.writeString("SSD1306 Test", Font_7x10, SSD1306_WHITE);
  oled.updateScreen();
}

void loop() {
  
}
