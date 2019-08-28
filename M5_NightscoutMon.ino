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
    Additions to the code:
    Peter Leimbach (Nightscout token)
*/

#include <M5Stack.h>
#include <Preferences.h>
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

extern const unsigned char m5stack_startup_music[];
extern const unsigned char WiFi_symbol[];
extern const unsigned char alarmSndData[];

extern const unsigned char sun_icon16x16[];
extern const unsigned char clock_icon16x16[];
extern const unsigned char timer_icon16x16[];
// extern const unsigned char powerbutton_icon16x16[];
extern const unsigned char door_icon16x16[];
extern const unsigned char warning_icon16x16[];
extern const unsigned char wifi1_icon16x16[];
extern const unsigned char wifi2_icon16x16[];

extern const unsigned char bat0_icon16x16[];
extern const unsigned char bat1_icon16x16[];
extern const unsigned char bat2_icon16x16[];
extern const unsigned char bat3_icon16x16[];
extern const unsigned char bat4_icon16x16[];
extern const unsigned char plug_icon16x16[];

Preferences preferences;
tConfig cfg;

const char* ntpServer = "pool.ntp.org"; // "time.nist.gov", "time.google.com"
struct tm localTimeInfo;
int MAX_TIME_RETRY = 30;
int lastSec = 61;
int lastMin = 61;
char localTimeStr[30];

struct err_log_item {
  struct tm err_time;
  int err_code;
} err_log[10];
int err_log_ptr = 0;
int err_log_count = 0;

int dispPage = 0;
#define MAX_PAGE 2
int icon_xpos[3] = {144, 144+18, 144+2*18};
int icon_ypos[3] = {0, 0, 0};

#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

WiFiMulti WiFiMulti;
unsigned long msCount;
unsigned long msCountLog;
unsigned long msStart;
static uint8_t lcdBrightness = 10;
static char *iniFilename = "/M5NS.INI";

DynamicJsonDocument JSONdoc(16384);
time_t lastAlarmTime = 0;
time_t lastSnoozeTime = 0;
static uint8_t music_data[25000]; // 5s in sample rate 5000 samp/s

void setPageIconPos(int page) {
  switch(page) {
    case 0:
      icon_xpos[0] = 144;
      icon_xpos[1] = 144+18;
      icon_xpos[2] = 144+2*18;
      icon_ypos[0] = 0;
      icon_ypos[1] = 0;
      icon_ypos[2] = 0;
      break;
    case 1:
      icon_xpos[0] = 126;
      icon_xpos[1] = 126+18;
      icon_xpos[2] = 126+18; // 320-16;
      icon_ypos[0] = 0;
      icon_ypos[1] = 0;
      icon_ypos[2] = 18; // 220;
      break;
    default:
      icon_xpos[0] = 144;
      icon_xpos[1] = 144+18;
      icon_xpos[2] = 144+2*18;
      icon_ypos[0] = 0;
      icon_ypos[1] = 0;
      icon_ypos[2] = 0;
      break;
  }
}

void addErrorLog(int code){
  if(err_log_ptr>9) {
    for(int i=0; i<9; i++) {
      err_log[i].err_time=err_log[i+1].err_time;
      err_log[i].err_code=err_log[i+1].err_code;
    }
    err_log_ptr=9;
  }
  getLocalTime(&err_log[err_log_ptr].err_time);
  err_log[err_log_ptr].err_code=code;
  err_log_ptr++;
  err_log_count++;
}

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
    // M5.Speaker.playMusic(m5stack_startup_music,25000);
    /*
    int avg=0;
    for(uint16_t i=0; i<40000; i++) {
      avg+=m5stack_startup_music[i];
      if(i%4 == 3) {
        music_data[i/4]=avg/4;
        avg=0;
      }
    }
    play_music_data(10000, 100);

    for(int i=0; i>=100; i++) {
        M5.Lcd.setBrightness(i);
        delay(2);
    }
    */
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

