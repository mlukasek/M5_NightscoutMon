#ifndef _M5NSCONFIG_H
#define _M5NSCONFIG_H

#include <M5Stack.h>
#include "IniFile.h"

struct tConfig {
  char url[128];
  char token[32]; // security token
  char bootPic[64];
  char userName[32];
  char deviceName[32];
  int timeZone = 3600; // time zone offset in hours, must be corrected for internatinal use and DST
  int dst = 0; // DST time offset in hours, must be corrected for internatinal use and DST
  int show_mgdl = 0; // 0 = display mg/DL, 1 = diplay mmol/L
  int sgv_only = 0; // 1 = filter only SGV values from Nightscout, 0 = read everything (default)
  int default_page = 0; // page number displayed after startup
  char restart_at_time[10]; // time in HH:MM format when the device will restart
  int restart_at_logged_errors = 0; // restart device after particular number of errors in the log (0 = do not restart)
  int show_current_time = 0; // show currnet time instead of last valid data time_
  int show_COB_IOB = 0; // show COB and IOB, values are grayed out if COB/IOB value is 0
  int snooze_timeout = 30; // timeout to snooze alarm in minutes
  int alarm_repeat = 5; // repeat alarm every X minutes
  float yellow_low = 4.5;
  float yellow_high = 9;
  float red_low = 3.9;
  float red_high = 11;
  float snd_warning = 3.7;
  float snd_alarm = 3.0;
  float snd_warning_high = 14;
  float snd_alarm_high = 20;
  int snd_no_readings = 20; 
  int snd_warning_at_startup = 1;
  int snd_alarm_at_startup =0;
  char warning_music[64];
  int warning_volume = 30;
  char alarm_music[64];
  int alarm_volume = 100;
  int info_line = 1; // 0 = sensor info, 1 = button function icons, 2 = loop info + basal
  uint8_t brightness1, brightness2, brightness3;
  int date_format = 0; // 0 = dd.mm., 1 = MM/DD
  int display_rotation = 1; // 1 = buttons down, 3 = buttons up, 5 = mirror buttons up, 7 = mirror buttons down
  int invert_display = -1; // -1 = not defined, no not call M5.Lcd.invertDisplay(), 0 or 1 call M5.Lcd.invertDisplay(parameter value)
  int temperature_unit = 1; // 1 = CELSIUS, 2 = KELVIN, 3= FAHRENHEIT
  int dev_mode = 0; // developer mode, do not use, does strange things and changes often ;-)
  int disable_web_server = 0; // 1 = disable internal web server
  char wlanssid[10][32];
  char wlanpass[10][64];
} ;

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
  bool is_Sugarmate = 0;  
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
} ;

void readConfiguration(const char *iniFilename, tConfig *cfg);

#endif
