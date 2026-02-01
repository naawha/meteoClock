#ifndef DISPLAY_COMPACT_H
#define DISPLAY_COMPACT_H

#include "display_common.h"

// Объявления внешних переменных и объектов (данные для отрисовки)
extern SensorData sensorData;
extern TimeData timeData;
extern uint8_t dotFlag;
extern char tempBuf[];
extern char humBuf[];
extern char presBuf[];
extern char co2Buf[];
extern char clockBuf[];
extern char dayBuf[];
extern char monthBuf[];

// ========== КОМПАКТНЫЙ СТИЛЬ: КРУПНЫЕ ЧАСЫ + КОМПАКТНЫЕ ДАТЧИКИ ==========
// Разметка: 40% экрана — часы (128 px), 60% — датчики (192 px), 2 ряда по 96 px
#define CLOCK_ZONE_H  128

#define QUAD_W     120   // половина ширины, ширина квадранта датчиков
#define QUAD_H     96   // половина высоты, высота квадранта датчиков
#define SENSOR_LABEL_MARGIN 20 // расстояние от верхнего края квадранта до подписи датчика
#define DATE_TOP_MARGIN 100 // расстояние от верхнего края экрана до подписи даты

inline void drawInterface() {
  tft.fillScreen(ILI9341_BLACK);

  tft.drawFastHLine(0, CLOCK_ZONE_H-2, TFT_W, ILI9341_WHITE);
  tft.drawFastHLine(0, CLOCK_ZONE_H-1, TFT_W, ILI9341_WHITE);

  // Температура — левый верхний квадрант
  tft.fillRect(0, CLOCK_ZONE_H, QUAD_W, CLOCK_ZONE_H+QUAD_H, ILI9341_RED);
  drawTempCelsiusLabel(CLOCK_ZONE_H + SENSOR_LABEL_MARGIN, 0, QUAD_W, ILI9341_WHITE, ILI9341_RED);

  // Влажность — правый верхний квадрант
  tft.fillRect(QUAD_W, CLOCK_ZONE_H, TFT_W, CLOCK_ZONE_H+QUAD_H, ILI9341_BLUE);
  drawDroplet(CLOCK_ZONE_H + SENSOR_LABEL_MARGIN, QUAD_W, TFT_W);  // верх капли, квадрант влажности

  // Давление — левый нижний квадрант
  tft.fillRect(0, CLOCK_ZONE_H + QUAD_H, QUAD_W, TFT_H, ILI9341_DARKGREEN);
  drawPressureArrows(CLOCK_ZONE_H + QUAD_H + SENSOR_LABEL_MARGIN, 0, QUAD_W);

  // CO2 — правый нижний квадрант
  tft.fillRect(QUAD_W, CLOCK_ZONE_H+QUAD_H, TFT_W, TFT_H, ILI9341_PURPLE);
  drawCO2Label(CLOCK_ZONE_H + QUAD_H + SENSOR_LABEL_MARGIN, QUAD_W, TFT_W, ILI9341_WHITE, ILI9341_PURPLE);
}

// ----- Свой шрифт для крупных цифр времени: 7×11 px, масштаб 4 → 28×44 -----
#define CLOCK_FONT_W    7
#define CLOCK_FONT_H    11
#define CLOCK_FONT_SCALE 4
#define CLOCK_GLYPH_GAP 2
#define CLOCK_TOP_MARGIN 26

