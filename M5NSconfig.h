#ifndef _M5NSCONFIG_H
#define _M5NSCONFIG_H

#include <M5Stack.h>
#include "IniFile.h"

struct tConfig {
  char url[128];
  char token[32]; // security token
  char bootPic[64];
  char userName[32];
  int timeZone = 3600; // time zone offset in hours, must be corrected for internatinal use and DST
  int dst = 0; // DST time offset in hours, must be corrected for internatinal use and DST
  int show_mgdl = 0; // 0 = display mg/DL, 1 = diplay mmol/L
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
  int dev_mode = 0; // developer mode, do not use, does strange things and changes often ;-)
  char wlanssid[10][32];
  char wlanpass[10][32];
} ;

void readConfiguration(char *iniFilename, tConfig *cfg);

#endif
