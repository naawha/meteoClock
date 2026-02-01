#ifndef DISPLAY_COMMON_H
#define DISPLAY_COMMON_H

// Общий заголовок для примитивов отрисовки.
// Подключается из display_compact.h и display_original.h.
// Ожидает: ILI9341_*, SYMBOL_W, SYMBOL_H, SPACE_W — определены в main.ino до включения display_*.h

extern Adafruit_ILI9341 tft;

// ----- Примитивы отрисовки (inline, чтобы не плодить символы в .ino) -----

inline void printText(const char* text, int x, int y, byte size, byte index, uint16_t COLOR, uint16_t FILL_COLOR) {
  byte len = strlen(text);
  tft.setCursor(
    x+index*SYMBOL_W*size+index*SPACE_W*size,
    y
  );
  tft.setTextColor(COLOR);
  tft.fillRect(
    x+index*SYMBOL_W*size+index*SPACE_W*size,
    y,
    SYMBOL_W*size*len+SPACE_W*size*(len-1),
    SYMBOL_H*size,
    FILL_COLOR
  );
  tft.setTextSize(size);
  tft.print(text);
}

// Подпись «температура, °C»: по вертикали от topY, по горизонтали — по центру между leftX и rightX.
inline void drawTempCelsiusLabel(int topY, int leftX, int rightX, uint16_t textColor, uint16_t fillColor) {
  const int labelW = 42;  // ширина «t»(15) + «°»(10) + «C»(15) в пикселях
  int x = leftX + (rightX - leftX - labelW) / 2;
  printText("t", x, topY, 3, 0, textColor, fillColor);
  printText("o", x+16, topY-5, 2, 0, textColor, fillColor);
  printText("C", x+27, topY, 3, 0, textColor, fillColor);
}

// Подпись «CO2»: по вертикали от topY, по горизонтали — по центру между leftX и rightX.
inline void drawCO2Label(int topY, int leftX, int rightX, uint16_t textColor, uint16_t fillColor) {
  const int labelW = 45;  // ширина «CO»(33) + смещение «2» вправо до конца (35+10)
  int x = leftX + (rightX - leftX - labelW) / 2;
  printText("CO", x, topY, 3, 0, textColor, fillColor);
  printText("2", x+35, topY+9, 2, 0, textColor, fillColor);
}

// Капля (влажность): по вертикали верх (остриё) в topY, по горизонтали — по центру между leftX и rightX.
inline void drawDroplet(int topY, int leftX, int rightX) {
  int x = (leftX + rightX) / 2;
  int y = topY + 15;  // центр круга под остриём треугольника (высота треугольника 15)
  tft.fillCircle(x, y, 5, ILI9341_WHITE);
  tft.fillTriangle(x, topY, x-5, y-3, x+5, y-3, ILI9341_WHITE);
}

inline void drawArrow(byte x, byte y) {
  tft.drawFastVLine(x+5, y, 20, ILI9341_WHITE);
  tft.drawFastVLine(x+6, y, 20, ILI9341_WHITE);
  tft.drawFastVLine(x+7, y, 20, ILI9341_WHITE);

  tft.drawLine(x+2, y+14, x+7, y+19, ILI9341_WHITE);
  tft.drawLine(x+1, y+14, x+6, y+19, ILI9341_WHITE);
  tft.drawLine(x+0, y+14, x+5, y+19, ILI9341_WHITE);

  tft.drawLine(x+10, y+14, x+5, y+19, ILI9341_WHITE);
  tft.drawLine(x+11, y+14, x+6, y+19, ILI9341_WHITE);
  tft.drawLine(x+12, y+14, x+7, y+19, ILI9341_WHITE);
}

// Две стрелки давления: по вертикали от topY, по горизонтали — по центру между leftX и rightX.
inline void drawPressureArrows(int topY, int leftX, int rightX) {
  const int arrowW = 13;  // ширина одной стрелки (x..x+12)
  const int gap = 6;
  const int totalW = arrowW + gap + arrowW;
  int x = leftX + (rightX - leftX - totalW) / 2;
  drawArrow(x, topY);
  drawArrow(x + arrowW + gap, topY);
}

