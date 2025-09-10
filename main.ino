/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#define ILI9341_BLACK       0x0000  ///<   0,   0,   0
#define ILI9341_NAVY        0x000F  ///<   0,   0, 123
#define ILI9341_DARKGREEN   0x03E0  ///<   0, 125,   0
#define ILI9341_DARKCYAN    0x03EF  ///<   0, 125, 123
#define ILI9341_MAROON      0x7800  ///< 123,   0,   0
#define ILI9341_PURPLE      0x780F  ///< 123,   0, 123
#define ILI9341_OLIVE       0x7BE0  ///< 123, 125,   0
#define ILI9341_LIGHTGREY   0xC618  ///< 198, 195, 198
#define ILI9341_DARKGREY    0x7BEF  ///< 123, 125, 123
#define ILI9341_BLUE        0x001F  ///<   0,   0, 255
#define ILI9341_GREEN       0x07E0  ///<   0, 255,   0
#define ILI9341_CYAN        0x07FF  ///<   0, 255, 255
#define ILI9341_RED         0xF800  ///< 255,   0,   0
#define ILI9341_MAGENTA     0xF81F  ///< 255,   0, 255
#define ILI9341_YELLOW      0xFFE0  ///< 255, 255,   0
#define ILI9341_WHITE       0xFFFF  ///< 255, 255, 255
#define ILI9341_ORANGE      0xFD20  ///< 255, 165,   0
#define ILI9341_GREENYELLOW 0xAFE5  ///< 173, 255,  41
#define ILI9341_PINK        0xFC18  ///< 255, 130, 198

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <Wire.h>
#include "RTClib.h"
#include <GTimer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <MHZ19_uart.h>

// For the Adafruit shield, these are the default.
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
#define MIN_LED_LEVEL 1

#define SYMBOL_W 5
#define SYMBOL_H 7
#define SPACE_W 1
#define RESET_CLOCK 0       // сброс часов на время загрузки прошивки (для модуля с несъёмной батарейкой). Не забудь поставить 0 и прошить ещё раз!
#define SENS_TIME 10000     // время в мс
#define MHZ_RX 2
#define MHZ_TX 3

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

RTC_DS3231 rtc;
DateTime now;

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;

MHZ19_uart mhz19;

GTimer<millis> clockTimer;
GTimer<millis> sensorsTimer;
GTimer<millis> dimmerTimer;

boolean dotFlag;
int8_t hrs, mins, secs, day, month;

byte screenBrightness;
float dispTemp;
byte dispHum;
int dispPres;
int dispCO2;
int pressureArray[6];

boolean button;
boolean button_flag;
boolean brightnessFlag;

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

  Serial.begin(9600);
  Serial.println("ILI9341 Test!");

  tft.begin();
  bme.begin(0x76);
  mhz19.begin(MHZ_TX, MHZ_RX);
  mhz19.setAutoCalibration(false);
  tft.setCursor(0, 0);
  tft.setTextSize(3);

  delay(50);
  if (rtc.begin()) {
    Serial.println(F("OK"));
  } else {
    Serial.println(F("ERROR"));
  }

  if (RESET_CLOCK || rtc.lostPower())
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // temperature
                  Adafruit_BME280::SAMPLING_X1, // pressure
                  Adafruit_BME280::SAMPLING_X1, // humidity
                  Adafruit_BME280::FILTER_OFF   );
  now = rtc.now();
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
  if (button == 1 && button_flag == 0) {
    button_flag = 1;
    brightnessFlag = 1;
  }

  if (button == 0 && button_flag == 1) {
    button_flag = 0;
    dimmerTimer.start();
  }

  if (brightnessFlag) {
    analogWrite(TFT_LED, MAX_LED_LEVEL);
  } else {
    analogWrite(TFT_LED, screenBrightness);
  }

}

void drawClock(byte hours, byte minutes, byte secs, boolean dotState) {
  String clock = String("");
  if (hours < 10) {
    clock += String(0);
  }
  clock += String(hours);
  clock += String(":");
  if (minutes < 10) {
    clock += String(0);
  }
  clock += String(minutes);

  printText(clock, 10, 10, 3, 0, ILI9341_YELLOW, true);
}

