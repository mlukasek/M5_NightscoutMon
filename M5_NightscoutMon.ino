/*  M5Stack Nightscout monitor
    Copyright (C) 2018, 2019 Martin Lukasek <martin@lukasek.cz>
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>. 
    
    This software uses some 3rd party libraries:
    IniFile by Steve Marple <stevemarple@googlemail.com> (GNU LGPL v2.1)
    ArduinoJson by Benoit BLANCHON (MIT License) 
*/

#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include "time.h"
// #include <util/eu_dst.h>
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include "Free_Fonts.h"
#include "IniFile.h"

struct tConfig {
  char url[32];
  char bootPic[64];
  char userName[32];
  int timeZone = 3600; // time zone offset in hours, must be corrected for internatinal use and DST
  int dst = 0; // DST time offset in hours, must be corrected for internatinal use and DST
  int show_mgdl = 0; 
  float yellow_low = 4.5;
  float yellow_high = 9;
  float red_low = 3.9;
  float red_high = 11;
  float snd_warning = 3.7;
  float snd_alarm = 3.0;
  uint8_t brightness1, brightness2, brightness3;
  char wlan1ssid[32];
  char wlan1pass[32];
  char wlan2ssid[32];
  char wlan2pass[32];
  char wlan3ssid[32];
  char wlan3pass[32];
} ;

tConfig cfg;

extern const unsigned char gImage_logoM5[];
extern const unsigned char m5stack_startup_music[];

const char* ntpServer = "pool.ntp.org";

#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

WiFiMulti WiFiMulti;
unsigned long msCount;
static uint8_t lcdBrightness = 10;
static char *iniFilename = "/M5NS.INI";

DynamicJsonDocument JSONdoc(16384);
float last10sgv[10];
int wasError = 0;

void startupLogo() {
    static uint8_t brightness, pre_brightness;
    uint32_t length = strlen((char*)m5stack_startup_music);
    M5.Lcd.setBrightness(0);
    if(cfg.bootPic[0]==0)
      M5.Lcd.pushImage(0, 0, 320, 240, (uint16_t *)gImage_logoM5);
    else
      M5.Lcd.drawJpgFile(SD, cfg.bootPic);
    M5.Lcd.setBrightness(100);
    M5.update();
    M5.Speaker.playMusic(m5stack_startup_music,25000);
    delay(1000);
    /*
    for(int i=0; i<length; i++) {
        dacWrite(SPEAKER_PIN, m5stack_startup_music[i]>>2);
        delayMicroseconds(40);
        brightness = (i/157);
        if(pre_brightness != brightness) {
            pre_brightness = brightness;
            M5.Lcd.setBrightness(brightness);
        }
    }

    for(int i=255; i>=0; i--) {
        M5.Lcd.setBrightness(i);
        if(i<=32) {
            // dacWrite(SPEAKER_PIN, i);
        }
        delay(2);
    }
    */ 
    M5.Lcd.fillScreen(BLACK);
    delay(800);
    for(int i=0; i>=100; i++) {
        M5.Lcd.setBrightness(i);
        delay(2);
    }
}

void printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    M5.Lcd.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  M5.Lcd.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void sndAlarm() {
    M5.Speaker.setVolume(8);
    // M5.Speaker.beep(); //beep
    M5.Speaker.update();
    for(int j=0; j<6; j++) {
      M5.Speaker.tone(660, 400);
      for(int i=0; i<600; i++) {
        delay(1);
        M5.update();
      }
    }
    //M5.Speaker.playMusic(m5stack_startup_music, 25000);        
    M5.Speaker.mute();
    M5.Speaker.update();
}

void sndWarning() {
  M5.Speaker.setVolume(4);
  M5.Speaker.update();
  M5.Speaker.tone(3000, 100);
  for(int i=0; i<400; i++) {
    delay(1);
    M5.update();
  }
  M5.Speaker.tone(3000, 100);
  for(int i=0; i<400; i++) {
    delay(1);
    M5.update();
  }
  M5.Speaker.tone(3000, 100);
  for(int i=0; i<400; i++) {
    delay(1);
    M5.update();
  }
  M5.Speaker.mute();
  M5.Speaker.update();
}

