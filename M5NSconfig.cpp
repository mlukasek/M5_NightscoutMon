#include "M5NSconfig.h"
#include <Preferences.h>

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

void readConfigFromFlash(tConfig *cfg) {
  Serial.println("Reading configuration from flash.");
  Preferences prefs;
  if( !prefs.begin("M5NSconfig", false) ) {
    Serial.println("Error opening Preferences M5NSconfig");
    M5.Lcd.println("No config in flash");
    Serial.println("No configuration found in Preferences");
  } else {
    Serial.println("Reading configuration from Preferences M5NSconfig");
    prefs.getString("nightscout", cfg->url, 128);
    prefs.getString("token", cfg->token, 64);
    prefs.getString("bootpic", cfg->bootPic, 64);
    prefs.getString("user_name", cfg->userName, 32);
    if(strlen(cfg->userName)==0)
      strcpy(cfg->userName, " ");
    prefs.getString("device_name", cfg->deviceName, 32);
    if(strlen(cfg->deviceName)==0)
      strcpy(cfg->deviceName, "M5NS");
    cfg->timeZone = prefs.getInt("time_zone", 3600);
    cfg->dst = prefs.getInt("dst", 0);
    cfg->show_mgdl = prefs.getInt("show_mgdl", 0);
    cfg->show_current_time = prefs.getInt("show_cur_time", 1);
    cfg->show_COB_IOB = prefs.getInt("show_COB_IOB", 0);
    cfg->default_page = prefs.getInt("default_page", 0);
    prefs.getString("restart_at_time", cfg->restart_at_time, 10);
    if(strlen(cfg->restart_at_time)==0)
      strcpy(cfg->restart_at_time, "03:30");
    cfg->restart_at_logged_errors = prefs.getInt("restart_log_err", 30);
    cfg->snooze_timeout = prefs.getInt("snooze_timeout", 30);
    cfg->alarm_repeat = prefs.getInt("alarm_repeat", 5);
    cfg->yellow_low = prefs.getFloat("yellow_low", 4.5);
    cfg->yellow_high = prefs.getFloat("yellow_high", 9.0);
    cfg->red_low = prefs.getFloat("red_low", 3.9);
    cfg->red_high = prefs.getFloat("red_high", 11.0);
    cfg->snd_warning = prefs.getFloat("snd_warning", 3.7);
    cfg->snd_alarm = prefs.getFloat("snd_alarm", 3.0);
    cfg->snd_warning_high = prefs.getFloat("snd_warn_high", 14.0);
    // Serial.printf("READ CFG Flash: cfg->snd_warning_high = %f\r\n", cfg->snd_warning_high);
    cfg->snd_alarm_high = prefs.getFloat("snd_alarm_high", 20.0);
    cfg->snd_no_readings = prefs.getInt("snd_no_readings", 20);
    cfg->snd_loop_error = prefs.getInt("snd_loop_error", 1);
    cfg->snd_warning_at_startup = prefs.getInt("snd_warn_start", 1);
    cfg->snd_alarm_at_startup = prefs.getInt("snd_alarm_start", 0);
    cfg->warning_volume = prefs.getInt("warning_volume", 30);
    cfg->alarm_volume = prefs.getInt("alarm_volume", 100);
    cfg->brightness1 = prefs.getInt("brightness1", 50);
    cfg->brightness2 = prefs.getInt("brightness2", 100);
    cfg->brightness3 = prefs.getInt("brightness3", 10);
    cfg->sgv_only = prefs.getInt("sgv_only", 0);
    cfg->info_line = prefs.getInt("info_line", 1);
    cfg->date_format = prefs.getInt("date_format", 0);
    cfg->time_format = prefs.getInt("time_format", 0);
    cfg->display_rotation = prefs.getInt("display_rotate", 1);
    cfg->invert_display = prefs.getInt("invert_display", -1);
    cfg->temperature_unit = prefs.getInt("temperat_unit", 1);
    cfg->LED_strip_mode = prefs.getInt("LED_strip_mode", 0);
    cfg->LED_strip_pin = prefs.getInt("LED_strip_pin", 15);
    cfg->LED_strip_count = prefs.getInt("LED_strip_count", 10);
    cfg->LED_strip_brightness = prefs.getInt("LED_strip_brigh", 10);
    cfg->vibration_mode = prefs.getInt("vibration_mode", 0);
    cfg->vibration_pin = prefs.getInt("vibration_pin", 26);
    cfg->vibration_strength = prefs.getInt("vibration_stren", 512);
    cfg->micro_dot_pHAT = prefs.getInt("micro_dot_pHAT", 0);
    cfg->disable_web_server = prefs.getInt("dis_web_server", 0);
    cfg->dev_mode = prefs.getInt("developer_mode", 0);
    char tmps[64];
    int wlans_defined_count = 0;
    for(int i=0; i<10; i++) {
      sprintf(tmps, "SSID%01d", i);
      if(prefs.getString(tmps, cfg->wlanssid[i], 64) > 0) {
        // Serial.print("Getting key ["); Serial.print(tmps); Serial.println("]");
        sprintf(tmps, "PASS%01d", i);
        wlans_defined_count++;
        if(prefs.getString(tmps, cfg->wlanpass[i], 64) > 0) {
          // Serial.print("Getting key ["); Serial.print(tmps); Serial.println("]");
        } else {
          cfg->wlanpass[i][0] = 0;
        }
      } else {
        cfg->wlanssid[i][0] = 0;
      }
    }
    prefs.end();
    if (wlans_defined_count < 1)
      cfg->is_task_bootstrapping = 1;
    Serial.println("New settings read from flash.");
  }
}