void play_music_data(uint32_t data_length, uint8_t volume) {
  uint8_t vol;
  if( volume>100 )
    vol=1;
  else
    vol=101-volume;
  if(vol != 101) {
    ledcSetup(TONE_PIN_CHANNEL, 0, 13);
    ledcAttachPin(SPEAKER_PIN, TONE_PIN_CHANNEL);
    delay(10);
    for(int i=0; i<data_length; i++) {
      dacWrite(SPEAKER_PIN, music_data[i]/vol);
      delayMicroseconds(194); // 200 = 1 000 000 microseconds / sample rate 5000
    }
    /* takes too long
    // slowly set DAC to zero from the last value
    for(int t=music_data[data_length-1]; t>=0; t--) {
      dacWrite(SPEAKER_PIN, t);
      delay(2);
    } */
    for(int t = music_data[data_length - 1] / vol; t >= 0; t--) {
      dacWrite(SPEAKER_PIN, t);
      delay(2);
    }
    // dacWrite(SPEAKER_PIN, 0);
    // delay(10);
    ledcAttachPin(SPEAKER_PIN, TONE_PIN_CHANNEL);
    ledcWriteTone(TONE_PIN_CHANNEL, 0);
    CLEAR_PERI_REG_MASK(RTC_IO_PAD_DAC1_REG, RTC_IO_PDAC1_XPD_DAC | RTC_IO_PDAC1_DAC_XPD_FORCE);
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
}

void sndWarning() {
  for(int j=0; j<3; j++) {
    if( cfg.dev_mode )
      play_tone(3000, 100, 1);
    else
      play_tone(3000, 100, cfg.warning_volume);
    delay(300);
  }
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
    // addErrorLog(500);
  }
  if(M5.BtnB.wasPressed()) {
    // M5.Lcd.printf("B");
    Serial.printf("B");
    /*
    play_tone(440, 100, 1);
    delay(10);
    play_tone(880, 100, 1);
    delay(10);
    play_tone(1760, 100, 1);
    */
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
    if(dispPage<MAX_PAGE)
      drawIcon(icon_xpos[1], icon_ypos[1], (uint8_t*)clock_icon16x16, TFT_RED);
  } 
  
  if(M5.BtnC.wasPressed()) {
    // M5.Lcd.printf("C");
    Serial.printf("C");
    unsigned long btnCPressTime = millis();
    long pwrOffTimeout = 4000;
    int lastDispTime = pwrOffTimeout/1000;
    int longPress = 0;
    char tmpstr[32];
    while(M5.BtnC.read()) {
      M5.Lcd.setTextSize(1);
      M5.Lcd.setFreeFont(FSSB12);
      // M5.Lcd.fillRect(110, 220, 100, 20, TFT_RED);
      // M5.Lcd.fillRect(0, 220, 320, 20, TFT_RED);
      M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
      int timeToPwrOff = (pwrOffTimeout - (millis()-btnCPressTime))/1000;
      if((lastDispTime!=timeToPwrOff) && (millis()-btnCPressTime>800)) {
        longPress = 1;
        sprintf(tmpstr, "OFF in %1d   ", timeToPwrOff);
        M5.Lcd.drawString(tmpstr, 210, 220, GFXFF);
        lastDispTime=timeToPwrOff;
      }
      if(timeToPwrOff<=0) {
        // play_tone(3000, 100, 1);
        M5.setWakeupButton(BUTTON_C_PIN);
        M5.powerOFF();
      }
      M5.update();
    }
    if(longPress) {
      M5.Lcd.fillRect(210, 220, 110, 20, TFT_BLACK);
      drawIcon(246, 220, (uint8_t*)door_icon16x16, TFT_LIGHTGREY);
    } else {
      dispPage++;
      if(dispPage>MAX_PAGE)
        dispPage = 0;
      // update_glycemia();
      setPageIconPos(dispPage);
      M5.Lcd.clear(BLACK);
      msCount = millis()-16000;
      // play_tone(440, 100, 1);
    }
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
  for(int i=0; i<=9; i++) {
    if((cfg.wlanssid[i][0]!=0) && (cfg.wlanpass[i][0]!=0))
      WiFiMulti.addAP(cfg.wlanssid[i], cfg.wlanpass[i]);
  }

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
  Serial.print("WiFi connected to SSID "); Serial.println(WiFi.SSID());
  M5.Lcd.print("WiFi SSID "); M5.Lcd.println(WiFi.SSID());
  Serial.println("IP address: ");
  M5.Lcd.println("IP address: ");
  Serial.println(WiFi.localIP());
  M5.Lcd.println(WiFi.localIP());

  configTime(cfg.timeZone, cfg.dst, ntpServer, "time.nist.gov", "time.google.com");
  delay(1000);
  Serial.print("Waiting for time.");
  int i = 0;
  while(!getLocalTime(&localTimeInfo)) {
    Serial.print(".");
    delay(1000);
    i++;
    if (i > MAX_TIME_RETRY) {
      Serial.print("Gave up waiting for time to have a valid value.");
      break;
    }
  }
  Serial.println();
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
    
    // M5.Speaker.mute();

    // Lcd display
    M5.Lcd.setBrightness(100);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(2);
    yield();

    Serial.print("Free Heap: "); Serial.println(ESP.getFreeHeap());

    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        M5.Lcd.println("No SD card attached");
        while(1);
    }

    Serial.print("SD Card Type: ");
    M5.Lcd.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
        M5.Lcd.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
        M5.Lcd.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
        M5.Lcd.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
        M5.Lcd.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %llu MB\n", cardSize);
    M5.Lcd.printf("SD Card Size: %llu MB\n", cardSize);

    readConfiguration(iniFilename, &cfg);
    // strcpy(cfg.url, "user.herokuapp.com");
    // cfg.dev_mode = 0;
    // cfg.show_mgdl = 1;
    // cfg.default_page = 1;
    // strcpy(cfg.restart_at_time, "21:59");
    // cfg.restart_at_logged_errors=3;
    // cfg.show_COB_IOB = 1;
    // cfg.snd_warning = 5.5;
    // cfg.snd_alarm = 4.5;
    // cfg.snd_warning_high = 9;
    // cfg.snd_alarm_high = 14;
    // cfg.alarm_volume = 0;
    // cfg.warning_volume = 0;
    // cfg.snd_warning_at_startup = 1;
    // cfg.snd_alarm_at_startup = 1;
  
    // cfg.alarm_repeat = 1;
    // cfg.snooze_timeout = 2;
    // cfg.brightness1 = 0;
    // cfg.info_line = 1;

    lcdBrightness = cfg.brightness1;
    M5.Lcd.setBrightness(lcdBrightness);
    
    startupLogo();
    yield();

    preferences.begin("M5StackNS", false);
    if(preferences.getBool("SoftReset", false)) {
      // no startup sound after soft reset and remove the SoftReset key
      lastSnoozeTime=preferences.getUInt("LastSnoozeTime", 0);
      preferences.remove("SoftReset");
      preferences.remove("LastSnoozeTime");
    } else {
      // normal startup so decide by M5NS.INI if to play startup sound
      if(cfg.snd_warning_at_startup) {
        play_tone(3000, 100, cfg.warning_volume);
        delay(500);
      }
      if(cfg.snd_alarm_at_startup) {
        play_tone(660, 400, cfg.alarm_volume);
        delay(500);
      }
    }
    preferences.end();

    delay(1000);
    M5.Lcd.fillScreen(BLACK);

    M5.Lcd.setBrightness(lcdBrightness);
    wifi_connect();
    yield();

    M5.Lcd.setBrightness(lcdBrightness);
    M5.Lcd.fillScreen(BLACK);

    // fill dummy values to error log
    /*
    for(int i=0; i<10; i++) {
      getLocalTime(&err_log[i].err_time);
      err_log[i].err_code=404;
    }
    getLocalTime(&err_log[6].err_time);
    err_log[6].err_code=HTTPC_ERROR_CONNECTION_REFUSED;
    err_log_ptr=10;
    err_log_count=23;
    */
    
    // test file with time stamps
    // msCountLog = millis()-6000;

    dispPage = cfg.default_page;
    setPageIconPos(dispPage);
    // stat startup time
    msStart = millis();
    // update glycemia now
    msCount = msStart-16000;
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

struct NSinfo {
  char sensDev[64];
  uint64_t rawtime = 0;
  time_t sensTime = 0;
  struct tm sensTm;
  char sensDir[32];
  float sensSgvMgDl = 0;
  float sensSgv = 0;
  float last10sgv[10];
  bool is_xDrip = 0;  
  int arrowAngle = 180;
  float iob = 0;
  char iob_display[16];
  char iob_displayLine[16];
  float cob = 0;
  char cob_display[16];
  char cob_displayLine[16];
  int delta_absolute = 0;
  float delta_elapsedMins = 0;
  bool delta_interpolated = 0;
  int delta_mean5MinsAgo = 0;
  int delta_mgdl = 0;
  float delta_scaled = 0;
  char delta_display[16];
  char loop_display_symbol = '?';
  char loop_display_code[16];
  char loop_display_label[16];
  char basal_display[16];
  float basal_current = 0;
  float basal_tempbasal = 0;
  float basal_combobolusbasal = 0;
  float basal_totalbasal = 0;
} ns;

void drawMiniGraph(struct NSinfo *ns){
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
    glk = *(ns->last10sgv+9-i);
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
    Serial.print(*(ns->last10sgv+i)); Serial.print(" ");
    if(*(ns->last10sgv+9-i)!=0)
      M5.Lcd.fillCircle(234+i*9, 203-(glk-3.0)*10.0, 3, sgvColor);
  }
  Serial.println();
}