// Битмапы: 0–9 и ':' (индекс 10). Стиль «цифровой дисплей»: двойные штрихи, ровные дуги.
// Ряд = 7 бит, бит 6 = левый пиксель.
const uint8_t PROGMEM clockFont[11][11] = {
  { 0x3E, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3E }, // 0 — овал, 2px борта
  { 0x0C, 0x1C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F }, // 1 — вертикальная полоса с каплей сверху
  { 0x3E, 0x63, 0x03, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x63, 0x7F }, // 2 — дуга сверху, наклон вниз
  { 0x3E, 0x63, 0x03, 0x03, 0x1E, 0x03, 0x03, 0x03, 0x63, 0x63, 0x3E }, // 3 — две дуги
  { 0x06, 0x0E, 0x1E, 0x36, 0x66, 0x66, 0x7F, 0x06, 0x06, 0x06, 0x06 }, // 4 — открытый верх, вертикаль справа
  { 0x7F, 0x60, 0x60, 0x60, 0x3E, 0x03, 0x03, 0x03, 0x63, 0x63, 0x3E }, // 5 — полка сверху, дуга снизу
  { 0x1E, 0x30, 0x60, 0x60, 0x3E, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3E }, // 6 — дуга сверху и снизу
  { 0x7F, 0x63, 0x03, 0x06, 0x06, 0x0C, 0x0C, 0x18, 0x18, 0x18, 0x18 }, // 7 — полка, наклон
  { 0x3E, 0x63, 0x63, 0x63, 0x3E, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3E }, // 8 — два овала
  { 0x3E, 0x63, 0x63, 0x63, 0x63, 0x3F, 0x03, 0x03, 0x06, 0x0C, 0x38 }, // 9 — дуга, открытый низ
  { 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00 }  // : — две точки 2px
};

inline void drawClockGlyph(uint8_t glyphIdx, int x, int y, uint8_t scale, uint16_t color, uint16_t bgColor) {
  for (int row = 0; row < CLOCK_FONT_H; row++) {
    uint8_t line = pgm_read_byte(&clockFont[glyphIdx][row]);
    for (int col = 0; col < CLOCK_FONT_W; col++) {
      uint16_t c = (line >> (6 - col)) & 1 ? color : bgColor;
      tft.fillRect(x + col * scale, y + row * scale, scale, scale, c);
    }
  }
}

inline void drawClockDigits(const char* s, int x, int y, uint16_t color, uint16_t bgColor) {
  int scale = CLOCK_FONT_SCALE;
  int step = CLOCK_FONT_W * scale + CLOCK_GLYPH_GAP;
  for (byte i = 0; s[i]; i++) {
    uint8_t idx = (s[i] == ':') ? 10 : (uint8_t)(s[i] - '0');
    if (idx <= 10) drawClockGlyph(idx, x, y, scale, color, bgColor);
    x += step;
  }
}

inline int getClockDigitsWidth(int charCount) {
  return charCount * (CLOCK_FONT_W * CLOCK_FONT_SCALE + CLOCK_GLYPH_GAP) - CLOCK_GLYPH_GAP;
}

inline void drawClock(byte hours, byte minutes, byte secs, uint8_t dotState) {
  sprintf(clockBuf, hours<10 ? (minutes<10 ? "0%d:0%d" : "0%d:%d") : (minutes<10 ? "%d:0%d" : "%d:%d"), hours, minutes);
  int totalW = getClockDigitsWidth(5);
  int x = (TFT_W - totalW) / 2;
  int y = CLOCK_TOP_MARGIN;
  int step = CLOCK_FONT_W * CLOCK_FONT_SCALE + CLOCK_GLYPH_GAP;
  uint16_t color = ILI9341_YELLOW;
  uint16_t bg = ILI9341_BLACK;

  drawClockGlyph(clockBuf[0] - '0', x, y, CLOCK_FONT_SCALE, color, bg);
  drawClockGlyph(clockBuf[1] - '0', x + step, y, CLOCK_FONT_SCALE, color, bg);
  if (dotState)
    drawClockGlyph(10, x + 2 * step, y, CLOCK_FONT_SCALE, color, bg);
  else
    tft.fillRect(x + 2 * step, y, CLOCK_FONT_W * CLOCK_FONT_SCALE, CLOCK_FONT_H * CLOCK_FONT_SCALE, bg);
  drawClockGlyph(clockBuf[3] - '0', x + 3 * step, y, CLOCK_FONT_SCALE, color, bg);
  drawClockGlyph(clockBuf[4] - '0', x + 4 * step, y, CLOCK_FONT_SCALE, color, bg);
}


