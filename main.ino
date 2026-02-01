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

// Структуры данных (должны быть объявлены до включения display_*.h)
struct SensorData {
  float temperature;
  byte humidity;
  int pressure;
  int co2;
  byte brightness;
};

struct TimeData {
  byte hours;
  byte minutes;
  byte seconds;
  byte day;
  byte month;
  byte dayOfWeek;
};

// Размеры символов (используются в display_*.h)
#define SYMBOL_W 5
#define SYMBOL_H 7
#define SPACE_W 1

// Размеры экрана и общие константы разметки (для display_*.h)
#define TFT_W      240
#define TFT_H      320

// ========== ВЫБОР СТИЛЯ ОТОБРАЖЕНИЯ ==========
// Раскомментируйте нужный стиль:
// #define DISPLAY_STYLE_ORIGINAL  1
#define DISPLAY_STYLE_COMPACT  1

// Включаем выбранный стиль
#if defined(DISPLAY_STYLE_ORIGINAL)
  #include "display_original.h"
#elif defined(DISPLAY_STYLE_COMPACT)
  #include "display_compact.h"
#else
  #error "Должен быть выбран хотя бы один стиль отображения!"
#endif

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

#define RESET_CLOCK 1
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

// Глобальные структуры данных
SensorData sensorData;
TimeData timeData;

uint8_t dotFlag;
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
  sensorData.brightness = MAX_LED_LEVEL;
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
  
  // Инициализация данных
  updateTimeData();
  updateSensorData();
  
  // Отрисовка интерфейса и данных
  drawInterface();
  drawClock(timeData.hours, timeData.minutes, timeData.seconds, 1);
  drawDate(timeData.day, timeData.month);
  drawSensors();
}

void loop() {
  if (clockTimer) clockTick();
  if (sensorsTimer) {
    updateSensorData();
    drawSensors();
  }
  if (dimmerTimer) {
    brightnessFlag = 0;
    analogWrite(TFT_LED, sensorData.brightness);
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
    analogWrite(TFT_LED, sensorData.brightness);
  }

}

void clockTick() {
  dotFlag = !dotFlag;
  if (dotFlag) {
    timeData.seconds++;
    if (timeData.seconds > 59) {
      timeData.seconds = 0;
      timeData.minutes++;
      if (timeData.minutes <= 59) {
        drawClock(timeData.hours, timeData.minutes, timeData.seconds, dotFlag);
      }
    }
    if (timeData.minutes > 59) {
      updateTimeData(); // Обновляем все время из RTC
      drawClock(timeData.hours, timeData.minutes, timeData.seconds, dotFlag);
      drawDate(timeData.day, timeData.month);
    }
  }
  clockTickDraw();
}

// ========== ФУНКЦИИ СБОРА ДАННЫХ ==========
void updateTimeData() {
  DateTime now = rtc.now();
  timeData.hours = now.hour();
  timeData.minutes = now.minute();
  timeData.seconds = now.second();
  timeData.day = now.day();
  timeData.month = now.month();
  timeData.dayOfWeek = now.dayOfTheWeek();
}

void updateSensorData() {
  bme.takeForcedMeasurement();
  sensorData.temperature = bme.readTemperature();
  sensorData.humidity = bme.readHumidity();
  sensorData.pressure = (float)bme.readPressure() * 0.00750062;
  sensorData.co2 = mhz19.getPPM();
  // Яркий свет → высокий ADC. map не ограничивает: при ADC > 800 получается >255, byte переполняется → минимум. Ограничиваем 1..255
  sensorData.brightness = constrain(map(analogRead(BRIGHTNESS_PIN), 0, 800, 1, 255), 1, 255);
}

