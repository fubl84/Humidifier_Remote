/******************************************************************************
 * @file		  main.cpp
 *
 * @creator		Christoph Seiler
 * @created		2020
 *
 * @brief  		TODO: Short description of this module
 *
 * $Id$
 *
 * $Revision$
 *192.168.31.145
 MAC address: 8C:AA:B5:8C:23:FC

 ******************************************************************************/

/* ****************************************************************************/
/* ******************************** INCLUDES **********************************/
/* ****************************************************************************/

#include <Arduino.h>
#include <SPI.h>              // Display
#include <Adafruit_GFX.h>     // Display
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <WiFi.h>             // WiFi
#include <FirebaseESP32.h>    // Google Firebase
#include "FS.h"
#include <time.h>
#include <Adafruit_I2CDevice.h>
#include <ESPmDNS.h>          //OTA Update 1
#include <WiFiUdp.h>          //OTA Update 2
#include <ArduinoOTA.h>       //OTA Update 3


/* ****************************************************************************/
/* *************************** DEFINES AND MACROS *****************************/
/* ****************************************************************************/

/*__Select your hardware version__*/
// select one version and deselect the other versions
// #define AZ_TOUCH_ESP            // AZ-Touch ESP
#define AZ_TOUCH_MOD_SMALL_TFT  // AZ-Touch MOD with 2.4 inch TFT
//#define AZ_TOUCH_MOD_BIG_TFT    // AZ-Touch MOD with 2.8 inch TFT

//defines for I/O pins
#define TFT_CS   5
#define TFT_DC   4
#define TFT_LED  15
#define TFT_MOSI 23
#define TFT_CLK  18
#define TFT_RST  22
#define TFT_MISO 19
#define TFT_LED  15
#define TOUCH_CS 14
#ifdef AZ_TOUCH_ESP
// AZ-Touch ESP
#define TOUCH_IRQ 2
#else
// AZ-Touch MOD
#define TOUCH_IRQ 27
#endif
#define WIFI_NETWORK          "Hoppler"
#define WIFI_PASSWORD         "!12121987!09061984!"
#define WIFI_TIMEOUT_MS       20000
#define FIREBASE_HOST         "humidifieresp32.firebaseio.com"
#define FIREBASE_AUTH         "F8EsoAhdc016A00AIojgia26BsHCMinJL2hkulwB"

//define NTP server
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 0;

//define firebase data objects
FirebaseData firebaseData;    //firebase data for relais
FirebaseData firebaseData2;   //firebase data for sensors
FirebaseJsonArray arr1;       //array for relais
FirebaseJsonArray arr2;       //array for sensors
String pathRelais = "/Relais";
String pathSensors = "/Sensors";

//define colors and fonts for display
#define COLOR_WHITE 0xFFFF
#define COLOR_BLACK 0x0000
#define COLOR_RED   0xC000
#define COLOR_TEMP  0xB437
#define COLOR_HUM   0x359A
#define COLOR_ON    0x2DEC
#define COLOR_OFF   0x5AEB
#define COLOR_MID   0xFD20
#define COLOR_LOW   0xF800
#define COLOR_OK    0x015C

