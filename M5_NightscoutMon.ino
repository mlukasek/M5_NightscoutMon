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
    IoT Icon Set by Artur Funk (GPL v3)
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
#include "M5NSconfig.h"

tConfig cfg;

// extern const unsigned char gImage_logoM5[];
extern const unsigned char m5stack_startup_music[];
extern const unsigned char WiFi_symbol[];
extern const unsigned char alarmSndData[];

extern const unsigned char sun_icon16x16[];
extern const unsigned char clock_icon16x16[];
extern const unsigned char timer_icon16x16[];
extern const unsigned char powerbutton_icon16x16[];

extern const unsigned char bat0_icon16x16[];
extern const unsigned char bat1_icon16x16[];
extern const unsigned char bat2_icon16x16[];
extern const unsigned char bat3_icon16x16[];
extern const unsigned char bat4_icon16x16[];
extern const unsigned char plug_icon16x16[];

const char* ntpServer = "pool.ntp.org";
struct tm localTimeInfo;
int lastSec = 61;
int lastMin = 61;
char localTimeStr[30];

#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

WiFiMulti WiFiMulti;
unsigned long msCount;
unsigned long msCountLog;
static uint8_t lcdBrightness = 10;
static char *iniFilename = "/M5NS.INI";

DynamicJsonDocument JSONdoc(16384);
float last10sgv[10];
int wasError = 0;
time_t lastAlarmTime = 0;
time_t lastSnoozeTime = 0;

void startupLogo() {
    static uint8_t brightness, pre_brightness;
    M5.Lcd.setBrightness(0);
    if(cfg.bootPic[0]==0) {
      // M5.Lcd.pushImage(0, 0, 320, 240, (uint16_t *)gImage_logoM5);
      M5.Lcd.drawString("M5 Stack", 120, 60, GFXFF);
      M5.Lcd.drawString("Nightscout monitor", 60, 80, GFXFF);
      M5.Lcd.drawString("(c) 2019 Martin Lukasek", 20, 120, GFXFF);
    } else {
      M5.Lcd.drawJpgFile(SD, cfg.bootPic);
    }
    M5.Lcd.setBrightness(100);
    M5.update();
    M5.Speaker.playMusic(m5stack_startup_music,25000);
    delay(1000);
    M5.Lcd.fillScreen(BLACK);
    delay(800);
    for(int i=0; i>=100; i++) {
        M5.Lcd.setBrightness(i);
        delay(2);
    }
}

void printLocalTime() {
  if(!getLocalTime(&localTimeInfo)){
    Serial.println("Failed to obtain time");
    M5.Lcd.println("Failed to obtain time");
    return;
  }
  Serial.println(&localTimeInfo, "%A, %B %d %Y %H:%M:%S");
  M5.Lcd.println(&localTimeInfo, "%A, %B %d %Y %H:%M:%S");
}

static uint8_t music_data[25000]; // 5s in sample rate 5000 samp/s

void play_music_data(uint32_t data_length, uint8_t volume) {
  uint8_t vol;
  if( volume>100 )
    vol=1;
  else
    vol=101-volume;
  if(vol != 101) {
    ledcAttachPin(SPEAKER_PIN, TONE_PIN_CHANNEL);
    delay(10);
    for(int i=0; i<data_length; i++) {
      dacWrite(SPEAKER_PIN, music_data[i]/vol);
      delayMicroseconds(200); // 1 000 000 microseconds / sample rate 5000
    }
    /* takes too long
    // slowly set DAC to zero from the last value
    for(int t=music_data[data_length-1]; t>=0; t--) {
      dacWrite(SPEAKER_PIN, t);
      delay(2);
    } */
    dacWrite(SPEAKER_PIN, 0);
    delay(10);
    ledcDetachPin(SPEAKER_PIN);
  }
}

void play_tone(uint16_t frequency, uint32_t duration, uint8_t volume) {
  // Serial.print("start fill music data "); Serial.println(millis());
  uint32_t data_length = 5000;
  if( duration*5 < data_length )
    data_length = duration*5;
  float interval = 2*M_PI*float(frequency)/float(5000);
  for (int i=0;i<data_length;i++) {
    music_data[i]=127+126*sin(interval*i);
  }
  // Serial.print("finish fill music data "); Serial.println(millis());
  play_music_data(data_length, volume);
}    