void drawDate(byte day, byte month) {
  drawDayOfWeek(140, 17, now.dayOfTheWeek());
  if (day<10) {
    printText("0"+String(day), 17, SYMBOL_H+10, 2, 13, ILI9341_WHITE, true);
  } else {
    printText(String(day), 17, SYMBOL_H+10, 2, 13, ILI9341_WHITE, true);
  }
  printText(".", 17, SYMBOL_H+10, 2, 15, ILI9341_WHITE, true);
  if (month<10) {
    printText("0", 17, SYMBOL_H+10, 2, 16, ILI9341_WHITE, true);
    printText(String(month), 17, SYMBOL_H+10, 2, 17, ILI9341_WHITE, true);
  } else {
    printText(String(month), 17, SYMBOL_H+10, 2, 16, ILI9341_WHITE, true);
  }
}

void printText(String text, int x, int y, byte size, byte index, uint16_t COLOR, uint16_t FILL_COLOR) {
  tft.setCursor(
    x+index*SYMBOL_W*size+index*SPACE_W*size,
    y
  );
  tft.setTextColor(COLOR);
  tft.fillRect(
    x+index*SYMBOL_W*size+index*SPACE_W*size,
    y,
    SYMBOL_W*size*text.length()+SPACE_W*size*(text.length()-1),
    SYMBOL_H*size,
    FILL_COLOR
  );
  tft.setTextSize(size);
  tft.print(text);
}

void clockTick() {
  dotFlag = !dotFlag;
  if (dotFlag) {          // каждую секунду пересчёт времени
    secs++;
    if (secs > 59) {      // каждую минуту
      secs = 0;
      mins++;
      if (mins <= 59) drawClock(hrs, mins, secs, dotFlag);
    }
    if (mins > 59) {      // каждый час
      now = rtc.now();
      secs = now.second();
      mins = now.minute();
      hrs = now.hour();
      day = now.day();
      month = now.month();
      drawClock(hrs, mins, secs, dotFlag);
      drawDate(day, month);
      if (hrs > 23) {
        hrs = 0;
      }
    }
  }
  if (dotFlag) printText(String(":"), 10, 10, 3, 2, ILI9341_YELLOW, true);
  else printText(String(" "), 10, 10, 3, 2, ILI9341_YELLOW, true);
}

void readSensors() {
  bme.takeForcedMeasurement();
  dispTemp = bme.readTemperature();
  Serial.print("Temperature: ");
  Serial.println(dispTemp);
  dispHum = bme.readHumidity();
  Serial.print("Humidity: ");
  Serial.println(dispHum);
  dispPres = (float)bme.readPressure() * 0.00750062;
  Serial.print("Pressure: ");
  Serial.println(dispPres);
  dispCO2 = mhz19.getPPM();
  Serial.print("CO2: ");
  Serial.println(dispCO2);
  Serial.print("Brightness sensor: ");
  Serial.println(analogRead(BRIGHTNESS_PIN));
  screenBrightness = map(analogRead(BRIGHTNESS_PIN), 0, 800, 1, 255);
  Serial.print("Brightness map: ");
  Serial.println(screenBrightness);
}

void drawSensors() {
  drawTemperature();
  drawHumidity();
  drawPressure();
  drawCO2();
}

void drawTemperature() {
  String temp = String(dispTemp, 0);
  if (dispTemp >= 0) {
    printText("+", 34, 44+69, 3, 0, ILI9341_YELLOW, ILI9341_RED);
    printText(temp, 34, 44+69, 3, 1, ILI9341_YELLOW, ILI9341_RED);
  } else {
    printText(temp, 34, 44+69, 3, 0, ILI9341_YELLOW, ILI9341_RED);
  }

}

void drawHumidity() {
  printText(String(dispHum) + "%", 120+34, 44+69, 3, 0, ILI9341_YELLOW, ILI9341_BLUE);
}

void drawPressure() {
  printText(String(dispPres), 20, 182+69, 3, 0, ILI9341_YELLOW, ILI9341_DARKGREEN);
  printText("MM", 75, 182+69+7, 2, 0, ILI9341_YELLOW, ILI9341_DARKGREEN);
}

void drawCO2() {
  int x = 34;
  if (dispCO2 >= 1000) {
    x = 25;
  }
  tft.fillRect(
    120,
    182+69,
    240,
    SYMBOL_H*3,
    ILI9341_PURPLE
  );
  printText(String(dispCO2), 120+x, 182+69, 3, 0, ILI9341_YELLOW, ILI9341_PURPLE);
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

void storePressure() {
  for (byte i = 0; i < 5; i++) {
    pressureArray[i] = pressureArray[i + 1];
  }
  pressureArray[5] = dispPres;

}