inline void drawDate(byte day, byte month) {
  drawDayOfWeek(140, DATE_TOP_MARGIN, timeData.dayOfWeek);
  sprintf(dayBuf, day<10 ? "0%d" : "%d", day);
  printText(dayBuf, 17, DATE_TOP_MARGIN, 2, 13, ILI9341_WHITE, ILI9341_BLACK);
  printText(".", 17, DATE_TOP_MARGIN, 2, 15, ILI9341_WHITE, ILI9341_BLACK);
  
  if (month<10) {
    printText("0", 17, DATE_TOP_MARGIN, 2, 16, ILI9341_WHITE, ILI9341_BLACK);
    sprintf(monthBuf, "%d", month);
    printText(monthBuf, 17, DATE_TOP_MARGIN, 2, 17, ILI9341_WHITE, ILI9341_BLACK);
  } else {
    sprintf(monthBuf, "%d", month);
    printText(monthBuf, 17, DATE_TOP_MARGIN, 2, 16, ILI9341_WHITE, ILI9341_BLACK);
  }
}

inline void drawTemperature() {
  dtostrf(sensorData.temperature, 0, 0, tempBuf);
  if (sensorData.temperature >= 0) {
    printText("+", 34, CLOCK_ZONE_H + QUAD_H/2, 3, 0, ILI9341_YELLOW, ILI9341_RED);
    printText(tempBuf, 34, CLOCK_ZONE_H + QUAD_H/2, 3, 1, ILI9341_YELLOW, ILI9341_RED);
  } else {
    printText(tempBuf, 34, CLOCK_ZONE_H + QUAD_H/2, 3, 0, ILI9341_YELLOW, ILI9341_RED);
  }
}

inline void drawHumidity() {
  sprintf(humBuf, "%d%%", sensorData.humidity);
  printText(humBuf, 120+34, CLOCK_ZONE_H + QUAD_H/2, 3, 0, ILI9341_YELLOW, ILI9341_BLUE);
}

inline void drawPressure() {
  sprintf(presBuf, "%d", sensorData.pressure);
  printText(presBuf, 20, CLOCK_ZONE_H + QUAD_H + QUAD_H/2, 3, 0, ILI9341_YELLOW, ILI9341_DARKGREEN);
  printText("MM", 75, CLOCK_ZONE_H + QUAD_H + QUAD_H/2 + 7, 2, 0, ILI9341_YELLOW, ILI9341_DARKGREEN);
}

inline void drawCO2() {
  byte x = (sensorData.co2 >= 1000) ? 25 : 34;
  tft.fillRect(QUAD_W, CLOCK_ZONE_H + QUAD_H + QUAD_H/2, TFT_W, SYMBOL_H*3, ILI9341_PURPLE);
  sprintf(co2Buf, "%d", sensorData.co2);
  printText(co2Buf, 120 + x, CLOCK_ZONE_H + QUAD_H + QUAD_H/2, 3, 0, ILI9341_YELLOW, ILI9341_PURPLE);
}

inline void drawSensors() {
  // Температура - левый верхний квадрант
  drawTemperature();
  
  // Влажность - правый верхний квадрант
  drawHumidity();
  
  // Давление - левый нижний квадрант
  drawPressure();
  
  // CO2 - правый нижний квадрант
  drawCO2();
}


inline void clockTickDraw() {
  int totalW = getClockDigitsWidth(5);
  int x = (TFT_W - totalW) / 2;
  int y = CLOCK_TOP_MARGIN;
  int step = CLOCK_FONT_W * CLOCK_FONT_SCALE + CLOCK_GLYPH_GAP;
  int colonX = x + 2 * step;

  if (dotFlag)
    drawClockGlyph(10, colonX, y, CLOCK_FONT_SCALE, ILI9341_YELLOW, ILI9341_BLACK);
  else
    tft.fillRect(colonX, y, CLOCK_FONT_W * CLOCK_FONT_SCALE, CLOCK_FONT_H * CLOCK_FONT_SCALE, ILI9341_BLACK);
}

#endif // DISPLAY_COMPACT_H

