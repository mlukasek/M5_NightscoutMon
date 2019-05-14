#include "M5NSconfig.h"

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

void readConfiguration(char *iniFilename, tConfig *cfg) {
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
    strlcpy(cfg->url, buffer, 64);
  }
  else {
    Serial.print("Could not read 'nightscout' from section 'config', error was ");
    printErrorMessage(ini.getError());
    M5.Lcd.println("No Nightscout URL in INI file");
    while (1)
      ;
  }

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
    Serial.println("NO snd_no_readings defined");
    cfg->snd_no_readings = 20;
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
    cfg->warning_volume = 30;
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

  if (ini.getValue("wlan1", "ssid", buffer, bufferLen)) {
    Serial.print("wlan1ssid = ");
    Serial.println(buffer);
    strlcpy(cfg->wlan1ssid, buffer,32);
  }
  else {
    Serial.println("NO wlan1 ssid");
    cfg->wlan1ssid[0] = 0;
  }

  if (ini.getValue("wlan1", "pass", buffer, bufferLen)) {
    Serial.print("wlan1pass = ");
    Serial.println(buffer);
    strlcpy(cfg->wlan1pass, buffer, 32);
  }
  else {
    Serial.println("NO wlan1 pass");
    cfg->wlan1pass[0] = 0;
  }

  if (ini.getValue("wlan2", "ssid", buffer, bufferLen)) {
    Serial.print("wlan2ssid = ");
    Serial.println(buffer);
    strlcpy(cfg->wlan2ssid, buffer, 32);
  }
  else {
    Serial.println("NO wlan2 ssid");
    cfg->wlan2ssid[0] = 0;
  }

  if (ini.getValue("wlan2", "pass", buffer, bufferLen)) {
    Serial.print("wlan2pass = ");
    Serial.println(buffer);
    strlcpy(cfg->wlan2pass, buffer, 32);
  }
  else {
    Serial.println("NO wlan2 pass");
    cfg->wlan2pass[0] = 0;
  }

  if (ini.getValue("wlan3", "ssid", buffer, bufferLen)) {
    Serial.print("wlan3ssid = ");
    Serial.println(buffer);
    strlcpy(cfg->wlan3ssid, buffer, 32);
  }
  else {
    Serial.println("NO wlan3 ssid");
    cfg->wlan3ssid[0] = 0;
  }

  if (ini.getValue("wlan3", "pass", buffer, bufferLen)) {
    Serial.print("wlan3pass = ");
    Serial.println(buffer);
    strlcpy(cfg->wlan3pass, buffer, 32);
  }
  else {
    Serial.println("NO wlan3 pass");
    cfg->wlan3pass[0] = 0;
  }
}