void sndAlarm() {
    for(int j=0; j<6; j++) {
      if( cfg.dev_mode )
        play_tone(660, 400, 1);
      else
        play_tone(660, 400, cfg.alarm_volume);
      delay(200);
    }
    M5.Speaker.mute();
    // M5.Speaker.playMusic(m5stack_startup_music, 25000);        
    // M5.Speaker.update();
}

void sndWarning() {
  for(int j=0; j<3; j++) {
    if( cfg.dev_mode )
      play_tone(3000, 100, 1);
    else
      play_tone(3000, 100, cfg.warning_volume);
    delay(300);
  }
  M5.Speaker.mute();
}

int tmpvol = 1;
void buttons_test() {
  if(M5.BtnA.wasPressed()) {
      // M5.Lcd.printf("A");
      Serial.printf("A");
      // play_tone(1000, 10, 1);
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
      // play_tone(1000, 10, 1);
      struct tm timeinfo;
      if(!getLocalTime(&timeinfo)){
        lastSnoozeTime=0;
      } else {
        lastSnoozeTime=mktime(&timeinfo);
      }
      M5.Lcd.setTextSize(1);
      M5.Lcd.setFreeFont(FSSB12);
      M5.Lcd.fillRect(110, 220, 100, 20, TFT_WHITE);
      M5.Lcd.setTextColor(TFT_BLACK, TFT_WHITE);
      char tmpStr[10];
      sprintf(tmpStr, "%i", cfg.snooze_timeout);
      int txw=M5.Lcd.textWidth(tmpStr);
      Serial.print("Set SNOOZE: "); Serial.println(tmpStr);
      M5.Lcd.drawString(tmpStr, 159-txw/2, 220, GFXFF);
  } 
  if(M5.BtnC.wasPressed()) {
      // M5.Lcd.printf("C");
      Serial.printf("C");
      // play_tone(1000, 10, 1);
      M5.setWakeupButton(BUTTON_B_PIN);
      M5.powerOFF();

      /*
      for (int i=0;i<25000;i++) {
        music_data[i]=alarmSndData[i];
      }
      play_music_data(25000, 10); */
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

int8_t getBatteryLevel()
{
  Wire.beginTransmission(0x75);
  Wire.write(0x78);
  if (Wire.endTransmission(false) == 0
   && Wire.requestFrom(0x75, 1)) {
    int8_t bdata=Wire.read();
    /* 
    // write battery info to logfile.txt
    File fileLog = SD.open("/logfile.txt", FILE_WRITE);    
    if(!fileLog) {
      Serial.println("Cannot write to logfile.txt");
    } else {
      int pos = fileLog.seek(fileLog.size());
      struct tm timeinfo;
      getLocalTime(&timeinfo);
      fileLog.print(asctime(&timeinfo));
      fileLog.print("   Battery level: "); fileLog.println(bdata, HEX);
      fileLog.close();
      Serial.print("Log file written: "); Serial.print(asctime(&timeinfo));
    }
    */
    switch (bdata & 0xF0) {
      case 0xE0: return 25;
      case 0xC0: return 50;
      case 0x80: return 75;
      case 0x00: return 100;
      default: return 0;
    }
  }
  return -1;
}

// the setup routine runs once when M5Stack starts up
void setup() {
    // initialize the M5Stack object
    M5.begin();
    // prevent button A "ghost" random presses
    Wire.begin();
    SD.begin();
    
    M5.Speaker.mute();

    // Lcd display
    M5.Lcd.setBrightness(100);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(2);
    yield();

    Serial.println(ESP.getFreeHeap());

    readConfiguration(iniFilename, &cfg);
    // strcpy(cfg.url, "user.herokuapp.com");
    // cfg.show_mgdl = 1;
    // cfg.show_COB_IOB = 0;
    // cfg.snd_warning = 5.5;
    // cfg.snd_alarm = 4.5;
    // cfg.snd_warning_high = 9;
    // cfg.snd_alarm_high = 14;
    // cfg.alarm_volume = 0;
    // cfg.warning_volume = 0;
    
    // cfg.alarm_repeat = 1;
    // cfg.snooze_timeout = 2;
    // cfg.brightness1 = 0;
    // cfg.info_line = 1;
    
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

    // test file with time stamps
    msCountLog = millis()-6000;
     
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
    if(*(last10sgv+9-i)!=0)
      M5.Lcd.fillCircle(234+i*9, 203-(glk-3.0)*10.0, 3, sgvColor);
  }
  Serial.println();
}
void drawIcon(int16_t x, int16_t y, const uint8_t *bitmap, uint16_t color) {
  int16_t w = 16;
  int16_t h = 16; 
  int32_t i, j, byteWidth = (w + 7) / 8;
  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++) {
      if (pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
        M5.Lcd.drawPixel(x + i, y + j, color);
      }
    }
  }
}

