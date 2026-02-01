#ifndef DISPLAY_ORIGINAL_H
#define DISPLAY_ORIGINAL_H

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

// ========== ОРИГИНАЛЬНЫЙ СТИЛЬ: 4 КВАДРАНТА С ДАТЧИКАМИ ==========
// Разметка: шапка до 44, верхний ряд квадрантов 44–138, нижний 182–320

#define HEADER_BOTTOM  44 // расстояние от верхнего края экрана до верхнего края квадранта датчиков
#define QUAD_W     120   // половина ширины, ширина квадранта датчиков
#define QUAD_H     138   // половина высоты, высота квадранта датчиков
#define SENSOR_LABEL_MARGIN 40 // расстояние от верхнего края квадранта до подписи датчика

inline void drawInterface() {
  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(0, HEADER_BOTTOM - 3, TFT_W, 3, ILI9341_WHITE);

  tft.fillRect(0, HEADER_BOTTOM, QUAD_W, QUAD_H, ILI9341_RED);
  drawTempCelsiusLabel(HEADER_BOTTOM + SENSOR_LABEL_MARGIN, 0, QUAD_W, ILI9341_WHITE, ILI9341_RED);

  tft.fillRect(QUAD_W, HEADER_BOTTOM, TFT_W, QUAD_H, ILI9341_BLUE);
  drawDroplet(HEADER_BOTTOM + SENSOR_LABEL_MARGIN, QUAD_W, TFT_W);

  tft.fillRect(0, HEADER_BOTTOM + QUAD_H, QUAD_W, TFT_H, ILI9341_DARKGREEN);
  drawPressureArrows(HEADER_BOTTOM + QUAD_H + SENSOR_LABEL_MARGIN, 0, QUAD_W);

  tft.fillRect(QUAD_W, HEADER_BOTTOM + QUAD_H, TFT_W, TFT_H, ILI9341_PURPLE);
  drawCO2Label(HEADER_BOTTOM + QUAD_H + SENSOR_LABEL_MARGIN, QUAD_W, TFT_W, ILI9341_WHITE, ILI9341_PURPLE);
}

#define ORIG_CLOCK_X  10
#define ORIG_CLOCK_Y  10
#define ORIG_DATE_Y   (SYMBOL_H + 10)

inline void drawClock(byte hours, byte minutes, byte secs, uint8_t dotState) {
  sprintf(clockBuf, hours<10 ? (minutes<10 ? "0%d:0%d" : "0%d:%d") : (minutes<10 ? "%d:0%d" : "%d:%d"), hours, minutes);
  printText(clockBuf, ORIG_CLOCK_X, ORIG_CLOCK_Y, 3, 0, ILI9341_YELLOW, true);
}

inline void drawDate(byte day, byte month) {
  drawDayOfWeek(140, ORIG_CLOCK_Y + 7, timeData.dayOfWeek);
  sprintf(dayBuf, day<10 ? "0%d" : "%d", day);
  printText(dayBuf, 17, ORIG_DATE_Y, 2, 13, ILI9341_WHITE, true);
  printText(".", 17, ORIG_DATE_Y, 2, 15, ILI9341_WHITE, true);
  if (month<10) {
    printText("0", 17, ORIG_DATE_Y, 2, 16, ILI9341_WHITE, true);
    sprintf(monthBuf, "%d", month);
    printText(monthBuf, 17, ORIG_DATE_Y, 2, 17, ILI9341_WHITE, true);
  } else {
    sprintf(monthBuf, "%d", month);
    printText(monthBuf, 17, ORIG_DATE_Y, 2, 16, ILI9341_WHITE, true);
  }
}

inline void drawTemperature() {
  byte valY = HEADER_BOTTOM + QUAD_H/2;
  dtostrf(sensorData.temperature, 0, 0, tempBuf);
  if (sensorData.temperature >= 0) {
    printText("+", 34, HEADER_BOTTOM + QUAD_H/2, 3, 0, ILI9341_YELLOW, ILI9341_RED);
    printText(tempBuf, 34, HEADER_BOTTOM + QUAD_H/2, 3, 1, ILI9341_YELLOW, ILI9341_RED);
  } else {
    printText(tempBuf, 34, HEADER_BOTTOM + QUAD_H/2, 3, 0, ILI9341_YELLOW, ILI9341_RED);
  }
}

inline void drawHumidity() {
  sprintf(humBuf, "%d%%", sensorData.humidity);
  printText(humBuf, 120+34, HEADER_BOTTOM + QUAD_H/2, 3, 0, ILI9341_YELLOW, ILI9341_BLUE);
}

inline void drawPressure() {
  sprintf(presBuf, "%d", sensorData.pressure);
  printText(presBuf, 20, HEADER_BOTTOM + QUAD_H + QUAD_H/2, 3, 0, ILI9341_YELLOW, ILI9341_DARKGREEN);
  printText("MM", 75, HEADER_BOTTOM + QUAD_H + QUAD_H/2 + 7, 2, 0, ILI9341_YELLOW, ILI9341_DARKGREEN);
}

inline void drawCO2() {
  byte x = (sensorData.co2 >= 1000) ? 25 : 34;
  tft.fillRect(QUAD_W, HEADER_BOTTOM + QUAD_H + QUAD_H/2, TFT_W, SYMBOL_H*3, ILI9341_PURPLE);
  sprintf(co2Buf, "%d", sensorData.co2);
  printText(co2Buf, 120 + x, HEADER_BOTTOM + QUAD_H + QUAD_H/2, 3, 0, ILI9341_YELLOW, ILI9341_PURPLE);
}

inline void drawSensors() {
  drawTemperature();
  drawHumidity();
  drawPressure();
  drawCO2();
}

inline void clockTickDraw() {
  if (dotFlag) printText(":", ORIG_CLOCK_X, ORIG_CLOCK_Y, 3, 2, ILI9341_YELLOW, true);
  else printText(" ", ORIG_CLOCK_X, ORIG_CLOCK_Y, 3, 2, ILI9341_YELLOW, true);
}

#endif // DISPLAY_ORIGINAL_H