void buttons_test() {
  if(M5.BtnA.wasPressed()) {
      // M5.Lcd.printf("A");
      Serial.printf("A");
      // sndAlarm();
      if(lcdBrightness==cfg.brightness1) 
        lcdBrightness = cfg.brightness2;
      else
        if(lcdBrightness==cfg.brightness2) 
          lcdBrightness = cfg.brightness3;
        else
          lcdBrightness = cfg.brightness1;
      M5.Lcd.setBrightness(lcdBrightness);
  }
  if(M5.BtnB.wasPressed()) {
      // M5.Lcd.printf("B");
      Serial.printf("B");
      // sndWarning();
  } 
  if(M5.BtnC.wasPressed()) {
      // M5.Lcd.printf("C");
      Serial.printf("C");
      M5.setWakeupButton(BUTTON_B_PIN);
      M5.powerOFF();
  } 
}

void wifi_connect() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.println("WiFi connect start");
    M5.Lcd.println("WiFi connect start");

    // We start by connecting to a WiFi network
    if(cfg.wlan1ssid[0]!=0)
      WiFiMulti.addAP(cfg.wlan1ssid, cfg.wlan1pass);
    if(cfg.wlan2ssid[0]!=0)
      WiFiMulti.addAP(cfg.wlan2ssid, cfg.wlan2pass);
    if(cfg.wlan3ssid[0]!=0)
      WiFiMulti.addAP(cfg.wlan3ssid, cfg.wlan3pass);

    Serial.println();
    M5.Lcd.println("");
    Serial.print("Wait for WiFi... ");
    M5.Lcd.print("Wait for WiFi... ");

    while(WiFiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        M5.Lcd.print(".");
        delay(500);
    }

    Serial.println("");
    M5.Lcd.println("");
    Serial.println("WiFi connected");
    M5.Lcd.println("WiFi connected");
    Serial.println("IP address: ");
    M5.Lcd.println("IP address: ");
    Serial.println(WiFi.localIP());
    M5.Lcd.println(WiFi.localIP());

    configTime(cfg.timeZone, cfg.dst, ntpServer);
    delay(1000);
    printLocalTime();

    Serial.println("Connection done");
    M5.Lcd.println("Connection done");
}

void printErrorMessage(uint8_t e, bool eol = true)
{
  switch (e) {
  case IniFile::errorNoError:
    Serial.print("no error");
    break;
  case IniFile::errorFileNotFound:
    Serial.print("file not found");
    break;
  case IniFile::errorFileNotOpen:
    Serial.print("file not open");
    break;
  case IniFile::errorBufferTooSmall:
    Serial.print("buffer too small");
    break;
  case IniFile::errorSeekError:
    Serial.print("seek error");
    break;
  case IniFile::errorSectionNotFound:
    Serial.print("section not found");
    break;
  case IniFile::errorKeyNotFound:
    Serial.print("key not found");
    break;
  case IniFile::errorEndOfFile:
    Serial.print("end of file");
    break;
  case IniFile::errorUnknownError:
    Serial.print("unknown error");
    break;
  default:
    Serial.print("unknown error value");
    break;
  }
  if (eol)
    Serial.println();
}