int readNightscout(char *url, char *token, struct NSinfo *ns) {
  HTTPClient http;
  char NSurl[128];
  int err=0;
  char tmpstr[32];
  
  if((WiFiMulti.run() == WL_CONNECTED)) {
    // configure target server and url
    if(strncmp(url, "http", 4))
      strcpy(NSurl,"https://");
    else
      strcpy(NSurl,"");
    strcat(NSurl,url);
    strcat(NSurl,"/api/v1/entries.json?find[type][$eq]=sgv");
    if ((token!=NULL) && (strlen(token)>0)){
      strcat(NSurl,"?token=");
      strcat(NSurl,token);
    }

    M5.Lcd.fillRect(icon_xpos[0], icon_ypos[0], 16, 16, BLACK);
    drawIcon(icon_xpos[0], icon_ypos[0], (uint8_t*)wifi2_icon16x16, TFT_BLUE);
    
    Serial.print("JSON query NSurl = \'");Serial.print(NSurl);Serial.print("\'\n");
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
        Serial.println("JSON deserialized OK");
        JsonArray arr=JSONdoc.as<JsonArray>();
        Serial.print("JSON array size = "); Serial.println(arr.size());
        if (JSONerr || arr.size()==0) {   //Check for errors in parsing
          if(JSONerr) {
            err=1001; // "JSON parsing failed"
          } else {
            err=1002; // "No data from Nightscout"
          }
          addErrorLog(err);
        } else {
          JsonObject obj; 
          int sgvindex = 0;
          do {
            obj=JSONdoc[sgvindex].as<JsonObject>();
            sgvindex++;
          } while ((!obj.containsKey("sgv")) && (sgvindex<(arr.size()-1)));
          sgvindex--;
          if(sgvindex<0 || sgvindex>(arr.size()-1))
            sgvindex=0;
          strlcpy(ns->sensDev, JSONdoc[sgvindex]["device"] | "N/A", 64);
          ns->is_xDrip = obj.containsKey("xDrip_raw");
          ns->rawtime = JSONdoc[sgvindex]["date"].as<long long>(); // sensTime is time in milliseconds since 1970, something like 1555229938118
          strlcpy(ns->sensDir, JSONdoc[sgvindex]["direction"] | "N/A", 32);
          ns->sensSgv = JSONdoc[sgvindex]["sgv"]; // get value of sensor measurement
          ns->sensTime = ns->rawtime / 1000; // no milliseconds, since 2000 would be - 946684800, but ok
          for(int i=0; i<=9; i++) {
            ns->last10sgv[i]=JSONdoc[i]["sgv"];
            ns->last10sgv[i]/=18.0;
          }
          ns->sensSgvMgDl = ns->sensSgv;
          // internally we work in mmol/L
          ns->sensSgv/=18.0;
          
          localtime_r(&ns->sensTime, &ns->sensTm);
          
          ns->arrowAngle = 180;
          if(strcmp(ns->sensDir,"DoubleDown")==0)
            ns->arrowAngle = 90;
          else 
            if(strcmp(ns->sensDir,"SingleDown")==0)
              ns->arrowAngle = 75;
            else 
                if(strcmp(ns->sensDir,"FortyFiveDown")==0)
                  ns->arrowAngle = 45;
                else 
                    if(strcmp(ns->sensDir,"Flat")==0)
                      ns->arrowAngle = 0;
                    else 
                        if(strcmp(ns->sensDir,"FortyFiveUp")==0)
                          ns->arrowAngle = -45;
                        else 
                            if(strcmp(ns->sensDir,"SingleUp")==0)
                              ns->arrowAngle = -75;
                            else 
                                if(strcmp(ns->sensDir,"DoubleUp")==0)
                                  ns->arrowAngle = -90;
                                else 
                                    if(strcmp(ns->sensDir,"NONE")==0)
                                      ns->arrowAngle = 180;
                                    else 
                                        if(strcmp(ns->sensDir,"NOT COMPUTABLE")==0)
                                          ns->arrowAngle = 180;
                                          
          Serial.print("sensDev = ");
          Serial.println(ns->sensDev);
          Serial.print("sensTime = ");
          Serial.print(ns->sensTime);
          sprintf(tmpstr, " (JSON %lld)", (long long) ns->rawtime);
          Serial.print(tmpstr);
          sprintf(tmpstr, " = %s", ctime(&ns->sensTime));
          Serial.print(tmpstr);
          Serial.print("sensSgv = ");
          Serial.println(ns->sensSgv);
          Serial.print("sensDir = ");
          Serial.println(ns->sensDir);
          // Serial.print(ns->sensTm.tm_year+1900); Serial.print(" / "); Serial.print(ns->sensTm.tm_mon+1); Serial.print(" / "); Serial.println(ns->sensTm.tm_mday);
          Serial.print("Sensor time: "); Serial.print(ns->sensTm.tm_hour); Serial.print(":"); Serial.print(ns->sensTm.tm_min); Serial.print(":"); Serial.print(ns->sensTm.tm_sec); Serial.print(" DST "); Serial.println(ns->sensTm.tm_isdst);
        } 
      } else {
        addErrorLog(httpCode);
        err=httpCode;
      }
    } else {
      addErrorLog(httpCode);
      err=httpCode;
    }
    http.end();

    if(err!=0)
      return err;
      
    // the second query 
    if(strncmp(url, "http", 4))
      strcpy(NSurl,"https://");
    else
      strcpy(NSurl,"");
    strcat(NSurl,url);
    strcat(NSurl,"/api/v2/properties/iob,cob,delta,loop,basal");
    if (strlen(token) > 0){
      strcat(NSurl,"&token=");
      strcat(NSurl,token);
    }
    
    M5.Lcd.fillRect(icon_xpos[0], icon_ypos[0], 16, 16, BLACK);
    drawIcon(icon_xpos[0], icon_ypos[0], (uint8_t*)wifi1_icon16x16, TFT_BLUE);

    Serial.print("Properties query NSurl = \'");Serial.print(NSurl);Serial.print("\'\n");
    http.begin(NSurl); //HTTP
    Serial.print("[HTTP] GET properties...\n");
    httpCode = http.GET();
    if(httpCode > 0) {
      Serial.printf("[HTTP] GET properties... code: %d\n", httpCode);
      if(httpCode == HTTP_CODE_OK) {
        // const char* propjson = "{\"iob\":{\"iob\":0,\"activity\":0,\"source\":\"OpenAPS\",\"device\":\"openaps://Spike iPhone 8 Plus\",\"mills\":1557613521000,\"display\":\"0\",\"displayLine\":\"IOB: 0U\"},\"cob\":{\"cob\":0,\"source\":\"OpenAPS\",\"device\":\"openaps://Spike iPhone 8 Plus\",\"mills\":1557613521000,\"treatmentCOB\":{\"decayedBy\":\"2019-05-11T23:05:00.000Z\",\"isDecaying\":0,\"carbs_hr\":20,\"rawCarbImpact\":0,\"cob\":7,\"lastCarbs\":{\"_id\":\"5cd74c26156712edb4b32455\",\"enteredBy\":\"Martin\",\"eventType\":\"Carb Correction\",\"reason\":\"\",\"carbs\":7,\"duration\":0,\"created_at\":\"2019-05-11T22:24:00.000Z\",\"mills\":1557613440000,\"mgdl\":67}},\"display\":0,\"displayLine\":\"COB: 0g\"},\"delta\":{\"absolute\":-4,\"elapsedMins\":4.999483333333333,\"interpolated\":false,\"mean5MinsAgo\":69,\"mgdl\":-4,\"scaled\":-0.2,\"display\":\"-0.2\",\"previous\":{\"mean\":69,\"last\":69,\"mills\":1557613221946,\"sgvs\":[{\"mgdl\":69,\"mills\":1557613221946,\"device\":\"MIAOMIAO\",\"direction\":\"Flat\",\"filtered\":92588,\"unfiltered\":92588,\"noise\":1,\"rssi\":100}]}}}";
        String propjson = http.getString();
        auto propJSONerr = deserializeJson(JSONdoc, propjson);
        if(propJSONerr) {
          err=1003; // "JSON2 parsing failed"
          addErrorLog(err);
        } else {
          Serial.println("Deserialized the second JSON and OK");
          JsonObject iob = JSONdoc["iob"];
          ns->iob = iob["iob"]; // 0
          strncpy(ns->iob_display, iob["display"] | "N/A", 16); // 0
          strncpy(ns->iob_displayLine, iob["displayLine"] | "IOB: N/A", 16); // "IOB: 0U"
          // Serial.println("IOB OK");
          
          JsonObject cob = JSONdoc["cob"];
          ns->cob = cob["cob"]; // 0
          strncpy(ns->cob_display, cob["display"] | "N/A", 16); // 0
          strncpy(ns->cob_displayLine, cob["displayLine"] | "COB: N/A", 16); // "COB: 0g"
          // Serial.println("COB OK");
          
          JsonObject delta = JSONdoc["delta"];
          ns->delta_absolute = delta["absolute"]; // -4
          ns->delta_elapsedMins = delta["elapsedMins"]; // 4.999483333333333
          ns->delta_interpolated = delta["interpolated"]; // false
          ns->delta_mean5MinsAgo = delta["mean5MinsAgo"]; // 69
          ns->delta_mgdl = delta["mgdl"]; // -4
          ns->delta_scaled = delta["scaled"]; // -0.2
          strncpy(ns->delta_display, delta["display"] | "", 16); // "-0.2"
          // Serial.println("DELTA OK");
          
          JsonObject loop_obj = JSONdoc["loop"];
          JsonObject loop_display = loop_obj["display"];
          strncpy(tmpstr, loop_display["symbol"] | "?", 4); // "⌁"
          ns->loop_display_symbol = tmpstr[0];
          strncpy(ns->loop_display_code, loop_display["code"] | "N/A", 16); // "enacted"
          strncpy(ns->loop_display_label, loop_display["label"] | "N/A", 16); // "Enacted"
          // Serial.println("LOOP OK");

          JsonObject basal = JSONdoc["basal"];
          strncpy(ns->basal_display, basal["display"] | "N/A", 16); // "T: 0.950U"      
          // Serial.println("BASAL OK");
          
          JsonObject basal_current_doc = JSONdoc["basal"]["current"];
          ns->basal_current = basal_current_doc["basal"]; // 0.1
          ns->basal_tempbasal = basal_current_doc["tempbasal"]; // 0.1
          ns->basal_combobolusbasal = basal_current_doc["combobolusbasal"]; // 0
          ns->basal_totalbasal = basal_current_doc["totalbasal"]; // 0.1
          // Serial.println("LOOP OK");
        } 
      } else {
        addErrorLog(httpCode);
        err=httpCode;
      }
    } else {
      addErrorLog(httpCode);
      err=httpCode;
    }
    http.end();
  } else {
    // WiFi not connected
    ESP.restart();
  }

  M5.Lcd.fillRect(icon_xpos[0], icon_ypos[0], 16, 16, BLACK);

  return err;
}