//bitmaps
// 'fan', 40x40px
const unsigned char fan [] PROGMEM = {
  0x3f, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xfe, 0x60, 0x00, 0x00, 0x00, 0x06, 0x67, 
	0x00, 0x3c, 0x00, 0x62, 0x6d, 0x83, 0xff, 0xc0, 0xd2, 0x69, 0x8f, 0x00, 0x70, 0x92, 0x6f, 0x38, 
	0x00, 0x1c, 0xf2, 0x60, 0x71, 0xfe, 0x06, 0x06, 0x60, 0xc7, 0x80, 0x43, 0x06, 0x61, 0x8c, 0x3c, 
	0x01, 0x86, 0x63, 0x18, 0xff, 0x00, 0xc6, 0x63, 0x30, 0xc1, 0x80, 0x66, 0x66, 0x60, 0x80, 0x80, 
	0x66, 0x64, 0xc0, 0x80, 0x80, 0x36, 0x6c, 0xc0, 0x81, 0x80, 0x32, 0x6d, 0x8e, 0xc1, 0x81, 0x92, 
	0x69, 0xbf, 0xc3, 0xfc, 0x9a, 0x69, 0x30, 0xef, 0xc6, 0xda, 0x79, 0x20, 0x7e, 0x02, 0xde, 0x79, 
	0x60, 0x26, 0x02, 0xce, 0x79, 0x60, 0x66, 0x02, 0xce, 0x79, 0x60, 0x3e, 0x06, 0xde, 0x69, 0x20, 
	0xff, 0x06, 0xda, 0x69, 0xbf, 0xc3, 0xcc, 0x9a, 0x6d, 0x84, 0xc1, 0xf9, 0x92, 0x6c, 0x81, 0x81, 
	0x01, 0xb2, 0x64, 0xc1, 0x81, 0x83, 0x36, 0x66, 0x61, 0x81, 0x83, 0x26, 0x62, 0x61, 0x81, 0x86, 
	0x66, 0x63, 0x38, 0xf3, 0x0c, 0xc6, 0x61, 0x9c, 0x3e, 0x39, 0x86, 0x60, 0xc7, 0x00, 0xf3, 0x86, 
	0x60, 0x61, 0xff, 0xc7, 0x06, 0x67, 0x38, 0x3e, 0x0c, 0x72, 0x6d, 0x8e, 0x00, 0x78, 0x92, 0x6d, 
	0x87, 0xff, 0xe0, 0x92, 0x67, 0x00, 0x7f, 0x00, 0xf2, 0x60, 0x00, 0x00, 0x00, 0x06, 0x7f, 0xff, 
	0xff, 0xff, 0xfe, 0x3f, 0xff, 0xff, 0xff, 0xfc
};
// 'heatmat', 40x40px
const unsigned char heatmat [] PROGMEM = {
  0x00, 0x80, 0x10, 0x02, 0x00, 0x01, 0x80, 0x30, 0x06, 0x00, 0x01, 0x80, 0x20, 0x0c, 0x00, 0x01, 
	0x80, 0x20, 0x0c, 0x00, 0x01, 0x80, 0x30, 0x06, 0x00, 0x00, 0xc0, 0x30, 0x06, 0x00, 0x00, 0xc0, 
	0x18, 0x02, 0x00, 0x00, 0xc0, 0x30, 0x06, 0x00, 0x01, 0x80, 0x30, 0x06, 0x00, 0x00, 0x00, 0x20, 
	0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x7e, 0x7c, 0x7c, 0x7c, 
	0x1e, 0x7e, 0x7c, 0x7c, 0xfe, 0x18, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 
	0x00, 0x00, 0x00, 0x0f, 0x3f, 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 
	0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 
	0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xcf, 0xcf, 0x8f, 0x80, 0x0f, 0xcf, 0xcf, 0xcf, 0x80, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x70, 
	0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x09, 0xf9, 
	0xf9, 0xf9, 0xf8, 0x01, 0xf1, 0xf1, 0xf1, 0xf0
};
// 'humidifier', 40x40px
const unsigned char humidifier [] PROGMEM = {
  0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 
	0x07, 0x00, 0x08, 0x00, 0x00, 0x0f, 0x00, 0x1c, 0x00, 0x00, 0x0f, 0x80, 0x3e, 0x00, 0x00, 0x1f, 
	0x80, 0x7a, 0x00, 0x00, 0x1f, 0xc0, 0x18, 0x00, 0x00, 0x3f, 0xc0, 0x18, 0x00, 0x00, 0x3f, 0xe0, 
	0x18, 0x00, 0x00, 0x7f, 0xe0, 0x18, 0x00, 0x00, 0x7f, 0xf0, 0x18, 0x00, 0x00, 0xff, 0xf0, 0x18, 
	0x00, 0x00, 0xff, 0xf8, 0x10, 0x30, 0x01, 0xff, 0xf8, 0x30, 0x78, 0x01, 0xff, 0xfc, 0x60, 0xfe, 
	0x03, 0xff, 0xfc, 0xe0, 0x30, 0x07, 0xff, 0xff, 0x80, 0x30, 0x07, 0xff, 0xff, 0x00, 0x20, 0x0f, 
	0xff, 0xff, 0x00, 0x60, 0x0f, 0xff, 0xff, 0x80, 0x40, 0x1f, 0xff, 0xff, 0x80, 0xc0, 0x1f, 0xff, 
	0xff, 0xc1, 0x80, 0x1f, 0xff, 0xff, 0xc7, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0x00, 0x3f, 0xff, 0xff, 
	0xe0, 0x00, 0x7f, 0xff, 0xff, 0xe0, 0x00, 0x7f, 0xff, 0xff, 0xe0, 0x00, 0x3f, 0xff, 0xff, 0xe0, 
	0x00, 0x3f, 0xff, 0xff, 0xc0, 0x00, 0x1f, 0xff, 0xff, 0xc0, 0x00, 0x1f, 0xff, 0xff, 0xc0, 0x00, 
	0x1f, 0xff, 0xff, 0x80, 0x00, 0x0f, 0xff, 0xff, 0x80, 0x00, 0x0f, 0xff, 0xff, 0x00, 0x00, 0x07, 
	0xff, 0xfe, 0x00, 0x00, 0x03, 0xff, 0xfc, 0x00, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x7f, 
	0xf0, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00
};
// 'power', 40x40px
const unsigned char power [] PROGMEM = {
  0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x0f, 0xff, 0xf0, 0x00, 0x00, 
	0x3f, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x80, 0x03, 0xff, 
	0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xe7, 0xff, 0xf0, 0x0f, 0xfc, 0x67, 
	0x1f, 0xf0, 0x1f, 0xf0, 0xe7, 0x0f, 0xf8, 0x1f, 0xe1, 0xe7, 0xc7, 0xf8, 0x3f, 0xc7, 0xe7, 0xe3, 
	0xfc, 0x3f, 0xcf, 0xe7, 0xf1, 0xfc, 0x3f, 0x8f, 0xe7, 0xf9, 0xfe, 0x7f, 0x9f, 0xe7, 0xf8, 0xfe, 
	0x7f, 0x1f, 0xe7, 0xfc, 0xfe, 0x7f, 0x3f, 0xe7, 0xfc, 0xff, 0xff, 0x3f, 0xe7, 0xfc, 0x7f, 0xff, 
	0x3f, 0xe7, 0xfc, 0x7f, 0xff, 0x3f, 0xe7, 0xfc, 0x7f, 0xff, 0x3f, 0xff, 0xfc, 0x7f, 0x7f, 0x3f, 
	0xff, 0xfc, 0xff, 0x7f, 0x1f, 0xff, 0xfc, 0xfe, 0x7f, 0x9f, 0xff, 0xf8, 0xfe, 0x3f, 0x8f, 0xff, 
	0xf9, 0xfe, 0x3f, 0x8f, 0xff, 0xf1, 0xfc, 0x3f, 0xc7, 0xff, 0xe3, 0xfc, 0x1f, 0xe3, 0xff, 0xc7, 
	0xfc, 0x1f, 0xf0, 0xff, 0x0f, 0xf8, 0x0f, 0xf8, 0x00, 0x1f, 0xf0, 0x07, 0xfe, 0x00, 0x7f, 0xf0, 
	0x07, 0xff, 0xf7, 0xff, 0xe0, 0x03, 0xff, 0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0xff, 0x80, 0x00, 
	0x7f, 0xff, 0xff, 0x00, 0x00, 0x3f, 0xff, 0xfc, 0x00, 0x00, 0x0f, 0xff, 0xf0, 0x00, 0x00, 0x01, 
	0xff, 0xc0, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00
};
// 'ventilator', 40x40px
const unsigned char ventilator [] PROGMEM = {
  0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x00, 0x03, 0xff, 0xfc, 0x00, 0x00, 
	0x07, 0xc0, 0x1c, 0x00, 0x00, 0x0f, 0x00, 0x0c, 0x00, 0x00, 0x1c, 0x00, 0x1c, 0x00, 0x00, 0x18, 
	0x00, 0x18, 0x00, 0x00, 0x38, 0x00, 0x38, 0x00, 0x00, 0x38, 0x00, 0x38, 0x00, 0x00, 0x38, 0x00, 
	0x70, 0x00, 0x1c, 0x38, 0x00, 0xe7, 0x80, 0x7f, 0x18, 0x00, 0xff, 0xe0, 0x7f, 0xdc, 0x01, 0xff, 
	0xf0, 0x61, 0xfe, 0x7f, 0xf0, 0x38, 0xe0, 0x7f, 0xff, 0xe0, 0x1c, 0xe0, 0x3f, 0xc3, 0xc0, 0x1c, 
	0xe0, 0x1f, 0x81, 0xe0, 0x0e, 0xe0, 0x07, 0x00, 0xe0, 0x0e, 0xe0, 0x06, 0x00, 0x60, 0x06, 0xe0, 
	0x06, 0x00, 0x70, 0x07, 0x60, 0x06, 0x00, 0x70, 0x07, 0x60, 0x06, 0x00, 0x60, 0x07, 0x70, 0x07, 
	0x00, 0xe0, 0x07, 0x70, 0x03, 0x80, 0xf8, 0x03, 0x38, 0x03, 0xc3, 0xfc, 0x07, 0x38, 0x03, 0xff, 
	0xfe, 0x07, 0x1c, 0x0f, 0xff, 0x7f, 0x87, 0x0f, 0x3f, 0x88, 0x3b, 0xee, 0x07, 0xff, 0x80, 0x18, 
	0xfe, 0x03, 0xe7, 0x00, 0x1c, 0x3c, 0x00, 0x0e, 0x00, 0x0c, 0x00, 0x00, 0x0e, 0x00, 0x0c, 0x00, 
	0x00, 0x1c, 0x00, 0x1c, 0x00, 0x00, 0x18, 0x00, 0x1c, 0x00, 0x00, 0x38, 0x00, 0x38, 0x00, 0x00, 
	0x38, 0x00, 0xf0, 0x00, 0x00, 0x38, 0x03, 0xe0, 0x00, 0x00, 0x1f, 0x7f, 0xc0, 0x00, 0x00, 0x1f, 
	0xff, 0x00, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00
};

