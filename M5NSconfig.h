#ifndef _M5NSCONFIG_H
#define _M5NSCONFIG_H

#include <M5Stack.h>
#include "IniFile.h"

struct tConfig {
  char url[64];
  char bootPic[64];
  char userName[32];
  int timeZone = 3600; // time zone offset in hours, must be corrected for internatinal use and DST
  int dst = 0; // DST time offset in hours, must be corrected for internatinal use and DST
  int show_mgdl = 0; // 0 = display mg/DL, 1 = diplay mmol/L
  int show_current_time = 0;
  int show_COB_IOB = 0;
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
  char warning_music[64];
  int warning_volume = 30;
  char alarm_music[64];
  int alarm_volume = 100;
  int info_line = 1; // 0 = sensor info, 1 = button function icons, 2 = loop info + basal
  uint8_t brightness1, brightness2, brightness3;
  int dev_mode = 0; // developer mode, do not use, does strange things and changes often ;-)
  char wlan1ssid[32];
  char wlan1pass[32];
  char wlan2ssid[32];
  char wlan2pass[32];
  char wlan3ssid[32];
  char wlan3pass[32];
} ;

void readConfiguration(char *iniFilename, tConfig *cfg);

#endif