void readConfiguration() {
  const size_t bufferLen = 80;
  char buffer[bufferLen];
    
  IniFile ini(iniFilename); //(uint8_t)"/M5NS.INI"
  
  if (!ini.open()) {
    Serial.print("Ini file ");
    Serial.print(iniFilename);
    Serial.println(" does not exist");
    M5.Lcd.println("No INI file");
    // Cannot do anything else
    while (1)
      ;
  }
  Serial.println("Ini file exists");

  // Check the file is valid. This can be used to warn if any lines
  // are longer than the buffer.
  if (!ini.validate(buffer, bufferLen)) {
    Serial.print("ini file ");
    Serial.print(ini.getFilename());
    Serial.print(" not valid: ");
    printErrorMessage(ini.getError());
    // Cannot do anything else
    M5.Lcd.println("Bad INI file");
    while (1)
      ;
  }
  
  // Fetch a value from a key which is present
  if (ini.getValue("config", "nightscout", buffer, bufferLen)) {
    Serial.print("section 'config' has an entry 'nightscout' with value ");
    Serial.println(buffer);
    strcpy(cfg.url, buffer);
  }
  else {
    Serial.print("Could not read 'nightscout' from section 'config', error was ");
    printErrorMessage(ini.getError());
    M5.Lcd.println("No URL in INI file");
    while (1)
      ;
  }

  if (ini.getValue("config", "bootpic", buffer, bufferLen)) {
    Serial.print("bootpic = ");
    Serial.println(buffer);
    strcpy(cfg.bootPic, buffer);
  }
  else {
    Serial.println("NO bootpic");
    cfg.bootPic[0]=0;
  }
  
  if (ini.getValue("config", "name", buffer, bufferLen)) {
    Serial.print("name = ");
    Serial.println(buffer);
    strcpy(cfg.userName, buffer);
  }
  else {
    Serial.println("NO user name");
    strcpy(cfg.userName, " ");
  }
  
  if (ini.getValue("config", "time_zone", buffer, bufferLen)) {
    Serial.print("time_zone = ");
    cfg.timeZone = atoi(buffer);
    Serial.println(cfg.timeZone);
  }
  else {
    Serial.println("NO time zone defined -> Central Europe");
    cfg.timeZone = 3600;
  }

  if (ini.getValue("config", "dst", buffer, bufferLen)) {
    Serial.print("dst = ");
    cfg.dst = atoi(buffer);
    Serial.println(cfg.dst);
  }
  else {
    Serial.println("NO DST defined -> summer time");
    cfg.dst = 3600;
  }

  if (ini.getValue("config", "show_mgdl", buffer, bufferLen)) {
    Serial.print("show_mgdl = ");
    cfg.show_mgdl = atoi(buffer);
    Serial.println(cfg.show_mgdl);
  }
  else {
    Serial.println("NO show_mgdl defined -> 0 = show mmol/L");
    cfg.show_mgdl = 0;
  }


  if (ini.getValue("config", "yellow_low", buffer, bufferLen)) {
    Serial.print("yellow_low = ");
    cfg.yellow_low = atof(buffer);
    if( cfg.show_mgdl )
      cfg.yellow_low /= 18.0;
    Serial.println(cfg.yellow_low);
  }
  else {
    Serial.println("NO yellow_low defined");
    cfg.yellow_low = 4.5;
  }

  if (ini.getValue("config", "yellow_high", buffer, bufferLen)) {
    Serial.print("yellow_high = ");
    cfg.yellow_high = atof(buffer);
    if( cfg.show_mgdl )
      cfg.yellow_high /= 18.0;
    Serial.println(cfg.yellow_high);
  }
  else {
    Serial.println("NO yellow_high defined");
    cfg.yellow_high = 9.0;
  }

  if (ini.getValue("config", "red_low", buffer, bufferLen)) {
    Serial.print("red_low = ");
    cfg.red_low = atof(buffer);
    if( cfg.show_mgdl )
      cfg.red_low /= 18.0;
    Serial.println(cfg.red_low);
  }
  else {
    Serial.println("NO red_low defined");
    cfg.red_low = 3.9;
  }

  if (ini.getValue("config", "red_high", buffer, bufferLen)) {
    Serial.print("red_high = ");
    cfg.red_high = atof(buffer);
    if( cfg.show_mgdl )
      cfg.red_high /= 18.0;
    Serial.println(cfg.red_high);
  }
  else {
    Serial.println("NO red_high defined");
    cfg.red_high = 9.0;
  }

  if (ini.getValue("config", "snd_warning", buffer, bufferLen)) {
    Serial.print("snd_warning = ");
    cfg.snd_warning = atof(buffer);
    if( cfg.show_mgdl )
      cfg.snd_warning /= 18.0;
    Serial.println(cfg.snd_warning);
  }
  else {
    Serial.println("NO snd_warning defined");
    cfg.snd_warning = 3.7;
  }

  if (ini.getValue("config", "snd_alarm", buffer, bufferLen)) {
    Serial.print("snd_alarm = ");
    cfg.snd_alarm = atof(buffer);
    if( cfg.show_mgdl )
      cfg.snd_alarm /= 18.0;
    Serial.println(cfg.snd_alarm);
  }
  else {
    Serial.println("NO snd_alarm defined");
    cfg.snd_alarm = 3.0;
  }

  if (ini.getValue("config", "brightness1", buffer, bufferLen)) {
    Serial.print("brightness1 = ");
    Serial.println(buffer);
    cfg.brightness1 = atoi(buffer);
    if(cfg.brightness1<1 || cfg.brightness1>100)
      cfg.brightness1 = 50;
  }
  else {
    Serial.println("NO brightness1");
    cfg.brightness1 = 50;
  }
  
  if (ini.getValue("config", "brightness2", buffer, bufferLen)) {
    Serial.print("brightness2 = ");
    Serial.println(buffer);
    cfg.brightness2 = atoi(buffer);
    if(cfg.brightness2<1 || cfg.brightness2>100)
      cfg.brightness2 = 100;
  }
  else {
    Serial.println("NO brightness2");
    cfg.brightness2 = 100;
  }
  if (ini.getValue("config", "brightness3", buffer, bufferLen)) {
    Serial.print("brightness3 = ");
    Serial.println(buffer);
    cfg.brightness3 = atoi(buffer);
    if(cfg.brightness3<1 || cfg.brightness3>100)
      cfg.brightness3 = 10;
  }
  else {
    Serial.println("NO brightness3");
    cfg.brightness3 = 10;
  }

  if (ini.getValue("wlan1", "ssid", buffer, bufferLen)) {
    Serial.print("wlan1ssid = ");
    Serial.println(buffer);
    strcpy(cfg.wlan1ssid, buffer);
  }
  else {
    Serial.println("NO wlan1 ssid");
    cfg.wlan1ssid[0] = 0;
  }

  if (ini.getValue("wlan1", "pass", buffer, bufferLen)) {
    Serial.print("wlan1pass = ");
    Serial.println(buffer);
    strcpy(cfg.wlan1pass, buffer);
  }
  else {
    Serial.println("NO wlan1 pass");
    cfg.wlan1pass[0] = 0;
  }

  if (ini.getValue("wlan2", "ssid", buffer, bufferLen)) {
    Serial.print("wlan2ssid = ");
    Serial.println(buffer);
    strcpy(cfg.wlan2ssid, buffer);
  }
  else {
    Serial.println("NO wlan2 ssid");
    cfg.wlan2ssid[0] = 0;
  }

  if (ini.getValue("wlan2", "pass", buffer, bufferLen)) {
    Serial.print("wlan2pass = ");
    Serial.println(buffer);
    strcpy(cfg.wlan2pass, buffer);
  }
  else {
    Serial.println("NO wlan2 pass");
    cfg.wlan2pass[0] = 0;
  }

  if (ini.getValue("wlan3", "ssid", buffer, bufferLen)) {
    Serial.print("wlan3ssid = ");
    Serial.println(buffer);
    strcpy(cfg.wlan3ssid, buffer);
  }
  else {
    Serial.println("NO wlan3 ssid");
    cfg.wlan3ssid[0] = 0;
  }

  if (ini.getValue("wlan3", "pass", buffer, bufferLen)) {
    Serial.print("wlan3pass = ");
    Serial.println(buffer);
    strcpy(cfg.wlan3pass, buffer);
  }
  else {
    Serial.println("NO wlan3 pass");
    cfg.wlan3pass[0] = 0;
  }
}