/*____Calibrate Touchscreen_____*/
#define MINPRESSURE 10      // minimum required force for touch event
#define TS_MINX 370
#define TS_MINY 470
#define TS_MAXX 3700
#define TS_MAXY 3600

//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
//XPT2046_Touchscreen touch(TOUCH_CS);
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

/* ****************************************************************************/
/* ******************************** VARIABLES *********************************/
/* ****************************************************************************/

//setup timestamps and intervalls
const long timerCheckSensors = 3000;
const long timerSecond = 5000;
const long timerTimestamp = 30000;
unsigned long previousCheckSensors = 0;
unsigned long previousSecond = 0;
unsigned long previousTimestamp = 0;
unsigned long currentTime;
int localTime = 0;
int h=0;
int t=0;
int hour;
int minute;
int switchontime;
int X;
int Y;

//define states
bool statusVentilator=false;
bool statusHumidifier=false;
bool statusHeatmat=false;
bool statusStandby=false;
bool statusPumpCooling=true;
bool jsonVentilator;
bool jsonHumidifier;
bool jsonHeatmat;
bool jsonStandby;
bool timestamp=false;
bool touchPressed=false;

TS_Point p;

void subscribeFirebase(FirebaseData &data);
void initiateDisplay(void);
void updateDisplay(void);
void connectWiFi(void);
void initiateFirebase(void);
void detectButtons(void);
void buttonPressed(String whichButton);
void startOTA(void);
bool touchEvent();
void buttonVentilator(bool);
void buttonHumidifier(bool);
void buttonHeatmat(bool);
void buttonStandby(bool);

