#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include <Wire.h>
#include "DS3231.h"
#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp; // I2C
RTClib RTC;

#define TFT_CS        10
#define TFT_RST        8 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         9

#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11

#define PRINT_BUFF_SIZE  30
#define FLOAT_CONV_SIZE  6
#define REFRESH_COUNTER  10

typedef struct
{
  uint16_t year;
  uint8_t month;
  uint8_t day;

  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;

  uint8_t temp;
  uint8_t hum;
  float pres;
} DataStr;

DataStr dataStr;
char txtBuffer[PRINT_BUFF_SIZE]; // placehoder for sprintf
char floatBuffer[FLOAT_CONV_SIZE]; // placehoder for sprintf
uint8_t counter;

// For 1.44" and 1.8" TFT with ST7735 (including HalloWing) use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Temp/Hum sensor 
DHT dht(DHTPIN, DHTTYPE);

// sceen size
uint8_t tftHeight = 0;
uint8_t tftWidth = 0;

void setup(void) {
  counter = 0;
  
  Serial.begin(9600);

  // RTCf
  Wire.begin();

  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab

  tft.setRotation(3); // rotate 270
  tftHeight = tft.height();
  tftWidth = tft.width();
  tft.fillScreen(ST77XX_BLACK);

  SetupDHT();
  delay(100);
  SetupBMP280();
  delay(100);
  tftDrawOutline();
}

void loop() {
  
  GetTime(&dataStr);
  tftPrintTime(&dataStr, txtBuffer);

  if(counter >= REFRESH_COUNTER)
  {
    GetDate(&dataStr);
    tftPrintDate(&dataStr, txtBuffer);
    
    GetTempAndHum(&dataStr);
    tftPrintTemp(&dataStr, txtBuffer);
    tftPrintHum(&dataStr, txtBuffer);

    GetPressure(&dataStr);
    tftPrintPressure(&dataStr, txtBuffer, floatBuffer);
    
    counter = 0;
  }
  else
  {
    counter++;
  }
  
  delay(100);
}

void SetupDHT()
{
  dht.begin();  
}

void SetupBMP280()
{
  bmp.begin();
}

void tftDrawOutline(){
  tft.drawRect(1, 1, tftWidth-1, tftHeight-1, ST77XX_WHITE);
  tft.drawLine(1, tftHeight/3 , tftWidth-1, tftHeight/3, ST77XX_WHITE);
  tft.drawLine(1, tftHeight/(float)1.5 , tftWidth-1, tftHeight/(float)1.5, ST77XX_WHITE);
  tft.drawLine(tftWidth/2, tftHeight/(float)1.5, tftWidth/2, tftHeight/3, ST77XX_WHITE);
}

void tftPrintPressure(DataStr* dStr, char *buffer, char *floatBuffer){
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(25, 100);
  ClearBuffer(buffer);
  ClearBuffer(floatBuffer);
  dtostrf((dStr->pres)/(float)100, 4, 1, floatBuffer); // 4 whole numbers, 1 precision
  sprintf(buffer, "%s %s", floatBuffer, "hPa");
  tft.println(buffer);
}

void tftPrintTemp(DataStr* dStr, char *buffer){
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(3);
  tft.setCursor(15, 55);
  ClearBuffer(buffer);
  sprintf(buffer, "%d%c", dStr->temp, 'C');
  tft.println(buffer);
}

void tftPrintHum(DataStr* dStr, char *buffer){
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(3);
  tft.setCursor(95, 55);
  ClearBuffer(buffer);
  sprintf(buffer, "%d%c", dStr->hum, '%');
  tft.println(buffer);
}

void tftPrintTime(DataStr* dStr, char *buffer){
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setCursor(30, 10);
  ClearBuffer(buffer);
  sprintf(buffer, "%02d:%02d:%02d", dStr->hours, dStr->minutes, dStr->seconds);
  tft.println(buffer);
}

void tftPrintDate(DataStr* dStr, char *buffer){
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor(50, 30);
  ClearBuffer(buffer);
  sprintf(buffer, "%d-%02d-%02d", dStr->year, dStr->month, dStr->day);
  tft.println(buffer);
}


void GetTime(DataStr* dStr){
  DateTime now = RTC.now();
  dStr->hours = now.hour();
  dStr->minutes = now.minute();
  dStr->seconds = now.second();
}

void GetDate(DataStr* dStr){
  DateTime now = RTC.now();
  dStr->year = now.year();
  dStr->month = now.month();
  dStr->day = now.day();
}

void GetTempAndHum(DataStr* dStr){
  dStr->temp = dht.readTemperature();
  dStr->hum = dht.readHumidity();
}

void GetPressure(DataStr* dStr){
  dStr->pres = bmp.readPressure();
}

void ClearBuffer(char *ptrPrintBuffer)
{
  memset(ptrPrintBuffer, 0, sizeof(ptrPrintBuffer)); 
}

