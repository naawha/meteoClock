#define ILI9341_BLACK       0x0000
#define ILI9341_DARKGREEN   0x03E0
#define ILI9341_PURPLE      0x780F
#define ILI9341_BLUE        0x001F
#define ILI9341_RED         0xF800
#define ILI9341_YELLOW      0xFFE0
#define ILI9341_WHITE       0xFFFF

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Wire.h>
#include "RTClib.h"
#include <GTimer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <MHZ19_uart.h>

#define TFT_CLK 13
#define TFT_MISO 12
#define TFT_MOSI 11
#define TFT_DC 10
#define TFT_CS 8
#define TFT_RST 9
#define TFT_LED 6

#define BRIGHTNESS_PIN A3

#define BTN_PIN 7
#define MAX_LED_LEVEL 255

#define SYMBOL_W 5
#define SYMBOL_H 7
#define SPACE_W 1
#define RESET_CLOCK 0
#define SENS_TIME 10000
#define MHZ_RX 2
#define MHZ_TX 3

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

RTC_DS3231 rtc;

Adafruit_BME280 bme;

MHZ19_uart mhz19;

GTimer<millis> clockTimer;
GTimer<millis> sensorsTimer;
GTimer<millis> dimmerTimer;

uint8_t dotFlag;
int8_t hrs, mins, secs, day, month;

byte screenBrightness;
float dispTemp;
byte dispHum;
int dispPres;
int dispCO2;
char tempBuf[6];
char humBuf[5];
char presBuf[6];
char co2Buf[6];
char clockBuf[6];
char dayBuf[3];
char monthBuf[3];

uint8_t button;
uint8_t button_flag;
uint8_t brightnessFlag;

void setup() {
  screenBrightness = MAX_LED_LEVEL;
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(TFT_LED, OUTPUT);
  analogWrite(TFT_LED, MAX_LED_LEVEL);


  clockTimer.setMode(GTMode::Interval);
  clockTimer.setTime(500);
  clockTimer.start();
  sensorsTimer.setMode(GTMode::Interval);
  sensorsTimer.setTime(SENS_TIME);
  sensorsTimer.start();
  dimmerTimer.setMode(GTMode::Timeout);
  dimmerTimer.setTime(SENS_TIME);
  dimmerTimer.start();

  tft.begin();
  bme.begin(0x76);
  mhz19.begin(MHZ_TX, MHZ_RX);
  mhz19.setAutoCalibration(false);
  tft.setCursor(0, 0);
  tft.setTextSize(3);

  delay(50);
  rtc.begin();
  if (RESET_CLOCK || rtc.lostPower())
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // temperature
                  Adafruit_BME280::SAMPLING_X1, // pressure
                  Adafruit_BME280::SAMPLING_X1, // humidity
                  Adafruit_BME280::FILTER_OFF   );
  DateTime now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();
  day = now.day();
  month = now.month();
  bme.takeForcedMeasurement();
  drawInterface();
  drawClock(hrs, mins, secs, 1);
  drawDate(day, month);
  readSensors();
  drawSensors();
}

void loop() {
  if (clockTimer) clockTick();
  if (sensorsTimer) {
    readSensors();
    drawSensors();
  }
  if (dimmerTimer) {
    brightnessFlag = 0;
    analogWrite(TFT_LED, screenBrightness);
  }

  button = !digitalRead(BTN_PIN);
  if (button && !button_flag) {
    button_flag = 1;
    brightnessFlag = 1;
  }
  if (!button && button_flag) {
    button_flag = 0;
    dimmerTimer.start();
  }

  if (brightnessFlag) {
    analogWrite(TFT_LED, MAX_LED_LEVEL);
  } else {
    analogWrite(TFT_LED, screenBrightness);
  }

}

void drawClock(byte hours, byte minutes, byte secs, uint8_t dotState) {
  sprintf(clockBuf, hours<10 ? (minutes<10 ? "0%d:0%d" : "0%d:%d") : (minutes<10 ? "%d:0%d" : "%d:%d"), hours, minutes);
  printText(clockBuf, 10, 10, 3, 0, ILI9341_YELLOW, true);
}

void drawDate(byte day, byte month) {
  DateTime now = rtc.now();
  drawDayOfWeek(140, 17, now.dayOfTheWeek());
  sprintf(dayBuf, day<10 ? "0%d" : "%d", day);
  printText(dayBuf, 17, SYMBOL_H+10, 2, 13, ILI9341_WHITE, true);
  printText(".", 17, SYMBOL_H+10, 2, 15, ILI9341_WHITE, true);
  if (month<10) {
    printText("0", 17, SYMBOL_H+10, 2, 16, ILI9341_WHITE, true);
    sprintf(monthBuf, "%d", month);
    printText(monthBuf, 17, SYMBOL_H+10, 2, 17, ILI9341_WHITE, true);
  } else {
    sprintf(monthBuf, "%d", month);
    printText(monthBuf, 17, SYMBOL_H+10, 2, 16, ILI9341_WHITE, true);
  }
}