void setup()
{
  Serial.begin(115200);

  pinMode(TFT_LED, OUTPUT); // define as output for backlight control

  connectWiFi();

  startOTA();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  initiateFirebase();

  tft.begin();

  tft.setRotation(1);

  touch.begin();
  
  Serial.print("tftx ="); Serial.print(tft.width()); Serial.print(" tfty ="); Serial.println(tft.height());
  tft.fillScreen(ILI9341_BLACK);

  initiateDisplay();

  //sound configuration
  ledcSetup(0, 1E5, 12);
  ledcAttachPin(21, 0);
}

//////////////////////////////////////////////////
/////////////////MAIN LOOP////////////////////////
//////////////////////////////////////////////////

void loop()
{
  currentTime = millis();

  ArduinoOTA.handle();
 
  if((currentTime - previousCheckSensors) > timerCheckSensors)
    {
      
      previousCheckSensors = currentTime;
      
      subscribeFirebase(firebaseData2);

      updateDisplay();
    }
    else{}

    // check touchscreen for new events
  
  if(touchEvent() == true) 
  {
    X = p.y; Y = p.x;
    touchPressed = true;
    Serial.println(X);
    Serial.println(Y);
  } 
  else 
  {
    touchPressed = false;
    Serial.println("No touch.");
  }

  if(touchPressed == true)
  {
    detectButtons();
  }
  else{}
}