inline void drawCyrillicP(byte x, byte y) {
  tft.drawFastVLine(x, y, SYMBOL_H*2, ILI9341_WHITE);
  tft.drawFastVLine(x+1, y, SYMBOL_H*2, ILI9341_WHITE);
  tft.drawFastHLine(x, y, SYMBOL_W*2, ILI9341_WHITE);
  tft.drawFastHLine(x, y+1, SYMBOL_W*2, ILI9341_WHITE);
  tft.drawFastVLine(x+SYMBOL_W*2-1, y, SYMBOL_H*2, ILI9341_WHITE);
  tft.drawFastVLine(x+SYMBOL_W*2-2, y, SYMBOL_H*2, ILI9341_WHITE);
}

inline void drawCyrillict(byte x, byte y) {
  tft.drawFastHLine(x, y+5, SYMBOL_W*2, ILI9341_WHITE);
  tft.drawFastHLine(x, y+6, SYMBOL_W*2, ILI9341_WHITE);
  tft.drawFastVLine(x+SYMBOL_W-1, y+5, SYMBOL_H*2-5, ILI9341_WHITE);
  tft.drawFastVLine(x+SYMBOL_W, y+5, SYMBOL_H*2-5, ILI9341_WHITE);
}

inline void drawDayOfWeek(byte x, byte y, byte day) {
  tft.fillRect(
    x,
    y,
    SYMBOL_W*2*2+SPACE_W*2,
    SYMBOL_H*2,
    ILI9341_BLACK
  );
  tft.setCursor(x, y);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  if (day == 0) {
    tft.print("Bc");
  }
  if (day == 1) {
    drawCyrillicP(x, y);

    tft.drawFastVLine(x+SYMBOL_W*2+SPACE_W*2, y+5, SYMBOL_H*2-5, ILI9341_WHITE);
    tft.drawFastVLine(x+SYMBOL_W*2+SPACE_W*2+1, y+5, SYMBOL_H*2-5, ILI9341_WHITE);
    tft.drawFastHLine(x+SYMBOL_W*2+SPACE_W*2, y+9, SYMBOL_W*2, ILI9341_WHITE);
    tft.drawFastHLine(x+SYMBOL_W*2+SPACE_W*2, y+10, SYMBOL_W*2, ILI9341_WHITE);
    tft.drawFastVLine(x+SYMBOL_W*2+SPACE_W*2+SYMBOL_W*2-1, y+5, SYMBOL_H*2-5, ILI9341_WHITE);
    tft.drawFastVLine(x+SYMBOL_W*2+SPACE_W*2+SYMBOL_W*2-2, y+5, SYMBOL_H*2-5, ILI9341_WHITE);
  }
  if (day == 2) {
    tft.print("B");
    drawCyrillict(x+SYMBOL_W*2+SPACE_W*2, y);
  }
  if (day == 3) {
    tft.print("Cp");
  }
  if (day == 4) {
    tft.drawFastVLine(x, y, SYMBOL_H, ILI9341_WHITE);
    tft.drawFastVLine(x+1, y, SYMBOL_H, ILI9341_WHITE);
    tft.drawFastHLine(x+2, y+SYMBOL_H, SYMBOL_W*2-2, ILI9341_WHITE);
    tft.drawFastHLine(x+2, y+SYMBOL_H+1, SYMBOL_W*2-2, ILI9341_WHITE);
    tft.drawFastVLine(x+SYMBOL_W*2-1, y, SYMBOL_H*2, ILI9341_WHITE);
    tft.drawFastVLine(x+SYMBOL_W*2-2, y, SYMBOL_H*2, ILI9341_WHITE);
    drawCyrillict(x+SYMBOL_W*2+SPACE_W*2, y);
  }
  if (day == 5) {
    drawCyrillicP(x, y);
    drawCyrillict(x+SYMBOL_W*2+SPACE_W*2, y);
  }
  if (day == 6) {
    tft.print("Co");
    tft.drawFastVLine(x+SYMBOL_W*2+SPACE_W*2, y+2, 5, ILI9341_WHITE);
    tft.drawFastVLine(x+SYMBOL_W*2+SPACE_W*2+1, y+2, 5, ILI9341_WHITE);
    tft.drawFastHLine(x+SYMBOL_W*2+SPACE_W*2+2, y, 6, ILI9341_WHITE);
    tft.drawFastHLine(x+SYMBOL_W*2+SPACE_W*2+2, y+1, 6, ILI9341_WHITE);
  }
}

#endif // DISPLAY_COMMON_H