void saveConfigToFlash(tConfig *cfg) {
  Serial.println("Storing configuration to flash.");
  Preferences prefs;
  if( !prefs.begin("M5NSconfig", false) ) {
    Serial.println("Error opening Preferences M5NSconfig");
  } else {
    Serial.println("Writing configuration to Preferences M5NSconfig");
    prefs.clear();
    prefs.putString("nightscout", cfg->url);
    prefs.putString("token", cfg->token);
    prefs.putString("bootpic", cfg->bootPic);
    prefs.putString("user_name", cfg->userName);
    prefs.putString("device_name", cfg->deviceName);
    prefs.putInt("time_zone", cfg->timeZone);
    prefs.putInt("dst", cfg->dst);
    prefs.putInt("show_mgdl", cfg->show_mgdl);
    prefs.putInt("show_cur_time", cfg->show_current_time);
    prefs.putInt("show_COB_IOB", cfg->show_COB_IOB);
    prefs.putInt("default_page", cfg->default_page);
    prefs.putString("restart_at_time", cfg->restart_at_time);
    prefs.putInt("restart_log_err", cfg->restart_at_logged_errors);
    prefs.putInt("snooze_timeout", cfg->snooze_timeout);
    prefs.putInt("alarm_repeat", cfg->alarm_repeat);
    prefs.putFloat("yellow_low", cfg->yellow_low);
    prefs.putFloat("yellow_high", cfg->yellow_high);
    prefs.putFloat("red_low", cfg->red_low);
    prefs.putFloat("red_high", cfg->red_high);
    prefs.putFloat("snd_warning", cfg->snd_warning);
    prefs.putFloat("snd_alarm", cfg->snd_alarm);
    // Serial.printf("SAVE CFG Flash: cfg->snd_warning_high = %f\r\n", cfg->snd_warning_high);
    prefs.putFloat("snd_warn_high", cfg->snd_warning_high);
    prefs.putFloat("snd_alarm_high", cfg->snd_alarm_high);
    prefs.putInt("snd_no_readings", cfg->snd_no_readings);
    prefs.putInt("snd_loop_error", cfg->snd_loop_error);
    prefs.putInt("snd_warn_start", cfg->snd_warning_at_startup);
    prefs.putInt("snd_alarm_start", cfg->snd_alarm_at_startup);
    prefs.putInt("warning_volume", cfg->warning_volume);
    prefs.putInt("alarm_volume", cfg->alarm_volume);
    prefs.putInt("brightness1", cfg->brightness1);
    prefs.putInt("brightness2", cfg->brightness2);
    prefs.putInt("brightness3", cfg->brightness3);
    prefs.putInt("sgv_only", cfg->sgv_only);
    prefs.putInt("info_line", cfg->info_line);
    prefs.putInt("date_format", cfg->date_format);
    prefs.putInt("time_format", cfg->time_format);
    prefs.putInt("display_rotate", cfg->display_rotation);
    prefs.putInt("invert_display", cfg->invert_display);
    prefs.putInt("temperat_unit", cfg->temperature_unit);
    prefs.putInt("LED_strip_mode", cfg->LED_strip_mode);
    prefs.putInt("LED_strip_pin", cfg->LED_strip_pin);
    prefs.putInt("LED_strip_count", cfg->LED_strip_count);
    prefs.putInt("LED_strip_brigh", cfg->LED_strip_brightness);
    prefs.putInt("vibration_mode", cfg->vibration_mode);
    prefs.putInt("vibration_pin", cfg->vibration_pin);
    prefs.putInt("vibration_stren", cfg->vibration_strength);
    prefs.putInt("micro_dot_pHAT", cfg->micro_dot_pHAT);
    prefs.putInt("dis_web_server", cfg->disable_web_server);
    prefs.putInt("developer_mode", cfg->dev_mode);
    char tmps[64];
    for(int i=0; i<10; i++) {
      if(cfg->wlanssid[i][0] != 0) {
        sprintf(tmps, "SSID%01d", i);
        // Serial.print("Storing key ["); Serial.print(tmps); Serial.println("]");
        prefs.putString(tmps, cfg->wlanssid[i]);
        if(cfg->wlanpass[i][0]!=0) {
          sprintf(tmps, "PASS%01d", i);
          // Serial.print("Storing key ["); Serial.print(tmps); Serial.println("]");
          prefs.putString(tmps, cfg->wlanpass[i]);
        }
      }
    }
    prefs.end();
    Serial.println("Confguration saved to flash.");
  }
}