// the setup routine runs once when M5Stack starts up
void setup() {
    // initialize the M5Stack object
    M5.begin();
    // prevent button A "ghost" random presses
    Wire.begin();

    // M5.Speaker.mute();

    // Lcd display
    M5.Lcd.setBrightness(100);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(2);
    yield();

    Serial.println(ESP.getFreeHeap());

    readConfiguration();
    lcdBrightness = cfg.brightness1;
    M5.Lcd.setBrightness(lcdBrightness);
    
    startupLogo();
    //M5.Speaker.mute();
    yield();

    M5.Lcd.setBrightness(lcdBrightness);
    wifi_connect();
    yield();

    M5.Lcd.setBrightness(lcdBrightness);
    M5.Lcd.fillScreen(BLACK);

    // update glycemia now
    msCount = millis()-16000;
}

void drawArrow(int x, int y, int asize, int aangle, int pwidth, int plength, uint16_t color){
  float dx = (asize-10)*cos(aangle-90)*PI/180+x; // calculate X position  
  float dy = (asize-10)*sin(aangle-90)*PI/180+y; // calculate Y position  
  float x1 = 0;         float y1 = plength;
  float x2 = pwidth/2;  float y2 = pwidth/2;
  float x3 = -pwidth/2; float y3 = pwidth/2;
  float angle = aangle*PI/180-135;
  float xx1 = x1*cos(angle)-y1*sin(angle)+dx;
  float yy1 = y1*cos(angle)+x1*sin(angle)+dy;
  float xx2 = x2*cos(angle)-y2*sin(angle)+dx;
  float yy2 = y2*cos(angle)+x2*sin(angle)+dy;
  float xx3 = x3*cos(angle)-y3*sin(angle)+dx;
  float yy3 = y3*cos(angle)+x3*sin(angle)+dy;
  M5.Lcd.fillTriangle(xx1,yy1,xx3,yy3,xx2,yy2, color);
  M5.Lcd.drawLine(x, y, xx1, yy1, color);
  M5.Lcd.drawLine(x+1, y, xx1+1, yy1, color);
  M5.Lcd.drawLine(x, y+1, xx1, yy1+1, color);
  M5.Lcd.drawLine(x-1, y, xx1-1, yy1, color);
  M5.Lcd.drawLine(x, y-1, xx1, yy1-1, color);
  M5.Lcd.drawLine(x+2, y, xx1+2, yy1, color);
  M5.Lcd.drawLine(x, y+2, xx1, yy1+2, color);
  M5.Lcd.drawLine(x-2, y, xx1-2, yy1, color);
  M5.Lcd.drawLine(x, y-2, xx1, yy1-2, color);
}