void drawBatteryStatus(int16_t x, int16_t y) {
  int8_t battLevel = getBatteryLevel();
  Serial.print("Battery level: "); Serial.println(battLevel);
  M5.Lcd.fillRect(x, y, 16, 17, TFT_BLACK);
  if(battLevel!=-1) {
    switch(battLevel) {
      case 0:
        drawIcon(x, y+1, (uint8_t*)bat0_icon16x16, TFT_RED);
        break;
      case 25:
        drawIcon(x, y+1, (uint8_t*)bat1_icon16x16, TFT_YELLOW);
        break;
      case 50:
        drawIcon(x, y+1, (uint8_t*)bat2_icon16x16, TFT_WHITE);
        break;
      case 75:
        drawIcon(x, y+1, (uint8_t*)bat3_icon16x16, TFT_LIGHTGREY);
        break;
      case 100:
        drawIcon(x, y+0, (uint8_t*)plug_icon16x16, TFT_LIGHTGREY);
        break;
    }
  }
}

void handleAlarmsInfoLine(struct NSinfo *ns) {
  struct tm timeinfo;

  // calculate sensor time difference
  // calculate last alarm time difference
  int sensorDifSec=24*60*60; // too much
  int alarmDifSec=24*60*60; // too much
  int snoozeDifSec=cfg.snooze_timeout*60; // timeout
  if(getLocalTime(&timeinfo)){
    sensorDifSec=difftime(mktime(&timeinfo), ns->sensTime);
    alarmDifSec=difftime(mktime(&timeinfo), lastAlarmTime);
    snoozeDifSec=difftime(mktime(&timeinfo), lastSnoozeTime);
    if( snoozeDifSec>cfg.snooze_timeout*60 )
      snoozeDifSec=cfg.snooze_timeout*60; // timeout
  }
  unsigned int sensorDifMin = (sensorDifSec+30)/60;
  
  Serial.print("Alarm time difference = "); Serial.print(alarmDifSec); Serial.println(" sec");
  Serial.print("Snooze time difference = "); Serial.print(snoozeDifSec); Serial.println(" sec");
  char tmpStr[10];
  if( snoozeDifSec<cfg.snooze_timeout*60 ) {
    sprintf(tmpStr, "%i", (cfg.snooze_timeout*60-snoozeDifSec+59)/60);
    if(dispPage<MAX_PAGE)
      drawIcon(icon_xpos[1], icon_ypos[1], (uint8_t*)clock_icon16x16, TFT_RED);
  } else {
    strcpy(tmpStr, "Snooze");
    if(dispPage<MAX_PAGE)
      M5.Lcd.fillRect(icon_xpos[1], icon_ypos[1], 16, 16, BLACK);
  }
  M5.Lcd.setTextSize(1);
  M5.Lcd.setFreeFont(FSSB12);
  // Serial.print("sensSgv="); Serial.print(sensSgv); Serial.print(", cfg.snd_alarm="); Serial.println(cfg.snd_alarm); 
  if((ns->sensSgv<=cfg.snd_alarm) && (ns->sensSgv>=0.1)) {
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
    if((ns->sensSgv<=cfg.snd_warning) && (ns->sensSgv>=0.1)) {
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
      if( ns->sensSgv>=cfg.snd_alarm_high ) {
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
        if( ns->sensSgv>=cfg.snd_warning_high ) {
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
                strcpy(infoStr, ns->sensDev);
                if(strcmp(infoStr,"MIAOMIAO")==0) {
                  if(ns->is_xDrip) {
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
                drawIcon(58, 220, (uint8_t*)sun_icon16x16, TFT_LIGHTGREY);
                drawIcon(153, 220, (uint8_t*)clock_icon16x16, TFT_LIGHTGREY);
                // drawIcon(153, 220, (uint8_t*)timer_icon16x16, TFT_LIGHTGREY);
                drawIcon(246, 220, (uint8_t*)door_icon16x16, TFT_LIGHTGREY);
                break;
              case 2: // loop + basal information
                strcpy(infoStr, "L: ");
                strlcat(infoStr, ns->loop_display_label, 64);
                M5.Lcd.drawString(infoStr, 0, 220, GFXFF);
                strcpy(infoStr, "B: ");
                strlcat(infoStr, ns->basal_display, 64);
                M5.Lcd.drawString(infoStr, 160, 220, GFXFF);
                break;
            }
          }
        }
      }
    }
  }
}