void update_glycemia() {
  char loopInfoStr[64];
  char basalInfoStr[64];
  
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
  // M5.Lcd.drawJpgFile(SD, "/WiFi_symbol.jpg", 242, 130);
  M5.Lcd.drawBitmap(242, 130, 64, 48, (uint16_t *)WiFi_symbol);
  // uint16_t maxWidth, uint16_t maxHeight, uint16_t offX, uint16_t offY, jpeg_div_t scale);
  if((WiFiMulti.run() == WL_CONNECTED)) {

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure target server and url
    char NSurl[128];
    strcpy(NSurl,"https://");
    strcat(NSurl,cfg.url);
    strcat(NSurl,"/api/v1/entries.json");
    // more info at /api/v2/properties
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
        wasError = 0;
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
          uint64_t rawtime = 0;
          char sensDir[32];
          float sensSgv = 0;
          JsonObject obj; 
          int sgvindex = 0;
          do {
            obj=JSONdoc[sgvindex].as<JsonObject>();
            sgvindex++;
          } while ((!obj.containsKey("sgv")) && (sgvindex<9));
          sgvindex--;
          if(sgvindex<0 || sgvindex>8)
            sgvindex=0;
          strlcpy(sensDev, JSONdoc[sgvindex]["device"] | "N/A", 64);
          rawtime = JSONdoc[sgvindex]["date"].as<long long>(); // sensTime is time in milliseconds since 1970, something like 1555229938118
          strlcpy(sensDir, JSONdoc[sgvindex]["direction"] | "N/A", 32);
          sensSgv = JSONdoc[sgvindex]["sgv"]; // get value of sensor measurement
          time_t sensTime = rawtime / 1000; // no milliseconds, since 2000 would be - 946684800, but ok
          for(int i=0; i<=9; i++) {
            last10sgv[i]=JSONdoc[i]["sgv"];
            last10sgv[i]/=18.0;
          }
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
          Serial.print("Sensor time: "); Serial.print(sensTm.tm_hour); Serial.print(":"); Serial.print(sensTm.tm_min); Serial.print(":"); Serial.print(sensTm.tm_sec); Serial.print(" DST "); Serial.println(sensTm.tm_isdst);

          M5.Lcd.setFreeFont(FSSB12);
          M5.Lcd.setTextSize(1);
          M5.Lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
          // char dateStr[30];
          // sprintf(dateStr, "%d.%d.%04d", sensTm.tm_mday, sensTm.tm_mon+1, sensTm.tm_year+1900);
          // M5.Lcd.drawString(dateStr, 0, 48, GFXFF);
          // char timeStr[30];
          // sprintf(timeStr, "%02d:%02d:%02d", sensTm.tm_hour, sensTm.tm_min, sensTm.tm_sec);
          // M5.Lcd.drawString(timeStr, 0, 72, GFXFF);
          char datetimeStr[30];
          struct tm timeinfo;
          if(cfg.show_current_time) {
            if(getLocalTime(&timeinfo)) {
              // sprintf(datetimeStr, "%02d:%02d:%02d  %d.%d.  ", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon+1);  
              sprintf(datetimeStr, "%02d:%02d  %d.%d.  ", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_mday, timeinfo.tm_mon+1);  
            } else {
              // strcpy(datetimeStr, "??:??:??");
              strcpy(datetimeStr, "??:??");
            }
          } else {
            // sprintf(datetimeStr, "%02d:%02d:%02d  %d.%d.  ", sensTm.tm_hour, sensTm.tm_min, sensTm.tm_sec, sensTm.tm_mday, sensTm.tm_mon+1);
            sprintf(datetimeStr, "%02d:%02d  %d.%d.  ", sensTm.tm_hour, sensTm.tm_min, sensTm.tm_mday, sensTm.tm_mon+1);
          }
          M5.Lcd.drawString(datetimeStr, 0, 0, GFXFF);

          // draw battery status
          int8_t battLevel = getBatteryLevel();
          Serial.print("Battery level: "); Serial.println(battLevel);
          // sprintf(tmpstr, "%d", battLevel);
          // M5.Lcd.drawString(tmpstr, 0, 220, GFXFF);
          M5.Lcd.fillRect(168, 0, 16, 17, TFT_BLACK);
          if(battLevel!=-1) {
            switch(battLevel) {
              case 0:
                drawIcon(168, 1, (uint8_t*)bat0_icon16x16, TFT_RED);
                break;
              case 25:
                drawIcon(168, 1, (uint8_t*)bat1_icon16x16, TFT_YELLOW);
                break;
              case 50:
                drawIcon(168, 1, (uint8_t*)bat2_icon16x16, TFT_WHITE);
                break;
              case 75:
                drawIcon(168, 1, (uint8_t*)bat3_icon16x16, TFT_LIGHTGREY);
                break;
              case 100:
                drawIcon(168, 0, (uint8_t*)plug_icon16x16, TFT_LIGHTGREY);
                break;
            }
          }

          M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
          M5.Lcd.drawString(cfg.userName, 0, 24, GFXFF);
          
          /*
          char diffstr[10];
          if( cfg.show_mgdl ) {
              sprintf(diffstr, "%+3.0f", (last10sgv[sgvindex]-last10sgv[sgvindex+1])*18 );
          } else {
              sprintf(diffstr, "%+4.1f", last10sgv[sgvindex]-last10sgv[sgvindex+1] );
          }
          M5.Lcd.fillRect(130,24,69,23,TFT_BLACK);
          M5.Lcd.drawString(diffstr, 130, 24, GFXFF);
          */
          
          /*
          if( !cfg.dev_mode ) {
            M5.Lcd.drawString("Nightscout", 0, 48, GFXFF);
          } else {
            char heapstr[20];
            sprintf(heapstr, "%i free  ", ESP.getFreeHeap());
            M5.Lcd.drawString(heapstr, 0, 48, GFXFF);
          }
          */
          
          strcpy(NSurl,"https://");
          strcat(NSurl,cfg.url);
          strcat(NSurl,"/api/v2/properties/iob,cob,delta,loop,basal");
          http.begin(NSurl); //HTTP
          Serial.print("[HTTP] GET properties...\n");
          int httpCode = http.GET();
          if(httpCode > 0) {
            Serial.printf("[HTTP] GET properties... code: %d\n", httpCode);
            if(httpCode == HTTP_CODE_OK) {
              // const char* propjson = "{\"iob\":{\"iob\":0,\"activity\":0,\"source\":\"OpenAPS\",\"device\":\"openaps://Spike iPhone 8 Plus\",\"mills\":1557613521000,\"display\":\"0\",\"displayLine\":\"IOB: 0U\"},\"cob\":{\"cob\":0,\"source\":\"OpenAPS\",\"device\":\"openaps://Spike iPhone 8 Plus\",\"mills\":1557613521000,\"treatmentCOB\":{\"decayedBy\":\"2019-05-11T23:05:00.000Z\",\"isDecaying\":0,\"carbs_hr\":20,\"rawCarbImpact\":0,\"cob\":7,\"lastCarbs\":{\"_id\":\"5cd74c26156712edb4b32455\",\"enteredBy\":\"Martin\",\"eventType\":\"Carb Correction\",\"reason\":\"\",\"carbs\":7,\"duration\":0,\"created_at\":\"2019-05-11T22:24:00.000Z\",\"mills\":1557613440000,\"mgdl\":67}},\"display\":0,\"displayLine\":\"COB: 0g\"},\"delta\":{\"absolute\":-4,\"elapsedMins\":4.999483333333333,\"interpolated\":false,\"mean5MinsAgo\":69,\"mgdl\":-4,\"scaled\":-0.2,\"display\":\"-0.2\",\"previous\":{\"mean\":69,\"last\":69,\"mills\":1557613221946,\"sgvs\":[{\"mgdl\":69,\"mills\":1557613221946,\"device\":\"MIAOMIAO\",\"direction\":\"Flat\",\"filtered\":92588,\"unfiltered\":92588,\"noise\":1,\"rssi\":100}]}}}";
              String propjson = http.getString();
              const size_t propcapacity = 16300;
              DynamicJsonDocument propdoc(propcapacity);
              Serial.println("Created the second JSON document");
              auto propJSONerr = deserializeJson(propdoc, propjson);
              if(propJSONerr) {
                Serial.println("Properties JSON parsing failed");
                M5.Lcd.fillRect(130,24,69,23,TFT_BLACK);
                M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
                M5.Lcd.drawString("???", 130, 24, GFXFF);
              } else {
                Serial.println("Deserialized the second JSON and OK");
                JsonObject iob = propdoc["iob"];
                float iob_iob = iob["iob"]; // 0
                const char* iob_display = iob["display"] | "N/A"; // "0"
                const char* iob_displayLine = iob["displayLine"] | "IOB: N/A"; // "IOB: 0U"
                Serial.println("IOB OK");
                
                JsonObject cob = propdoc["cob"];
                float cob_cob = cob["cob"]; // 0
                const char* cob_display = cob["display"] | "N/A"; // 0
                const char* cob_displayLine = cob["displayLine"] | "COB: N/A"; // "COB: 0g"
                Serial.println("COB OK");
                
                JsonObject delta = propdoc["delta"];
                int delta_absolute = delta["absolute"]; // -4
                float delta_elapsedMins = delta["elapsedMins"]; // 4.999483333333333
                bool delta_interpolated = delta["interpolated"]; // false
                int delta_mean5MinsAgo = delta["mean5MinsAgo"]; // 69
                int delta_mgdl = delta["mgdl"]; // -4
                float delta_scaled = delta["scaled"]; // -0.2
                const char* delta_display = delta["display"] | ""; // "-0.2"
                M5.Lcd.fillRect(130,24,69,23,TFT_BLACK);
                M5.Lcd.drawString(delta_display, 130, 24, GFXFF);
                Serial.println("DELTA OK");
                
                JsonObject loop_obj = propdoc["loop"];
                JsonObject loop_display = loop_obj["display"];
                const char* loop_display_symbol = loop_display["symbol"] | "?"; // "⌁"
                const char* loop_display_code = loop_display["code"] | "N/A"; // "enacted"
                const char* loop_display_label = loop_display["label"] | "N/A"; // "Enacted"
                Serial.println("LOOP OK");

                JsonObject basal = propdoc["basal"];
                const char* basal_display = basal["display"] | "N/A"; // "T: 0.950U"      
                Serial.println("BASAL OK");

                strlcpy(loopInfoStr, loop_display_label, 64);
                strlcpy(basalInfoStr, basal_display, 64);
                Serial.println("LOOP copy string OK");
                
                if(cfg.show_COB_IOB) {
                  M5.Lcd.fillRect(0,48,199,47,TFT_BLACK);
                  if(iob_iob>0)
                    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
                  else
                    M5.Lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
                  Serial.print("iob_displayLine=\""); Serial.print(iob_displayLine); Serial.println("\"");
                  M5.Lcd.drawString(iob_displayLine, 0, 48, GFXFF);
                  Serial.println("drawString IOB OK");
                  if(cob_cob>0)
                    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
                  else
                    M5.Lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
                  Serial.print("cob_displayLine=\""); Serial.print(cob_displayLine); Serial.println("\"");
                  M5.Lcd.drawString(cob_displayLine, 0, 72, GFXFF);
                  Serial.println("drawString COB OK");
                }
              }
            }
          }
    
          // calculate sensor time difference
          int sensorDifSec=0;
          if(!getLocalTime(&timeinfo)){
            sensorDifSec=24*60*60; // too much
          } else {
            Serial.print("Local time: "); Serial.print(timeinfo.tm_hour); Serial.print(":"); Serial.print(timeinfo.tm_min); Serial.print(":"); Serial.print(timeinfo.tm_sec); Serial.print(" DST "); Serial.println(timeinfo.tm_isdst);
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

          drawMiniGraph();

          // calculate last alarm time difference
          int alarmDifSec=1000000;
          int snoozeDifSec=1000000;
          if(!getLocalTime(&timeinfo)){
            alarmDifSec=24*60*60; // too much
            snoozeDifSec=cfg.snooze_timeout*60; // timeout
          } else {
            alarmDifSec=difftime(mktime(&timeinfo), lastAlarmTime);
            snoozeDifSec=difftime(mktime(&timeinfo), lastSnoozeTime);
            if( snoozeDifSec>cfg.snooze_timeout*60 )
              snoozeDifSec=cfg.snooze_timeout*60; // timeout
          }
          Serial.print("Alarm time difference = "); Serial.print(alarmDifSec); Serial.println(" sec");
          Serial.print("Snooze time difference = "); Serial.print(snoozeDifSec); Serial.println(" sec");
          char tmpStr[10];
          if( snoozeDifSec<cfg.snooze_timeout*60 ) {
            sprintf(tmpStr, "%i", (cfg.snooze_timeout*60-snoozeDifSec+59)/60);
          } else {
            strcpy(tmpStr, "Snooze");
          }
          M5.Lcd.setTextSize(1);
          M5.Lcd.setFreeFont(FSSB12);
          // Serial.print("sensSgv="); Serial.print(sensSgv); Serial.print(", cfg.snd_alarm="); Serial.println(cfg.snd_alarm); 
          if((sensSgv<=cfg.snd_alarm) && (sensSgv>=0.1)) {
            // red alarm state
            // M5.Lcd.fillRect(110, 220, 100, 20, TFT_RED);
            Serial.println("ALARM LOW");
            M5.Lcd.fillRect(0, 220, 320, 20, TFT_RED);
            M5.Lcd.setTextColor(TFT_BLACK, TFT_RED);
            int stw=M5.Lcd.textWidth(tmpStr);
            M5.Lcd.drawString(tmpStr, 159-stw/2, 220, GFXFF);
            if( (alarmDifSec>cfg.alarm_repeat*60) && (snoozeDifSec==cfg.snooze_timeout*60) ) {
                sndAlarm();
                lastAlarmTime = mktime(&timeinfo);
            }
          } else {
            if((sensSgv<=cfg.snd_warning) && (sensSgv>=0.1)) {
              // yellow warning state
              // M5.Lcd.fillRect(110, 220, 100, 20, TFT_YELLOW);
              Serial.println("WARNING LOW");
              M5.Lcd.fillRect(0, 220, 320, 20, TFT_YELLOW);
              M5.Lcd.setTextColor(TFT_BLACK, TFT_YELLOW);
              int stw=M5.Lcd.textWidth(tmpStr);
              M5.Lcd.drawString(tmpStr, 159-stw/2, 220, GFXFF);
              if( (alarmDifSec>cfg.alarm_repeat*60) && (snoozeDifSec==cfg.snooze_timeout*60) ) {
                sndWarning();
                lastAlarmTime = mktime(&timeinfo);
              }
            } else {
              if( sensSgv>=cfg.snd_alarm_high ) {
                // red alarm state
                // M5.Lcd.fillRect(110, 220, 100, 20, TFT_RED);
                Serial.println("ALARM HIGH");
                M5.Lcd.fillRect(0, 220, 320, 20, TFT_RED);
                M5.Lcd.setTextColor(TFT_BLACK, TFT_RED);
                int stw=M5.Lcd.textWidth(tmpStr);
                M5.Lcd.drawString(tmpStr, 159-stw/2, 220, GFXFF);
                if( (alarmDifSec>cfg.alarm_repeat*60) && (snoozeDifSec==cfg.snooze_timeout*60) ) {
                    sndAlarm();
                    lastAlarmTime = mktime(&timeinfo);
                }
              } else {
                if( sensSgv>=cfg.snd_warning_high ) {
                  // yellow warning state
                  // M5.Lcd.fillRect(110, 220, 100, 20, TFT_YELLOW);
                  Serial.println("WARNING HIGH");
                  M5.Lcd.fillRect(0, 220, 320, 20, TFT_YELLOW);
                  M5.Lcd.setTextColor(TFT_BLACK, TFT_YELLOW);
                  int stw=M5.Lcd.textWidth(tmpStr);
                  M5.Lcd.drawString(tmpStr, 159-stw/2, 220, GFXFF);
                  if( (alarmDifSec>cfg.alarm_repeat*60) && (snoozeDifSec==cfg.snooze_timeout*60) ) {
                    sndWarning();
                    lastAlarmTime = mktime(&timeinfo);
                  }
                } else {
                  if( sensorDifMin>=cfg.snd_no_readings ) {
                    // yellow warning state
                    // M5.Lcd.fillRect(110, 220, 100, 20, TFT_YELLOW);
                    Serial.println("WARNING NO READINGS");
                    M5.Lcd.fillRect(0, 220, 320, 20, TFT_YELLOW);
                    M5.Lcd.setTextColor(TFT_BLACK, TFT_YELLOW);
                    int stw=M5.Lcd.textWidth(tmpStr);
                    M5.Lcd.drawString(tmpStr, 159-stw/2, 220, GFXFF);
                    if( (alarmDifSec>cfg.alarm_repeat*60) && (snoozeDifSec==cfg.snooze_timeout*60) ) {
                      sndWarning();
                      lastAlarmTime = mktime(&timeinfo);
                    }
                  } else {
                    // normal glycemia state
                    M5.Lcd.fillRect(0, 220, 320, 20, TFT_BLACK);
                    M5.Lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
                    // draw info line
                    char infoStr[64];
                    switch( cfg.info_line ) {
                      case 0: // sensor information
                        strcpy(infoStr, sensDev);
                        if(strcmp(infoStr,"MIAOMIAO")==0) {
                          if(obj.containsKey("xDrip_raw")) {
                            strcpy(infoStr,"xDrip MiaoMiao + Libre");
                          } else {
                            strcpy(infoStr,"Spike MiaoMiao + Libre");
                          }
                        }
                        if(strcmp(infoStr,"Tomato")==0)
                          strcat(infoStr," MiaoMiao + Libre");
                        M5.Lcd.drawString(infoStr, 0, 220, GFXFF);
                        break;
                      case 1: // button function icons
                        // M5.Lcd.drawBitmap(50, 224, 16, 16, (uint8_t*)home_icon16x16);
                        // M5.Lcd.pushImage(50, 224, 16, 16, (uint8_t*)home_icon16x16);
                        drawIcon(58, 220, (uint8_t*)sun_icon16x16, TFT_LIGHTGREY);
                        drawIcon(153, 220, (uint8_t*)clock_icon16x16, TFT_LIGHTGREY);
                        // drawIcon(153, 220, (uint8_t*)timer_icon16x16, TFT_LIGHTGREY);
                        drawIcon(246, 220, (uint8_t*)powerbutton_icon16x16, TFT_LIGHTGREY);
                        break;
                      case 2: // loop + basal information
                        strcpy(infoStr, "L: ");
                        strlcat(infoStr, loopInfoStr, 64);
                        M5.Lcd.drawString(infoStr, 0, 220, GFXFF);
                        strcpy(infoStr, "B: ");
                        strlcat(infoStr, basalInfoStr, 64);
                        M5.Lcd.drawString(infoStr, 160, 220, GFXFF);
                        break;
                    }
                  }
                }
              }
            }
          }
          
          /*
          if(sensorDifSec<23) {
            if((sensSgv<=cfg.snd_alarm) && (sensSgv>=0.1))
              sndAlarm();
            else
              if((sensSgv<=cfg.snd_warning) && (sensSgv>=0.1))
                sndWarning();
          }
          */
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
  delay(20);
  buttons_test();

  // update glycemia every 15s
  if(millis()-msCount>15000) {
    update_glycemia();
    msCount = millis();  
  } else {
    if(cfg.show_current_time) {
      // update current time on display
      M5.Lcd.setFreeFont(FSSB12);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      if(!getLocalTime(&localTimeInfo)) {
        // unknown time
        strcpy(localTimeStr,"??:??");
        lastMin = 61;
      } else {
        if(getLocalTime(&localTimeInfo)) {
          sprintf(localTimeStr, "%02d:%02d  %d.%d.  ", localTimeInfo.tm_hour, localTimeInfo.tm_min, localTimeInfo.tm_mday, localTimeInfo.tm_mon+1);  
        } else {
          strcpy(localTimeStr, "??:??");
          lastMin = 61;
        }
      }
      if(lastMin!=localTimeInfo.tm_min) {
        lastSec=localTimeInfo.tm_sec;
        lastMin=localTimeInfo.tm_min;
        M5.Lcd.drawString(localTimeStr, 0, 0, GFXFF);
      }
    }
  }

  /*
  if(millis()-msCountLog>5000) {
    File fileLog = SD.open("/logfile.txt", FILE_WRITE);    
    if(!fileLog) {
      Serial.println("Cannot write to logfile.txt");
    } else {
      int pos = fileLog.seek(fileLog.size());
      struct tm timeinfo;
      getLocalTime(&timeinfo);
      fileLog.println(asctime(&timeinfo));
      fileLog.close();
      Serial.print("Log file written: "); Serial.print(asctime(&timeinfo));
    }
    msCountLog = millis();  
  }  
  */
  
  M5.update();
}