//////////////////////////////////////////////////
/////////////////LOOP FUNCTIONS///////////////////
//////////////////////////////////////////////////

void subscribeFirebase(FirebaseData &data)
{
  if ((currentTime - previousSecond) > timerSecond)
  {
    previousSecond = currentTime;
    if (Firebase.get(firebaseData2, pathSensors))
    {
      FirebaseJsonArray &arr2 = data.jsonArray();
      String arrStr;
      arr2.toString(arrStr, true);
      FirebaseJsonData &jsonData = data.jsonData();
      arr2.get(jsonData, 0);
      h = jsonData.intValue;
      //h = ((int)(h*10))/10.0;
      Serial.println(h);
      arr2.get(jsonData, 1);
      t = jsonData.intValue;
      //t = ((int)(t*10))/10.0;
      Serial.println(t);    
    }
    else{}
  }
  else{}
  if (Firebase.get(firebaseData2, pathRelais))
  {
    FirebaseJsonArray &arr1 = data.jsonArray();
    String arrStr;
    arr1.toString(arrStr, true);
    FirebaseJsonData &jsonData = data.jsonData();
    arr1.get(jsonData, 0);
    if(jsonData.stringValue == "true")
    {
      jsonVentilator = true;
    }
    else
    {
      jsonVentilator = false;
    }
    if(jsonVentilator != statusVentilator)
    {
      statusVentilator = jsonVentilator;
      if(jsonVentilator == true)
      {
        buttonVentilator(true);
      }
      else
      {
        buttonVentilator(false);
      }
    }
    else{}
    arr1.get(jsonData, 1);
    if(jsonData.stringValue == "true")
    {
      jsonHumidifier = true;
    }
    else
    {
      jsonHumidifier = false;
    }
    if(jsonHumidifier != statusHumidifier)
    {
      statusHumidifier = jsonHumidifier;
      if(jsonHumidifier == true)
      {
        buttonHumidifier(true);
      }
      else
      {
        buttonHumidifier(false);
      }
    }
    else{}
    arr1.get(jsonData, 2);
    if(jsonData.stringValue == "true")
    {
      jsonHeatmat = true;
    }
    else
    {
      jsonHeatmat = false;
    }
    if(jsonHeatmat != statusHeatmat)
    {
      statusHeatmat = jsonHeatmat;
      if(jsonHeatmat == true)
      {
        buttonHeatmat(true);
      }
      else
      {
        buttonHeatmat(false);
      }
    }
    else{}  
    arr1.get(jsonData, 3);
    if(jsonData.stringValue == "true")
    {
      jsonStandby = true;
    }
    else
    {
      jsonStandby = false;
    }
    if(jsonStandby != statusStandby)
    {
      statusStandby = jsonStandby;
      if(jsonStandby == true)
      {
        buttonStandby(true);
      }
      else
      {
        buttonStandby(false);
      }
    }
    else{}
  } 
}

void updateDisplay(void)
{
  //Humidity and temperature
  tft.fillRoundRect(16, 16, 136, 60, 10, COLOR_HUM);
  tft.fillRoundRect(168, 16, 136, 60, 10, COLOR_TEMP);
  tft.setFont(&FreeSansBold24pt7b);
  tft.setTextColor(COLOR_WHITE);
  tft.setCursor(32, 62);
  tft.print(h);
  tft.print(" %");
  tft.setCursor(188, 62);
  tft.print(t);
  tft.print(" C");
}

bool touchEvent()
{
  p = touch.getPoint();
  delay(1);
  #ifdef AZ_TOUCH_MOD_BIG_TFT
    // 2.8 inch TFT with yellow header
    p.x = map(p.x, TS_MINX, TS_MAXX, 320, 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, 240, 0);
  #else
    // 2.4 inch TFT with black header
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, 240);
  #endif
    if (p.z > MINPRESSURE) return true;
    return false;
}

void detectButtons(void)
{
  if(X > 90 && X < 150) //detect if 2. row is pressed
  {
    if(Y > 168)
    {
      buttonPressed("Humidifier");
    }
    else
    {
      buttonPressed("Ventilator");
    }
  }
  else
  {
    if(X < 224) //detect if 3.row
    {
      if(Y > 168)
      {
        buttonPressed("Power");
      }
      else
      {
        buttonPressed("Heatmat");
      }
    }
    else{}
  }
  Serial.println("Press detected, but no Button!");
}