void drawMiniGraph(){
  /*
  // draw help lines
  for(int i=0; i<320; i+=40) {
    M5.Lcd.drawLine(i, 0, i, 240, TFT_DARKGREY);
  }
  for(int i=0; i<240; i+=30) {
    M5.Lcd.drawLine(0, i, 320, i, TFT_DARKGREY);
  }
  M5.Lcd.drawLine(0, 120, 320, 120, TFT_LIGHTGREY);
  M5.Lcd.drawLine(160, 0, 160, 240, TFT_LIGHTGREY);
  */
  int i;
  float glk;
  uint16_t sgvColor;
  // M5.Lcd.drawLine(231, 110, 319, 110, TFT_DARKGREY);
  // M5.Lcd.drawLine(231, 110, 231, 207, TFT_DARKGREY);
  // M5.Lcd.drawLine(231, 207, 319, 207, TFT_DARKGREY);
  // M5.Lcd.drawLine(319, 110, 319, 207, TFT_DARKGREY);
  M5.Lcd.drawLine(231, 113, 319, 113, TFT_LIGHTGREY);
  M5.Lcd.drawLine(231, 203, 319, 203, TFT_LIGHTGREY);
  M5.Lcd.drawLine(231, 200-(4-3)*10+3, 319, 200-(4-3)*10+3, TFT_LIGHTGREY);
  M5.Lcd.drawLine(231, 200-(9-3)*10+3, 319, 200-(9-3)*10+3, TFT_LIGHTGREY);
  Serial.print("Last 10 values: ");
  for(i=9; i>=0; i--) {
    sgvColor = TFT_GREEN;
    glk = *(last10sgv+9-i);
    if(glk>12) {
      glk = 12;
    } else {
      if(glk<3) {
        glk = 3;
      }
    }
    if(glk<cfg.red_low || glk>cfg.red_high) {
      sgvColor = TFT_RED;
    } else {
      if(glk<cfg.yellow_low || glk>cfg.yellow_high) {
        sgvColor = TFT_YELLOW;
      }
    }
    Serial.print(*(last10sgv+i)); Serial.print(" ");
    M5.Lcd.fillCircle(234+i*9, 203-(glk-3.0)*10.0, 3, sgvColor);
  }
  Serial.println();
}
void update_glycemia() {
  M5.Lcd.setTextDatum(TL_DATUM);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(0, 0);
  // if there was an error, then clear whole screen, otherwise only graphic updated part
  if( wasError ) {
    M5.Lcd.fillScreen(BLACK);
  } else {
    M5.Lcd.fillRect(230, 110, 90, 100, TFT_BLACK);
  }
  M5.Lcd.drawJpgFile(SD, "/WiFi_symbol.jpg", 242, 130);
  // uint16_t maxWidth, uint16_t maxHeight, uint16_t offX, uint16_t offY, jpeg_div_t scale);
  if((WiFiMulti.run() == WL_CONNECTED)) {

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure target server and url
    char NSurl[128];
    strcpy(NSurl,"https://");
    strcat(NSurl,cfg.url);
    strcat(NSurl,"/api/v1/entries.json");
    http.begin(NSurl); //HTTP
    
    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();
  
    // httpCode will be negative on error
    if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if(httpCode == HTTP_CODE_OK) {
        String json = http.getString();
        // Serial.println(json);
        // const size_t capacity = JSON_ARRAY_SIZE(10) + 10*JSON_OBJECT_SIZE(19) + 3840;
        // Serial.print("JSON size needed= "); Serial.print(capacity); 
        Serial.print("Free Heap = "); Serial.println(ESP.getFreeHeap());
        auto JSONerr = deserializeJson(JSONdoc, json);
        if (JSONerr) {   //Check for errors in parsing
          Serial.println("JSON parsing failed");
          M5.Lcd.setFreeFont(FSSB12);
          M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
          M5.Lcd.drawString("JSON parsing failed", 0, 0, GFXFF);
          wasError = 1;
        } else {
          char sensDev[64];
          strlcpy(sensDev, JSONdoc[0]["device"] | "N/A", 64);
          // char sensDT[64];
          // strlcpy(sensDT, JSONdoc[0]["dateString"] | "N/A", 64);
          uint64_t rawtime = 0;
          rawtime = JSONdoc[0]["date"].as<long long>(); // sensTime is time in milliseconds since 1970, something like 1555229938118
          time_t sensTime = rawtime / 1000; // no milliseconds, since 2000 would be - 946684800, but ok
          char sensDir[32];
          strlcpy(sensDir, JSONdoc[0]["direction"] | "N/A", 32);
          for(int i=0; i<=9; i++) {
            last10sgv[i]=JSONdoc[i]["sgv"];
            last10sgv[i]/=18.0;
          }
          float sensSgv = JSONdoc[0]["sgv"]; //Get value of sensor measurement
          float sensSgvMgDl = sensSgv;
          // internally we work in mmol/L
          sensSgv/=18.0;
          
          char tmpstr[255];
          struct tm sensTm;
          localtime_r(&sensTime, &sensTm);
          
          Serial.print("sensDev = ");
          Serial.println(sensDev);
          Serial.print("sensTime = ");
          Serial.print(sensTime);
          sprintf(tmpstr, " (JSON %lld)", (long long) rawtime);
          Serial.print(tmpstr);
          sprintf(tmpstr, " = %s", ctime(&sensTime));
          Serial.print(tmpstr);
          Serial.print("sensSgv = ");
          Serial.println(sensSgv);
          Serial.print("sensDir = ");
          Serial.println(sensDir);
         
          // Serial.print(sensTm.tm_year+1900); Serial.print(" / "); Serial.print(sensTm.tm_mon+1); Serial.print(" / "); Serial.println(sensTm.tm_mday);
          Serial.print("Sensor: "); Serial.print(sensTm.tm_hour); Serial.print(":"); Serial.print(sensTm.tm_min); Serial.print(":"); Serial.print(sensTm.tm_sec); Serial.print(" DST "); Serial.println(sensTm.tm_isdst);
          
          M5.Lcd.setFreeFont(FSSB12);
          M5.Lcd.setTextSize(1);
          M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
          M5.Lcd.drawString("Nightscout", 0, 0, GFXFF);
          // char heapstr[20];
          // sprintf(heapstr, "%i B free", ESP.getFreeHeap());
          // M5.Lcd.drawString(heapstr, 0, 0, GFXFF);
          M5.Lcd.drawString(cfg.userName, 0, 24, GFXFF);
 
          M5.Lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
          char dateStr[30];
          sprintf(dateStr, "%d.%d.%04d", sensTm.tm_mday, sensTm.tm_mon+1, sensTm.tm_year+1900);
          M5.Lcd.drawString(dateStr, 0, 48, GFXFF);
          char timeStr[30];
          sprintf(timeStr, "%02d:%02d:%02d", sensTm.tm_hour, sensTm.tm_min, sensTm.tm_sec);
          M5.Lcd.drawString(timeStr, 0, 72, GFXFF);

          // calculate sensor time difference
          int sensorDifSec=0;
          struct tm timeinfo;
          if(!getLocalTime(&timeinfo)){
            sensorDifSec=24*60*60; // too much
          } else {
            // Serial.print("LOCAL: "); Serial.print(timeinfo.tm_year+1900); Serial.print(" / "); Serial.print(timeinfo.tm_mon+1); Serial.print(" / "); Serial.println(timeinfo.tm_mday);
            Serial.print("Local: "); Serial.print(timeinfo.tm_hour); Serial.print(":"); Serial.print(timeinfo.tm_min); Serial.print(":"); Serial.print(timeinfo.tm_sec); Serial.print(" DST "); Serial.println(timeinfo.tm_isdst);
            sensorDifSec=difftime(mktime(&timeinfo), sensTime);
          }
          Serial.print("Sensor time difference = "); Serial.print(sensorDifSec); Serial.println(" sec");
          unsigned int sensorDifMin = (sensorDifSec+30)/60;
          uint16_t tdColor = TFT_LIGHTGREY;
          if(sensorDifMin>5) {
            tdColor = TFT_WHITE;
            if(sensorDifMin>15) {
              tdColor = TFT_RED;
            }
          }
          M5.Lcd.fillRoundRect(200,0,120,90,15,tdColor);
          M5.Lcd.setTextSize(1);
          M5.Lcd.setFreeFont(FSSB24);
          M5.Lcd.setTextDatum(MC_DATUM);
          M5.Lcd.setTextColor(TFT_BLACK, tdColor);
          if(sensorDifMin>99) {
            M5.Lcd.drawString("Err", 260, 32, GFXFF);
          } else {
            M5.Lcd.drawNumber(sensorDifMin, 260, 32, GFXFF);
          }
          M5.Lcd.setTextSize(1);
          M5.Lcd.setFreeFont(FSSB12);
          M5.Lcd.setTextDatum(MC_DATUM);
          M5.Lcd.setTextColor(TFT_BLACK, tdColor);
          M5.Lcd.drawString("min", 260, 70, GFXFF);
          
          uint16_t glColor = TFT_GREEN;
          if(sensSgv<cfg.yellow_low || sensSgv>cfg.yellow_high) {
            glColor=TFT_YELLOW; // warning is YELLOW
          }
          if(sensSgv<cfg.red_low || sensSgv>cfg.red_high) {
            glColor=TFT_RED; // alert is RED
          }

          char glykStr[128];
          sprintf(glykStr, "Glyk: %4.1f %s", sensSgv, sensDir);
          Serial.println(glykStr);
          // M5.Lcd.println(glykStr);
          
          M5.Lcd.fillRect(0, 110, 320, 114, TFT_BLACK);
          M5.Lcd.setTextSize(2);
          M5.Lcd.setTextDatum(TL_DATUM);
          M5.Lcd.setTextColor(glColor, TFT_BLACK);
          char sensSgvStr[30];
          int smaller_font = 0;
          if( cfg.show_mgdl ) {
            sprintf(sensSgvStr, "%3.0f", sensSgvMgDl);
          } else {
            sprintf(sensSgvStr, "%4.1f", sensSgv);
            if( sensSgvStr[0]!=' ' )
              smaller_font = 1;
          }
          // Serial.print("SGV string length = "); Serial.print(strlen(sensSgvStr));
          // Serial.print(", smaller_font = "); Serial.println(smaller_font);
          if( smaller_font ) {
            M5.Lcd.setFreeFont(FSSB18);
            M5.Lcd.drawString(sensSgvStr, 0, 130, GFXFF);
          } else {
            M5.Lcd.setFreeFont(FSSB24);
            M5.Lcd.drawString(sensSgvStr, 0, 120, GFXFF);
          }
          int tw=M5.Lcd.textWidth(sensSgvStr);
          int th=M5.Lcd.fontHeight(GFXFF);
          // Serial.print("textWidth="); Serial.println(tw);
          // Serial.print("textHeight="); Serial.println(th);

          /*
          M5.Lcd.setTextSize(1);
          M5.Lcd.setFreeFont(FSSB24);
          M5.Lcd.setTextColor(glColor, TFT_BLACK);
          M5.Lcd.setCursor(0, 160+2*24);
          M5.Lcd.println(sensDir);
          */

          int arrowAngle = 180;
          if(strcmp(sensDir,"DoubleDown")==0)
            arrowAngle = 90;
          else 
            if(strcmp(sensDir,"SingleDown")==0)
              arrowAngle = 75;
            else 
                if(strcmp(sensDir,"FortyFiveDown")==0)
                  arrowAngle = 45;
                else 
                    if(strcmp(sensDir,"Flat")==0)
                      arrowAngle = 0;
                    else 
                        if(strcmp(sensDir,"FortyFiveUp")==0)
                          arrowAngle = -45;
                        else 
                            if(strcmp(sensDir,"SingleUp")==0)
                              arrowAngle = -75;
                            else 
                                if(strcmp(sensDir,"DoubleUp")==0)
                                  arrowAngle = -90;
                                else 
                                    if(strcmp(sensDir,"NONE")==0)
                                      arrowAngle = 180;
                                    else 
                                        if(strcmp(sensDir,"NOT COMPUTABLE")==0)
                                          arrowAngle = 180;
          if(arrowAngle!=180)
            drawArrow(0+tw+25, 120+40, 10, arrowAngle+85, 40, 40, glColor);
          
          M5.Lcd.setTextSize(1);
          M5.Lcd.setFreeFont(FSSB12);
          M5.Lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
          char devStr[64];
          strcpy(devStr, sensDev);
          if(strcmp(devStr,"MIAOMIAO")==0) {
            JsonObject obj=JSONdoc[0].as<JsonObject>();
            if(obj.containsKey("xDrip_raw")) {
              strcpy(devStr,"xDrip MiaoMiao + Libre");
            } else {
              strcpy(devStr,"Spike MiaoMiao + Libre");
            }
          }
          if(strcmp(devStr,"Tomato")==0)
            strcat(devStr," MiaoMiao + Libre");
          M5.Lcd.drawString(devStr, 0, 220, GFXFF);

          /*
          // draw help lines
          for(int i=0; i<320; i+=40) {
            M5.Lcd.drawLine(i, 0, i, 240, TFT_DARKGREY);
          }
          for(int i=0; i<240; i+=30) {
            M5.Lcd.drawLine(0, i, 320, i, TFT_DARKGREY);
          }
          M5.Lcd.drawLine(0, 120, 320, 120, TFT_LIGHTGREY);
          M5.Lcd.drawLine(160, 0, 160, 240, TFT_LIGHTGREY);
          */

          if(sensorDifSec<23) {
            if((sensSgv<=cfg.snd_alarm) && (sensSgv>=0.1))
              sndAlarm();
            else
              if((sensSgv<=cfg.snd_warning) && (sensSgv>=0.1))
                sndWarning();
          }

          drawMiniGraph();
        }
      } else {
        String errstr = String("[HTTP] GET not ok, error: " + String(httpCode));
        Serial.println(errstr);
        M5.Lcd.setCursor(0, 23);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setFreeFont(FSSB12);
        M5.Lcd.println(errstr);
        wasError = 1;
      }
    } else {
      String errstr = String("[HTTP] GET failed, error: " + String(http.errorToString(httpCode).c_str()));
      Serial.println(errstr);
      M5.Lcd.setCursor(0, 23);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setFreeFont(FSSB12);
      M5.Lcd.println(errstr);
      wasError = 1;
    }
  
    http.end();
  }
}

// the loop routine runs over and over again forever
void loop(){
    delay(10);
    buttons_test();
    // update glycemia every 10s
    if(millis()-msCount>15000) {
      update_glycemia();
      msCount = millis();  
    }
    M5.update();
}