void drawLogWarningIcon() {
  if(err_log_ptr>5)
    drawIcon(icon_xpos[0], icon_ypos[0], (uint8_t*)warning_icon16x16, TFT_YELLOW);
  else
    if(err_log_ptr>0)
      drawIcon(icon_xpos[0], icon_ypos[0], (uint8_t*)warning_icon16x16, TFT_LIGHTGREY);
    else
      M5.Lcd.fillRect(icon_xpos[0], icon_ypos[0], 16, 16, BLACK);
}

void update_glycemia() {
  char tmpstr[255];
  
  M5.Lcd.setTextDatum(TL_DATUM);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(0, 0);

  switch(dispPage) {
    case 0: {
      // if there was an error, then clear whole screen, otherwise only graphic updated part
      // M5.Lcd.fillScreen(BLACK);
      // M5.Lcd.fillRect(230, 110, 90, 100, TFT_BLACK);
      // M5.Lcd.drawBitmap(242, 130, 64, 48, (uint16_t *)WiFi_symbol);
      
      readNightscout(cfg.url, cfg.token, &ns);

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
          // timeinfo.tm_mday=24; timeinfo.tm_mon=11;
          sprintf(datetimeStr, "%02d:%02d  %d.%d.  ", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_mday, timeinfo.tm_mon+1);  
        } else {
          // strcpy(datetimeStr, "??:??:??");
          strcpy(datetimeStr, "??:??");
        }
      } else {
        // sprintf(datetimeStr, "%02d:%02d:%02d  %d.%d.  ", sensTm.tm_hour, sensTm.tm_min, sensTm.tm_sec, sensTm.tm_mday, sensTm.tm_mon+1);
        sprintf(datetimeStr, "%02d:%02d  %d.%d.  ", ns.sensTm.tm_hour, ns.sensTm.tm_min, ns.sensTm.tm_mday, ns.sensTm.tm_mon+1);
      }
      M5.Lcd.drawString(datetimeStr, 0, 0, GFXFF);

      drawBatteryStatus(icon_xpos[2], icon_ypos[2]);
      
      if(err_log_ptr>0) {
        M5.Lcd.fillRect(icon_xpos[0], icon_ypos[0], 16, 16, BLACK);
        if(err_log_ptr>5)
          drawIcon(icon_xpos[0], icon_ypos[0], (uint8_t*)warning_icon16x16, TFT_YELLOW);
        else
          drawIcon(icon_xpos[0], icon_ypos[0], (uint8_t*)warning_icon16x16, TFT_LIGHTGREY);
      }
              
      M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
      M5.Lcd.drawString(cfg.userName, 0, 24, GFXFF);
      
      if(cfg.show_COB_IOB) {
        M5.Lcd.setFreeFont(FSSB12);
        M5.Lcd.setTextSize(1);
        // show small delta right from name
        /*
        M5.Lcd.setFreeFont(FSSB12);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.setTextSize(1);
        M5.Lcd.fillRect(130,24,69,23,TFT_BLACK);
        M5.Lcd.drawString(ns.delta_display, 130, 24, GFXFF);
        */
        
        M5.Lcd.fillRect(0,48,199,47,TFT_BLACK);
        if(ns.iob>0)
          M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        else
          M5.Lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        Serial.print("ns.iob_displayLine=\""); Serial.print(ns.iob_displayLine); Serial.println("\"");
        strncpy(tmpstr, ns.iob_displayLine, 16);
        if(strncmp(tmpstr, "IOB:", 4)==0) {
          strcpy(tmpstr,"I");
          strcat(tmpstr, &ns.iob_displayLine[3]);
        }
        M5.Lcd.drawString(tmpstr, 0, 48, GFXFF);
        if(ns.cob>0)
          M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        else
          M5.Lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        Serial.print("ns.cob_displayLine=\""); Serial.print(ns.cob_displayLine); Serial.println("\"");
        strncpy(tmpstr, ns.cob_displayLine, 16);
        if(strncmp(tmpstr, "COB:", 4)==0) {
          strcpy(tmpstr,"C");
          strcat(tmpstr, &ns.cob_displayLine[3]);
        }
        M5.Lcd.drawString(tmpstr, 0, 72, GFXFF);

        // show BIG delta bellow the name
        M5.Lcd.setFreeFont(FSSB24);
        // strcpy(ns.delta_display, "+8.9");
        if(ns.delta_mgdl>7)
          M5.Lcd.setTextColor(TFT_WHITE, BLACK);
        else
          M5.Lcd.setTextColor(TFT_LIGHTGREY, BLACK);
        M5.Lcd.drawString(ns.delta_display, 103, 48, GFXFF);
        M5.Lcd.setFreeFont(FSSB12);

      } else {
        // show BIG delta bellow the name
        M5.Lcd.setFreeFont(FSSB24);
        M5.Lcd.setTextColor(TFT_LIGHTGREY, BLACK);
        M5.Lcd.setTextSize(1);
        M5.Lcd.fillRect(0,48+10,199,47,TFT_BLACK);
        M5.Lcd.drawString(ns.delta_display, 0, 48+10, GFXFF);
        M5.Lcd.setFreeFont(FSSB12);
      }

      // calculate sensor time difference
      int sensorDifSec=0;
      if(!getLocalTime(&timeinfo)){
        sensorDifSec=24*60*60; // too much
      } else {
        Serial.print("Local time: "); Serial.print(timeinfo.tm_hour); Serial.print(":"); Serial.print(timeinfo.tm_min); Serial.print(":"); Serial.print(timeinfo.tm_sec); Serial.print(" DST "); Serial.println(timeinfo.tm_isdst);
        sensorDifSec=difftime(mktime(&timeinfo), ns.sensTime);
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
      if(ns.sensSgv<cfg.yellow_low || ns.sensSgv>cfg.yellow_high) {
        glColor=TFT_YELLOW; // warning is YELLOW
      }
      if(ns.sensSgv<cfg.red_low || ns.sensSgv>cfg.red_high) {
        glColor=TFT_RED; // alert is RED
      }
    
      sprintf(tmpstr, "Glyk: %4.1f %s", ns.sensSgv, ns.sensDir);
      Serial.println(tmpstr);
      
      M5.Lcd.fillRect(0, 110, 320, 114, TFT_BLACK);
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextDatum(TL_DATUM);
      M5.Lcd.setTextColor(glColor, TFT_BLACK);
      char sensSgvStr[30];
      int smaller_font = 0;
      if( cfg.show_mgdl ) {
        sprintf(sensSgvStr, "%3.0f", ns.sensSgvMgDl);
      } else {
        sprintf(sensSgvStr, "%4.1f", ns.sensSgv);
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
    
      if(ns.arrowAngle!=180)
        drawArrow(0+tw+25, 120+40, 10, ns.arrowAngle+85, 40, 40, glColor);
    
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

      drawMiniGraph(&ns);
      handleAlarmsInfoLine(&ns);
      drawLogWarningIcon();
    }
    break;
    
    case 1: {
      readNightscout(cfg.url, cfg.token, &ns);
      
      uint16_t glColor = TFT_GREEN;
      if(ns.sensSgv<cfg.yellow_low || ns.sensSgv>cfg.yellow_high) {
        glColor=TFT_YELLOW; // warning is YELLOW
      }
      if(ns.sensSgv<cfg.red_low || ns.sensSgv>cfg.red_high) {
        glColor=TFT_RED; // alert is RED
      }
    
      sprintf(tmpstr, "Glyk: %4.1f %s", ns.sensSgv, ns.sensDir);
      Serial.println(tmpstr);
      
      M5.Lcd.fillRect(0, 40, 320, 180, TFT_BLACK);
      M5.Lcd.setTextSize(4);
      M5.Lcd.setTextDatum(MC_DATUM);
      M5.Lcd.setTextColor(glColor, TFT_BLACK);
      char sensSgvStr[30];
      int smaller_font = 0;
      if( cfg.show_mgdl ) {
        if(ns.sensSgvMgDl<100) {
          sprintf(sensSgvStr, "%2.0f", ns.sensSgvMgDl);
          M5.Lcd.setFreeFont(FSSB24);
        } else {
          sprintf(sensSgvStr, "%3.0f", ns.sensSgvMgDl);
          M5.Lcd.setFreeFont(FSSB24);
        }
      } else {
        if(ns.sensSgv<10) {
          sprintf(sensSgvStr, "%3.1f", ns.sensSgv);
          M5.Lcd.setFreeFont(FSSB24);
        } else {
          sprintf(sensSgvStr, "%4.1f", ns.sensSgv);
          M5.Lcd.setFreeFont(FSSB18);
        }
      }
      M5.Lcd.drawString(sensSgvStr, 160, 120, GFXFF);
    
      M5.Lcd.fillRect(0, 0, 320, 40, TFT_BLACK);
      M5.Lcd.setFreeFont(FSSB24);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextDatum(TL_DATUM);
      M5.Lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);

      char datetimeStr[30];
      struct tm timeinfo;
      if(cfg.show_current_time) {
        if(getLocalTime(&timeinfo)) {
          sprintf(datetimeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);  
        } else {
          strcpy(datetimeStr, "??:??");
        }
      } else {
        sprintf(datetimeStr, "%02d:%02d", ns.sensTm.tm_hour, ns.sensTm.tm_min);
      }
      M5.Lcd.drawString(datetimeStr, 0, 0, GFXFF);
      
      M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
      M5.Lcd.drawString(ns.delta_display, 180, 0, GFXFF);

      int ay=0;

      // for(ns.arrowAngle=-90; ns.arrowAngle<=90; ns.arrowAngle+=15)

      if(ns.arrowAngle>=45)
        ay=4;
      else
        if(ns.arrowAngle>-45)
          ay=18;
        else
          ay=30;
      
      if(ns.arrowAngle!=180)
        drawArrow(280, ay, 10, ns.arrowAngle+85, 28, 28, glColor);
 
      handleAlarmsInfoLine(&ns);
      drawBatteryStatus(icon_xpos[2], icon_ypos[2]);
      drawLogWarningIcon();
    }
    break;
   
    case MAX_PAGE: {
      // display error log
      char tmpStr[32];
      HTTPClient http;
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0, 18);
      M5.Lcd.setTextDatum(TL_DATUM);
      M5.Lcd.setFreeFont(FMB9);
      M5.Lcd.setTextSize(1); 
      M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
      M5.Lcd.drawString("Date  Time  Error Log", 0, 0, GFXFF);
      // M5.Lcd.drawString("Error", 143, 0, GFXFF);
      M5.Lcd.setFreeFont(FM9);
      if(err_log_ptr==0) {
        M5.Lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        M5.Lcd.drawString("no errors in log", 0, 20, GFXFF);
      } else {
        for(int i=0; i<err_log_ptr; i++) {
          M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
          sprintf(tmpStr, "%02d.%02d.%02d:%02d", err_log[i].err_time.tm_mday, err_log[i].err_time.tm_mon+1, err_log[i].err_time.tm_hour, err_log[i].err_time.tm_min);
          M5.Lcd.drawString(tmpStr, 0, 20+i*18, GFXFF);
          if(err_log[i].err_code<0) {
            M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
            strlcpy(tmpStr, http.errorToString(err_log[i].err_code).c_str(), 32);
          } else {
            M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
            switch(err_log[i].err_code) {
              case 1001:
                strcpy(tmpStr, "JSON parsing failed");
                break;
              case 1002:
                strcpy(tmpStr, "No data from Nightscout");
                break;
              case 1003:
                strcpy(tmpStr, "JSON2 parsing failed");
                break;
              default:              
                sprintf(tmpStr, "HTTP error %d", err_log[i].err_code);
            }
          }
          M5.Lcd.drawString(tmpStr, 132, 20+i*18, GFXFF);
        }
        M5.Lcd.setFreeFont(FMB9);
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        sprintf(tmpStr, "Total errors %d", err_log_count);
        M5.Lcd.drawString(tmpStr, 0, 20+err_log_ptr*18, GFXFF);
      }
      handleAlarmsInfoLine(&ns);
    }
    break;
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
    Serial.print("msCount = "); Serial.println(msCount);
  } else {
    if((cfg.restart_at_logged_errors>0) && (err_log_count>=cfg.restart_at_logged_errors)) {
      preferences.begin("M5StackNS", false);
      preferences.putBool("SoftReset", true);
      preferences.putUInt("LastSnoozeTime", lastSnoozeTime);
      preferences.end();
      ESP.restart();
    }
    char lastResetTime[10];
    strcpy(lastResetTime, "Unknown");
    if(getLocalTime(&localTimeInfo)) {
      sprintf(localTimeStr, "%02d:%02d", localTimeInfo.tm_hour, localTimeInfo.tm_min);
      // no soft restart less than a minute from last restart to prevent several restarts in the same minute
      if((millis()-msStart>60000) && (strcmp(cfg.restart_at_time, localTimeStr)==0)) {
        preferences.begin("M5StackNS", false);
        preferences.putBool("SoftReset", true);
        preferences.putUInt("LastSnoozeTime", lastSnoozeTime);
        preferences.end();
        ESP.restart();
      }
    }
    if((dispPage==0) && cfg.show_current_time) {
      // update current time on display
      M5.Lcd.setFreeFont(FSSB12);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      if(getLocalTime(&localTimeInfo)) {
        sprintf(localTimeStr, "%02d:%02d  %d.%d.  ", localTimeInfo.tm_hour, localTimeInfo.tm_min, localTimeInfo.tm_mday, localTimeInfo.tm_mon+1);  
      } else {
        strcpy(localTimeStr, "??:??");
        lastMin = 61;
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

  // Serial.println("M5.update() and loop again");
  M5.update();
}