void printText(const char* text, int x, int y, byte size, byte index, uint16_t COLOR, uint16_t FILL_COLOR) {
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

void clockTick() {
  dotFlag = !dotFlag;
  if (dotFlag) {
    secs++;
    if (secs > 59) {
      secs = 0;
      mins++;
      if (mins <= 59) drawClock(hrs, mins, secs, dotFlag);
    }
    if (mins > 59) {
      DateTime now = rtc.now();
      secs = now.second();
      mins = now.minute();
      hrs = now.hour();
      day = now.day();
      month = now.month();
      drawClock(hrs, mins, secs, dotFlag);
      drawDate(day, month);
      if (hrs > 23) hrs = 0;
    }
  }
  if (dotFlag) printText(":", 10, 10, 3, 2, ILI9341_YELLOW, true);
  else printText(" ", 10, 10, 3, 2, ILI9341_YELLOW, true);
}

void readSensors() {
  bme.takeForcedMeasurement();
  dispTemp = bme.readTemperature();
  dispHum = bme.readHumidity();
  dispPres = (float)bme.readPressure() * 0.00750062;
  dispCO2 = mhz19.getPPM();
  screenBrightness = map(analogRead(BRIGHTNESS_PIN), 0, 800, 1, 255);
}

void drawSensors() {
  drawTemperature();
  drawHumidity();
  drawPressure();
  drawCO2();
}

void drawTemperature() {
  dtostrf(dispTemp, 0, 0, tempBuf);
  if (dispTemp >= 0) {
    printText("+", 34, 44+69, 3, 0, ILI9341_YELLOW, ILI9341_RED);
    printText(tempBuf, 34, 44+69, 3, 1, ILI9341_YELLOW, ILI9341_RED);
  } else {
    printText(tempBuf, 34, 44+69, 3, 0, ILI9341_YELLOW, ILI9341_RED);
  }
}

void drawHumidity() {
  sprintf(humBuf, "%d%%", dispHum);
  printText(humBuf, 120+34, 44+69, 3, 0, ILI9341_YELLOW, ILI9341_BLUE);
}

void drawPressure() {
  sprintf(presBuf, "%d", dispPres);
  printText(presBuf, 20, 182+69, 3, 0, ILI9341_YELLOW, ILI9341_DARKGREEN);
  printText("MM", 75, 182+69+7, 2, 0, ILI9341_YELLOW, ILI9341_DARKGREEN);
}

void drawCO2() {
  byte x = (dispCO2 >= 1000) ? 25 : 34;
  tft.fillRect(120, 182+69, 240, SYMBOL_H*3, ILI9341_PURPLE);
  sprintf(co2Buf, "%d", dispCO2);
  printText(co2Buf, 120+x, 182+69, 3, 0, ILI9341_YELLOW, ILI9341_PURPLE);
}

void drawArrow(byte x, byte y) {
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

void drawInterface() {
  tft.fillScreen(ILI9341_BLACK);
  tft.drawFastHLine(0, 41, 320, ILI9341_WHITE);
  tft.drawFastHLine(0, 42, 320, ILI9341_WHITE);
  tft.drawFastHLine(0, 43, 320, ILI9341_WHITE);

  //температура
  tft.fillRect(0, 44, 120, 138, ILI9341_RED);
  printText("t", 38, 44+69-27, 3, 0, ILI9341_WHITE, ILI9341_RED);
  printText("C", 65, 44+69-27, 3, 0, ILI9341_WHITE, ILI9341_RED);
  printText("o", 54, 44+69-27-5, 2, 0, ILI9341_WHITE, ILI9341_RED);

  //влажность
  tft.fillRect(120, 44, 240, 138, ILI9341_BLUE);
  tft.fillCircle(180, 44+58, 5, ILI9341_WHITE);
  tft.fillTriangle(180, 44+43, 180-5, 44+58-3, 180+5, 44+58-3, ILI9341_WHITE);

  //давление
  tft.fillRect(0, 182, 120, 320, ILI9341_DARKGREEN);
  drawArrow(47, 225);
  drawArrow(63, 225);

  //CO2
  tft.fillRect(120, 182, 240, 320, ILI9341_PURPLE);
  printText("CO", 120+37, 182+69-27, 3, 0, ILI9341_WHITE, ILI9341_PURPLE);
  printText("2", 120+37+35, 182+69-18, 2, 0, ILI9341_WHITE, ILI9341_PURPLE);
}

void drawDayOfWeek(byte x, byte y, byte day) {
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

void drawCyrillicP(byte x, byte y) {
    tft.drawFastVLine(x, y, SYMBOL_H*2, ILI9341_WHITE);
    tft.drawFastVLine(x+1, y, SYMBOL_H*2, ILI9341_WHITE);
    tft.drawFastHLine(x, y, SYMBOL_W*2, ILI9341_WHITE);
    tft.drawFastHLine(x, y+1, SYMBOL_W*2, ILI9341_WHITE);
    tft.drawFastVLine(x+SYMBOL_W*2-1, y, SYMBOL_H*2, ILI9341_WHITE);
    tft.drawFastVLine(x+SYMBOL_W*2-2, y, SYMBOL_H*2, ILI9341_WHITE);
}

void drawCyrillict(byte x, byte y) {
    tft.drawFastHLine(x, y+5, SYMBOL_W*2, ILI9341_WHITE);
    tft.drawFastHLine(x, y+6, SYMBOL_W*2, ILI9341_WHITE);
    tft.drawFastVLine(x+SYMBOL_W-1, y+5, SYMBOL_H*2-5, ILI9341_WHITE);
    tft.drawFastVLine(x+SYMBOL_W, y+5, SYMBOL_H*2-5, ILI9341_WHITE);
}