void readConfiguration(const char *iniFilename, tConfig *cfg) {
  const size_t bufferLen = 80;
  char buffer[bufferLen];
    
  File dstFil;
  IniFile ini(iniFilename); //(uint8_t)"/M5NS.INI"
  bool iniFileRead = false;
  
  if (!ini.open()) {
    Serial.print("Ini file ");
    Serial.print(iniFilename);
    Serial.println(" does not exist");
    M5.Lcd.println("No INI file");
    // Cannot do anything else
    /*
    dstFil = SD.open("/M5NS.INI", FILE_WRITE);
    if(!dstFil) {
      Serial.println("Error opening M5NS.INI for write");
      while (1)
        ;
    } else {
      Serial.println("Writing configuration to M5NS.INI");
      dstFil.print("[config]\r\n");
      dstFil.flush();
      dstFil.close();
      if (!ini.open()) {
        Serial.println("Error retrying opening M5NS.INI for write");
        while (1)
          ;
      }
    } */
    readConfigFromFlash(cfg);
    // aaa this needs more work
    return;
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
    strlcpy(cfg->url, buffer, 64);
  }
  else {
    Serial.print("Could not read 'nightscout' from section 'config', error was ");
    printErrorMessage(ini.getError());
    M5.Lcd.println("No Nightscout URL in INI file");
    // while (1);
    // Go into bootstrapping mode.
    cfg->is_task_bootstrapping = 1;
    // cfg->url[0] = '\0';
  }

  // begin Peter Leimbach
  // Read the value of the token parameter from the config file
  if (ini.getValue("config", "token", buffer, bufferLen)) {
    Serial.print("section 'config' has an entry 'token' with value ");
    Serial.println(buffer);
    strlcpy(cfg->token, buffer, 64);
  }
  else {
    // no token parameter set in INI file - no error just set cfg->token[0] to 0
    cfg->token[0] = '\0';
  }
  // end Peter Leimbach

  if (ini.getValue("config", "bootpic", buffer, bufferLen)) {
    Serial.print("bootpic = ");
    Serial.println(buffer);
    strlcpy(cfg->bootPic, buffer, 64);
  }
  else {
    Serial.println("NO bootpic");
    cfg->bootPic[0]=0;
  }
  
  if (ini.getValue("config", "name", buffer, bufferLen)) {
    Serial.print("name = ");
    Serial.println(buffer);
    strlcpy(cfg->userName, buffer, 32);
  }
  else {
    Serial.println("NO user name");
    strcpy(cfg->userName, " ");
  }
  
  if (ini.getValue("config", "device_name", buffer, bufferLen)) {
    Serial.print("device_name = ");
    Serial.println(buffer);
    strlcpy(cfg->deviceName, buffer, 32);
  }
  else {
    Serial.println("NO device name defined, default M5NS");
    strcpy(cfg->deviceName, "M5NS");
  }
  
  if (ini.getValue("config", "time_zone", buffer, bufferLen)) {
    Serial.print("time_zone = ");
    cfg->timeZone = atoi(buffer);
    Serial.println(cfg->timeZone);
  }
  else {
    Serial.println("NO time zone defined -> Central Europe");
    cfg->timeZone = 3600;
  }

  if (ini.getValue("config", "dst", buffer, bufferLen)) {
    Serial.print("dst = ");
    cfg->dst = atoi(buffer);
    Serial.println(cfg->dst);
  }
  else {
    Serial.println("NO DST defined -> summer time");
    cfg->dst = 3600;
  }

  if (ini.getValue("config", "show_mgdl", buffer, bufferLen)) {
    Serial.print("show_mgdl = ");
    cfg->show_mgdl = atoi(buffer);
    Serial.println(cfg->show_mgdl);
  }
  else {
    Serial.println("NO show_mgdl defined -> 0 = show mmol/L");
    cfg->show_mgdl = 0;
  }

  if (ini.getValue("config", "sgv_only", buffer, bufferLen)) {
    Serial.print("sgv_only = ");
    cfg->sgv_only = atoi(buffer);
    Serial.println(cfg->sgv_only);
  }
  else {
    Serial.println("NO sgv_only defined -> 0 = try to read everything");
    cfg->sgv_only = 0;
  }

  if (ini.getValue("config", "default_page", buffer, bufferLen)) {
    Serial.print("default_page = ");
    cfg->default_page = atoi(buffer);
    Serial.println(cfg->default_page);
  }
  else {
    Serial.println("NO default page defined -> page 0 is default");
    cfg->default_page = 0;
  }

  if (ini.getValue("config", "restart_at_time", buffer, bufferLen)) {
    Serial.print("restart_at_time = ");
    strlcpy(cfg->restart_at_time, buffer, 10);
    Serial.println(cfg->restart_at_time);
  }
  else {
    Serial.println("NO restart_at_time defined -> no restarts");
    strcpy(cfg->restart_at_time, "NORES");
  }

  if (ini.getValue("config", "restart_at_logged_errors", buffer, bufferLen)) {
    Serial.print("restart_at_logged_errors = ");
    cfg->restart_at_logged_errors = atoi(buffer);
    Serial.println(cfg->restart_at_logged_errors);
  }
  else {
    Serial.println("NO restart_at_logged_errors defined -> no restarts");
    cfg->restart_at_logged_errors = 0;
  }

  if (ini.getValue("config", "show_current_time", buffer, bufferLen)) {
    Serial.print("show_current_time = ");
    cfg->show_current_time = atoi(buffer);
    Serial.println(cfg->show_current_time);
  }
  else {
    Serial.println("NO show_current_time defined");
    cfg->show_current_time = 0;
  }

  if (ini.getValue("config", "show_COB_IOB", buffer, bufferLen)) {
    Serial.print("show_COB_IOB = ");
    cfg->show_COB_IOB = atoi(buffer);
    Serial.println(cfg->show_COB_IOB);
  }
  else {
    Serial.println("NO show_COB_IOB defined");
    cfg->show_COB_IOB = 0;
  }

  if (ini.getValue("config", "snooze_timeout", buffer, bufferLen)) {
    Serial.print("snooze_timeout = ");
    cfg->snooze_timeout = atoi(buffer);
    Serial.println(cfg->snooze_timeout);
  }
  else {
    Serial.println("NO snooze_timeout defined -> set snooze time for 30 minutes");
    cfg->snooze_timeout = 30;
  }

  if (ini.getValue("config", "alarm_repeat", buffer, bufferLen)) {
    Serial.print("alarm_repeat = ");
    cfg->alarm_repeat = atoi(buffer);
    Serial.println(cfg->alarm_repeat);
  }
  else {
    Serial.println("NO alarm_repeat defined -> repeat alarm every 5 minutes");
    cfg->alarm_repeat = 5;
  }

  if (ini.getValue("config", "developer_mode", buffer, bufferLen)) {
    Serial.print("dev_mode = ");
    cfg->dev_mode = atoi(buffer);
    Serial.println(cfg->dev_mode);
  }
  else {
    Serial.println("NO developer_mode defined -> normal user mode");
    cfg->dev_mode = 0;
  }

  if (ini.getValue("config", "yellow_low", buffer, bufferLen)) {
    Serial.print("yellow_low = ");
    cfg->yellow_low = atof(buffer);
    if( cfg->show_mgdl )
      cfg->yellow_low /= 18.0;
    Serial.println(cfg->yellow_low);
  }
  else {
    Serial.println("NO yellow_low defined");
    cfg->yellow_low = 4.5;
  }

  if (ini.getValue("config", "yellow_high", buffer, bufferLen)) {
    Serial.print("yellow_high = ");
    cfg->yellow_high = atof(buffer);
    if( cfg->show_mgdl )
      cfg->yellow_high /= 18.0;
    Serial.println(cfg->yellow_high);
  }
  else {
    Serial.println("NO yellow_high defined");
    cfg->yellow_high = 9.0;
  }

  if (ini.getValue("config", "red_low", buffer, bufferLen)) {
    Serial.print("red_low = ");
    cfg->red_low = atof(buffer);
    if( cfg->show_mgdl )
      cfg->red_low /= 18.0;
    Serial.println(cfg->red_low);
  }
  else {
    Serial.println("NO red_low defined");
    cfg->red_low = 3.9;
  }

  if (ini.getValue("config", "red_high", buffer, bufferLen)) {
    Serial.print("red_high = ");
    cfg->red_high = atof(buffer);
    if( cfg->show_mgdl )
      cfg->red_high /= 18.0;
    Serial.println(cfg->red_high);
  }
  else {
    Serial.println("NO red_high defined");
    cfg->red_high = 9.0;
  }

  if (ini.getValue("config", "snd_warning", buffer, bufferLen)) {
    Serial.print("snd_warning = ");
    cfg->snd_warning = atof(buffer);
    if( cfg->show_mgdl )
      cfg->snd_warning /= 18.0;
    Serial.println(cfg->snd_warning);
  }
  else {
    Serial.println("NO snd_warning defined");
    cfg->snd_warning = 3.7;
  }

  if (ini.getValue("config", "snd_alarm", buffer, bufferLen)) {
    Serial.print("snd_alarm = ");
    cfg->snd_alarm = atof(buffer);
    if( cfg->show_mgdl )
      cfg->snd_alarm /= 18.0;
    Serial.println(cfg->snd_alarm);
  }
  else {
    Serial.println("NO snd_alarm defined");
    cfg->snd_alarm = 3.0;
  }

  if (ini.getValue("config", "snd_warning_high", buffer, bufferLen)) {
    Serial.print("snd_warning_high = ");
    cfg->snd_warning_high = atof(buffer);
    if( cfg->show_mgdl )
      cfg->snd_warning_high /= 18.0;
    Serial.println(cfg->snd_warning_high);
  }
  else {
    Serial.println("NO snd_warning_high defined");
    cfg->snd_warning_high = 14.0;
  }

  if (ini.getValue("config", "snd_alarm_high", buffer, bufferLen)) {
    Serial.print("snd_alarm_high = ");
    cfg->snd_alarm_high = atof(buffer);
    if( cfg->show_mgdl )
      cfg->snd_alarm_high /= 18.0;
    Serial.println(cfg->snd_alarm_high);
  }
  else {
    Serial.println("NO snd_alarm_high defined");
    cfg->snd_alarm_high = 20.0;
  }

  if (ini.getValue("config", "snd_no_readings", buffer, bufferLen)) {
    Serial.print("snd_no_readings = ");
    cfg->snd_no_readings = atoi(buffer);
    Serial.println(cfg->snd_no_readings);
  }
  else {
    Serial.println("NO snd_no_readings defined -> 20 minutes");
    cfg->snd_no_readings = 20;
  }

  if (ini.getValue("config", "snd_loop_error", buffer, bufferLen)) {
    Serial.print("snd_loop_error = ");
    cfg->snd_loop_error = atoi(buffer);
    Serial.println(cfg->snd_loop_error);
  }
  else {
    Serial.println("NO snd_loop_error defined -> 1 = ALARM ON");
    cfg->snd_loop_error = 1;
  }

  if (ini.getValue("config", "snd_warning_at_startup", buffer, bufferLen)) {
    Serial.print("snd_warning_at_startup = ");
    cfg->snd_warning_at_startup = atoi(buffer);
    Serial.println(cfg->snd_warning_at_startup);
  }
  else {
    Serial.println("NO snd_warning_at_startup defined -> enable warning sound at startup");
    cfg->snd_warning_at_startup = 1;
  }
  
  if (ini.getValue("config", "snd_alarm_at_startup", buffer, bufferLen)) {
    Serial.print("snd_alarm_at_startup = ");
    cfg->snd_alarm_at_startup = atoi(buffer);
    Serial.println(cfg->snd_alarm_at_startup);
  }
  else {
    Serial.println("NO snd_alarm_at_startup defined -> disable alarm sound at startup");
    cfg->snd_alarm_at_startup = 0;
  }
  
  if (ini.getValue("config", "warning_music", buffer, bufferLen)) {
    Serial.print("warning_music = ");
    Serial.println(buffer);
    strlcpy(cfg->warning_music, buffer, 64);
  }
  else {
    Serial.println("NO warning_music");
    cfg->warning_music[0]=0;
  }
  
  if (ini.getValue("config", "warning_volume", buffer, bufferLen)) {
    Serial.print("warning_volume = ");
    cfg->warning_volume = atoi(buffer);
    Serial.println(cfg->warning_volume);
  }
  else {
    Serial.println("NO warning_volume defined");
    cfg->warning_volume = 50;
  }

  if (ini.getValue("config", "alarm_music", buffer, bufferLen)) {
    Serial.print("alarm_music = ");
    Serial.println(buffer);
    strlcpy(cfg->alarm_music, buffer, 64);
  }
  else {
    Serial.println("NO alarm_music");
    cfg->alarm_music[0]=0;
  }

  if (ini.getValue("config", "alarm_volume", buffer, bufferLen)) {
    Serial.print("alarm_volume = ");
    cfg->alarm_volume = atoi(buffer);
    Serial.println(cfg->alarm_volume);
  }
  else {
    Serial.println("NO alarm_volume defined");
    cfg->alarm_volume = 100;
  }

  if (ini.getValue("config", "LED_strip_mode", buffer, bufferLen)) {
    Serial.print("LED_strip_mode = ");
    cfg->LED_strip_mode = atoi(buffer);
    Serial.println(cfg->LED_strip_mode);
  }
  else {
    Serial.println("NO LED_strip_mode defined = no LED strip used");
    cfg->LED_strip_mode = 0;
  }

  if (ini.getValue("config", "LED_strip_pin", buffer, bufferLen)) {
    Serial.print("LED_strip_pin = ");
    cfg->LED_strip_pin = atoi(buffer);
    Serial.println(cfg->LED_strip_pin);
  }
  else {
    Serial.println("NO LED_strip_pin defined = M5Stack Fire internal pin");
    cfg->LED_strip_pin = 15;
  }

  if (ini.getValue("config", "LED_strip_count", buffer, bufferLen)) {
    Serial.print("LED_strip_count = ");
    cfg->LED_strip_count = atoi(buffer);
    Serial.println(cfg->LED_strip_count);
  }
  else {
    Serial.println("NO LED_strip_count defined = 10 M5Stack Fire internal LEDs");
    cfg->LED_strip_count = 10;
  }

  if (ini.getValue("config", "LED_strip_brightness", buffer, bufferLen)) {
    Serial.print("LED_strip_brightness = ");
    cfg->LED_strip_brightness = atoi(buffer);
    Serial.println(cfg->LED_strip_brightness);
  }
  else {
    Serial.println("NO LED_strip_brightness defined -> 10");
    cfg->LED_strip_brightness = 10;
  }

  if (ini.getValue("config", "vibration_mode", buffer, bufferLen)) {
    Serial.print("vibration_mode = ");
    cfg->vibration_mode = atoi(buffer);
    Serial.println(cfg->vibration_mode);
  }
  else {
    Serial.println("NO vibration_mode defined = no vibrations");
    cfg->vibration_mode = 0;
  }

  if (ini.getValue("config", "vibration_pin", buffer, bufferLen)) {
    Serial.print("vibration_pin = ");
    cfg->vibration_pin = atoi(buffer);
    Serial.println(cfg->vibration_pin);
  }
  else {
    Serial.println("NO vibration_pin defined -> 26 = M5Stack Fire PORT B connector");
    cfg->vibration_pin = 26;
  }

  if (ini.getValue("config", "vibration_strength", buffer, bufferLen)) {
    Serial.print("vibration_strength = ");
    cfg->vibration_strength = atoi(buffer);
    Serial.println(cfg->vibration_strength);
  }
  else {
    Serial.println("NO vibration_strength defined -> 512 = 1/2 power = reasonable maximum");
    cfg->vibration_strength = 512;
  }

  if (ini.getValue("config", "micro_dot_pHAT", buffer, bufferLen)) {
    Serial.print("micro_dot_pHAT = ");
    cfg->micro_dot_pHAT = atoi(buffer);
    Serial.println(cfg->micro_dot_pHAT);
  }
  else {
    Serial.println("NO micro_dot_pHAT defined -> 0 = Micro Dot pHAT OFF");
    cfg->micro_dot_pHAT = 0;
  }

  if (ini.getValue("config", "info_line", buffer, bufferLen)) {
    Serial.print("info_line = ");
    cfg->info_line = atoi(buffer);
    Serial.println(cfg->info_line);
  }
  else {
    Serial.println("NO info_line defined = button function icons");
    cfg->info_line = 1;
  }

  if (ini.getValue("config", "brightness1", buffer, bufferLen)) {
    Serial.print("brightness1 = ");
    Serial.println(buffer);
    cfg->brightness1 = atoi(buffer);
    if(cfg->brightness1<1 || cfg->brightness1>100)
      cfg->brightness1 = 50;
  }
  else {
    Serial.println("NO brightness1");
    cfg->brightness1 = 50;
  }
  
  if (ini.getValue("config", "brightness2", buffer, bufferLen)) {
    Serial.print("brightness2 = ");
    Serial.println(buffer);
    cfg->brightness2 = atoi(buffer);
    if(cfg->brightness2<1 || cfg->brightness2>100)
      cfg->brightness2 = 100;
  }
  else {
    Serial.println("NO brightness2");
    cfg->brightness2 = 100;
  }
  if (ini.getValue("config", "brightness3", buffer, bufferLen)) {
    Serial.print("brightness3 = ");
    Serial.println(buffer);
    cfg->brightness3 = atoi(buffer);
    if(cfg->brightness3<1 || cfg->brightness3>100)
      cfg->brightness3 = 10;
  }
  else {
    Serial.println("NO brightness3");
    cfg->brightness3 = 10;
  }

  if (ini.getValue("config", "date_format", buffer, bufferLen)) {
    Serial.print("date_format = ");
    cfg->date_format = atoi(buffer);
    Serial.println(cfg->date_format);
  }
  else {
    Serial.println("NO date_format defined = 0 (dd.mm.)");
    cfg->date_format = 0;
  }

  if (ini.getValue("config", "time_format", buffer, bufferLen)) {
    Serial.print("time_format = ");
    cfg->time_format = atoi(buffer);
    Serial.println(cfg->time_format);
  }
  else {
    Serial.println("NO time_format defined = 0 (24 Hours)");
    cfg->time_format = 0;
  }

  if (ini.getValue("config", "invert_display", buffer, bufferLen)) {
    Serial.print("invert_display = ");
    cfg->invert_display = atoi(buffer);
    Serial.println(cfg->invert_display);
  }
  else {
    Serial.println("NO invert_display defined = -1");
    cfg->invert_display = -1;
  }

  if (ini.getValue("config", "display_rotation", buffer, bufferLen)) {
    Serial.print("display_rotation = ");
    cfg->display_rotation = atoi(buffer);
    Serial.println(cfg->display_rotation);
  }
  else {
    Serial.println("NO display_rotation defined = 1 = buttons down");
    cfg->display_rotation = 1;
  }

  if (ini.getValue("config", "temperature_unit", buffer, bufferLen)) {
    Serial.print("temperature_unit = ");
    cfg->temperature_unit = atoi(buffer);
    Serial.println(cfg->temperature_unit);
  }
  else {
    Serial.println("NO temperature_unit defined = CELSIUS");
    cfg->temperature_unit = 1;
  }

  if (ini.getValue("config", "disable_web_server", buffer, bufferLen)) {
    Serial.print("disable_web_server = ");
    cfg->disable_web_server = atoi(buffer);
    Serial.println(cfg->disable_web_server);
  }
  else {
    Serial.println("NO disable_web_server defined = 0");
    cfg->disable_web_server = 0;
  }

  int wlans_defined_count = 0;
  
  for(int i=0; i<=9; i++) {
    char wlansection[10];
    sprintf(wlansection, "wlan%1d", i);

    if (ini.getValue(wlansection, "ssid", buffer, bufferLen)) {
      Serial.printf("[wlan%1d] ssid = %s\r\n", i, buffer);
      strlcpy(cfg->wlanssid[i], buffer, 64);
      wlans_defined_count++;
    } else {
      Serial.printf("NO [wlan%1d] ssid\r\n", i);
      cfg->wlanssid[i][0] = 0;
    }
  
    if (ini.getValue(wlansection, "pass", buffer, bufferLen)) {
      Serial.printf("[wlan%1d] pass = %s\r\n", i, buffer);
      strlcpy(cfg->wlanpass[i], buffer, 64);
    } else {
      Serial.printf("NO [wlan%1d] pass\r\n", i);
      cfg->wlanpass[i][0] = 0;
    }
  }

  if (wlans_defined_count < 1) {
    cfg->is_task_bootstrapping = 1;
  }

}