void buttonPressed(String whichButton)
{
  if(whichButton == "Ventilator")
  {
    ledcWriteTone(0, 1000);
    delay(50);
    ledcWriteTone(0, 0);
    statusVentilator = !statusVentilator;
    if(statusVentilator == true)
    {
      buttonVentilator(true);
    }
    else
    {
      buttonVentilator(false);
    }
  }
  else{}
  if(whichButton == "Humidifier")
  {
    ledcWriteTone(0, 2000);
    delay(50);
    ledcWriteTone(0, 0);
    statusHumidifier = !statusHumidifier;
    if(statusHumidifier == true)
    {
      buttonHumidifier(true);
    }
    else
    {
      buttonHumidifier(false);
    }
  }
  else{}
  if(whichButton == "Heatmat")
  {
    ledcWriteTone(0, 4000);
    delay(50);
    ledcWriteTone(0, 0); 
    statusHeatmat = !statusHeatmat;
    if(statusHeatmat == true)
    {
      buttonHeatmat(true);
    }
    else
    {
      buttonHeatmat(false);
    }
  }
  else{}
  if(whichButton == "Power")
  {
    ledcWriteTone(0, 5000);
    delay(50);
    ledcWriteTone(0, 0); 
    statusStandby = !statusStandby;
    if(statusStandby == true)
    {
      buttonStandby(true);
    }
    else
    {
      buttonStandby(false);
    }
  }
  else{}
  
  arr1.clear();
  arr1.set("/[0]", statusVentilator);
  arr1.set("/[1]", statusHumidifier);
  arr1.set("/[2]", statusHeatmat);
  arr1.set("/[3]", statusStandby);
  if(Firebase.set(firebaseData, pathRelais, arr1))
  {
    Serial.println("Firebase relais updated!");
  }
  else
  {
    Serial.println("Firebase status update failed!");
    Serial.println("Reason: " + firebaseData.errorReason());
  }
}
//////////////////////////////////////////////////
////////////////SETUP FUNCTIONS///////////////////
//////////////////////////////////////////////////

void connectWiFi(void)
{
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
}

void initiateFirebase(void)
{
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  if(!Firebase.beginStream(firebaseData2, pathSensors))
  {
    Serial.println("Can't begin Firebase connection!");
    Serial.println("Reason: " + firebaseData2.errorReason());
  }
}

void initiateDisplay(void)
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextWrap(false);

  //Setup icons
  buttonVentilator(false);
  buttonHumidifier(false);  
  buttonHeatmat(false);  
  buttonStandby(false);
}

void buttonVentilator(bool active)
{
  if(active == true)
  {
    tft.fillRoundRect(16, 90, 136, 60, 10, COLOR_ON);
  }
  else
  {
    tft.fillRoundRect(16, 90, 136, 60, 10, COLOR_OFF);
  }
  tft.drawBitmap(64, 100, ventilator, 40, 40, COLOR_WHITE);
}

void buttonHumidifier(bool active)
{
  if(active == true)
  {
    tft.fillRoundRect(168, 90, 136, 60, 10, COLOR_ON);
  }
  else
  {
    tft.fillRoundRect(168, 90, 136, 60, 10, COLOR_OFF);
  }
  tft.drawBitmap(223, 100, humidifier, 40, 40, COLOR_WHITE);
}

void buttonHeatmat(bool active)
{
  if(active == true)
  {
    tft.fillRoundRect(16, 164, 136, 60, 10, COLOR_MID);
  }
  else
  {
    tft.fillRoundRect(16, 164, 136, 60, 10, COLOR_OFF);
  }
  tft.drawBitmap(66, 174, heatmat, 40, 40, COLOR_WHITE);
}

void buttonStandby(bool active)
{
  if(active == true)
  {
    tft.fillRoundRect(168, 164, 136, 60, 10, COLOR_RED);
  }
  else
  {
    tft.fillRoundRect(168, 164, 136, 60, 10, COLOR_OFF);
  }
  tft.drawBitmap(216, 174, power, 40, 40, COLOR_WHITE);
}

void startOTA(void)
{
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
}