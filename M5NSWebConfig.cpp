#include <M5Stack.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "time.h"
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include "Free_Fonts.h"
#include "IniFile.h"
#include "M5NSconfig.h"
#include "M5NSWebConfig.h"
#include "externs.h"

void handleRoot() {
  String webVer;
  String whatsNew;
  HTTPClient http;
  IPAddress ip = WiFi.localIP();
  char tmpStr[64];

  Serial.println("Serving root web page");

  if((WiFiMultiple.run() == WL_CONNECTED)) {
    http.begin("http://m5ns.goit.cz/update/update.inf");
    http.setTimeout(5000);
    http.setConnectTimeout(5000);
    int httpCode = http.GET();
    if(httpCode > 0) {
      if(httpCode == HTTP_CODE_OK) {
        webVer = http.getString();
      } else {
        Serial.println("Error getting update.inf +");
      }
    } else {
      Serial.println("Error getting update.inf");
    }
    http.end();
    http.begin("http://m5ns.goit.cz/update/whatsnew.txt");
    http.setTimeout(5000);
    http.setConnectTimeout(5000);
    httpCode = http.GET();
    if(httpCode > 0) {
      if(httpCode == HTTP_CODE_OK) {
        whatsNew = http.getString();
      } else {
        Serial.println("Error getting whatsnew.txt +");
      }
    } else {
      Serial.println("Error getting whatsnew.txt");
    }
    http.end();
    whatsNew.replace("\r\n","<br />\r\n");
  }
  // Serial.println(webVer.length());

  char NSurl[128];
  if(strncmp(cfg.url, "http", 4))
    strcpy(NSurl,"https://");
  else
    strcpy(NSurl,"");
  strcat(NSurl,cfg.url);
  if ((cfg.token!=NULL) && (strlen(cfg.token)>0)) {
    if(strchr(NSurl,'?'))
      strcat(NSurl,"&token=");
    else
      strcat(NSurl,"?token=");
    strcat(NSurl,cfg.token);
  }

  String sgvUnits = cfg.show_mgdl?"mg/dL":"mmol/L";
  int decpl = cfg.show_mgdl?0:1;
  int mult = cfg.show_mgdl?18:1;
  
  String message = "<!DOCTYPE HTML>\r\n";
  message += "<html>\r\n";
  message += "<head>\r\n";
  message += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n";
  message += "<style>\r\n";
  message += "html { font-family: Segoe UI; font-size: 15px; font-weight: 100; display: inline-block; margin: 5px auto; text-align: left;}\r\n";
  message += ".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;\r\n";
  message += "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}\r\n";
  message += ".button2 {background-color: #555555;}\r\n";
  message += "</style>\r\n";  
  message += "<title>M5 Nightscout - "; message += cfg.userName;  message += "</title>\r\n";
  message += "</head>\r\n";
  message += "<body>\r\n";
  message += "<h1>M5Stack Nightscout monitor for "; message += cfg.userName; message += "!</h1>\r\n";

  message += "<p><b>M5Stack Nightscout monitor status</b><br />\r\n";
  if ( WiFi.status() == WL_CONNECTED) {
    message += "WiFi connected to SSID: <b>"; message += WiFi.SSID(); message += "</b><br />\r\n";
    if(mDNSactive) {
      message += "mDNS name: <b>";
      sprintf(tmpStr, "%s.local", cfg.deviceName);
      message += tmpStr; message += "</b><br />\r\n";
    }
    message += "IP address: <b>";
    sprintf(tmpStr, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
    message += tmpStr; message += "</b><br />\r\n";
    byte mac[6];
    WiFi.macAddress(mac);
    message += "MAC address: <b>";
    message += String(mac[5],HEX) + ":";
    message += String(mac[4],HEX) + ":";
    message += String(mac[3],HEX) + ":";
    message += String(mac[2],HEX) + ":";
    message += String(mac[1],HEX) + ":";
    message += String(mac[0],HEX) + "</b><br />\r\n";
  }
  message += "Battery status: <b>"; message += getBatteryLevel(); message += "%</b><br />\r\n";
  struct tm timeinfo;
  if(getLocalTime(&timeinfo)) {
    switch(cfg.date_format) {
      case 1:
        sprintf(tmpStr, "%d/%d/%d %02d:%02d:%02d", timeinfo.tm_mon+1, timeinfo.tm_mday, timeinfo.tm_year+1900, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        break;
      default:
        sprintf(tmpStr, "%d.%d.%d %02d:%02d:%02d", timeinfo.tm_mday, timeinfo.tm_mon+1, timeinfo.tm_year+1900, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }
  } else {
    strcpy(tmpStr, "??:??");
  }
  message += "Current time: <b>"; message += tmpStr; message += "</b><br />\r\n";
  switch(cfg.date_format) {
    case 1:
        sprintf(tmpStr, "%d/%d/%d %02d:%02d:%02d", ns.sensTm.tm_mon+1, ns.sensTm.tm_mday, ns.sensTm.tm_year+1900, ns.sensTm.tm_hour, ns.sensTm.tm_min, ns.sensTm.tm_sec);
        break;
      default:
        sprintf(tmpStr, "%d.%d.%d %02d:%02d:%02d", ns.sensTm.tm_mday, ns.sensTm.tm_mon+1, ns.sensTm.tm_year+1900, ns.sensTm.tm_hour, ns.sensTm.tm_min, ns.sensTm.tm_sec);
  }
  message += "Sensor time: <b>"; message += tmpStr; message += "</b><br />\r\n";
  if( cfg.show_mgdl ) {
    sprintf(tmpStr, "%3.0f mg/dL", ns.sensSgvMgDl);
  } else {
    sprintf(tmpStr, "%4.1f mmol/L", ns.sensSgv);
  }
  message += "Last sensor value: <b>"; message += tmpStr; message += "</b><br />\r\n";
  message += "Last sensor direction: <b>"; message += ns.sensDir; message += "</b><br />\r\n";
/* can possibly display all this, but why?
  struct tm sensTm;
  char sensDir[32];
  float sensSgvMgDl = 0;
  float sensSgv = 0;
  float last10sgv[10];
  bool is_xDrip = 0;  
  bool is_Sugarmate = 0;  
  char iob_displayLine[16];
  char cob_displayLine[16];
  char delta_display[16];
  char loop_display_symbol = '?';
  char loop_display_code[16];
  char loop_display_label[16];
  char basal_display[16];
  */
  message += "</p>\r\n";

  message += "<p><b>Current configuration</b><br />\r\n";
  message += "Nightscout URL: "; message += "<a href=\""; message += NSurl; message += "\"><b>"; message += NSurl; message += "</b></a> <a href=\"edititem?param=NSurl\">[edit]</a><br />\r\n";
  message += "User name: <b>"; message += cfg.userName; message += "</b> <a href=\"edititem?param=userName\">[edit]</a><br />\r\n";
  message += "Device name: <b>"; message += cfg.deviceName; message += "</b> <a href=\"edititem?param=deviceName\">[edit]</a><br />\r\n";
  message += "Time offset: <b>"; message += cfg.timeZone; message += " seconds</b> <a href=\"edititem?param=timeZoneDST\">[edit]</a><br />\r\n";
  message += "Daylight saving time offset: <b>"; message += cfg.dst; message += " seconds</b> <a href=\"edititem?param=timeZoneDST\">[edit]</a><br />\r\n";
  message += "Display units: <b>"; message += cfg.show_mgdl?"mg/dL":"mmol/L"; message += "</b> <a href=\"switch?param=show_mgdl\">[change]</a><br />\r\n";
  message += "Filter only SGV values: <b>"; message += cfg.sgv_only?"YES":"NO"; message += "</b> <a href=\"switch?param=sgv_only\">[change]</a><br />\r\n";
  message += "Default page: <b>"; message += cfg.default_page; message += "</b> <a href=\"switch?param=default_page\">[change]</a><br />\r\n";
  message += "Restart at time: <b>"; message += strcmp(cfg.restart_at_time,"NORES")==0?"do NOT restart":cfg.restart_at_time; message += "</b> <a href=\"edititem?param=restartAt\">[edit]</a><br />\r\n";
  message += "Restart at logged number of errors: <b>"; message += cfg.restart_at_logged_errors==0?"do NOT restart":String(cfg.restart_at_logged_errors); message += "</b> <a href=\"edititem?param=restartAt\">[edit]</a><br />\r\n";
  message += "Show time: <b>"; message += cfg.show_current_time?"current time":"last valid data"; message += "</b> <a href=\"switch?param=show_current_time\">[change]</a><br />\r\n";
  message += "Show COB+IOB: <b>"; message += cfg.show_COB_IOB?"YES":"NO"; message += "</b> <a href=\"switch?param=show_COB_IOB\">[change]</a><br />\r\n";
  message += "Snooze timeout: <b>"; message += cfg.snooze_timeout; message += " minutes</b> <a href=\"edititem?param=alarmTiming\">[edit]</a><br />\r\n";
  message += "Repeat alarm/warning every <b>"; message += cfg.alarm_repeat; message += " minutes</b> <a href=\"edititem?param=alarmTiming\">[edit]</a><br />\r\n";
  message += "<font color=\"#BB9900\">Display yellow bellow <b>"; message += String(cfg.yellow_low*mult, decpl); message += " "+sgvUnits; message += "</b></font> <a href=\"edititem?param=dispColors\">[edit]</a><br />\r\n";
  message += "<font color=\"#BB9900\">Display yellow above <b>"; message += String(cfg.yellow_high*mult, decpl); message += " "+sgvUnits; message += "</b></font> <a href=\"edititem?param=dispColors\">[edit]</a><br />\r\n";
  message += "<font color=\"Red\">Display red bellow <b>"; message += String(cfg.red_low*mult, decpl); message += " "+sgvUnits; message += "</b></font> <a href=\"edititem?param=dispColors\">[edit]</a><br />\r\n";
  message += "<font color=\"Red\">Display red above <b>"; message += String(cfg.red_high*mult, decpl); message += " "+sgvUnits; message += "</b></font> <a href=\"edititem?param=dispColors\">[edit]</a><br />\r\n";
  message += "<font color=\"Teal\">Warning sound bellow <b>"; message += String(cfg.snd_warning*mult, decpl); message += " "+sgvUnits; message += "</b></font> <a href=\"edititem?param=sndAlarms\">[edit]</a><br />\r\n";
  message += "<font color=\"Teal\">Warning sound above <b>"; message += String(cfg.snd_warning_high*mult, decpl); message += " "+sgvUnits; message += "</b></font> <a href=\"edititem?param=sndAlarms\">[edit]</a><br />\r\n";
  message += "<font color=\"Teal\">Alarm sound bellow <b>"; message += String(cfg.snd_alarm*mult, decpl); message += " "+sgvUnits; message += "</b></font> <a href=\"edititem?param=sndAlarms\">[edit]</a><br />\r\n";
  message += "<font color=\"Teal\">Alarm sound above <b>"; message += String(cfg.snd_alarm_high*mult, decpl); message += " "+sgvUnits; message += "</b></font> <a href=\"edititem?param=sndAlarms\">[edit]</a><br />\r\n";
  message += "<font color=\"Teal\">Warning sound when no reading for <b>"; message += cfg.snd_no_readings; message += " minutes</b></font> <a href=\"edititem?param=sndAlarms\">[edit]</a><br />\r\n";
  message += "<font color=\"Teal\">Play test warning sound during startup: <b>"; message += cfg.snd_warning_at_startup?"YES":"NO"; message += "</b></font> <a href=\"switch?param=snd_warning_at_startup\">[change]</a><br />\r\n";
  message += "<font color=\"Teal\">Play test alarm sound during startup: <b>"; message += cfg.snd_alarm_at_startup?"YES":"NO"; message += "</b></font> <a href=\"switch?param=snd_alarm_at_startup\">[change]</a><br />\r\n";
  message += "<font color=\"Teal\">Warning sound volume: <b>"; message += cfg.warning_volume; message += "%</b></font> <a href=\"edititem?param=sndAlarms\">[edit]</a><br />\r\n";
  message += "<font color=\"Teal\">Alarm sound volume: <b>"; message += cfg.alarm_volume; message += "%</b></font> <a href=\"edititem?param=sndAlarms\">[edit]</a><br />\r\n";
  message += "Status line: <b>";
  switch(cfg.info_line) {
    case 0: message += "sensor info"; break;
    case 1: message += "button function icons"; break;
    case 2: message += "loop info + basal"; break;
  }
  message += "</b> <a href=\"switch?param=info_line\">[change]</a><br />\r\n";
  message += "Brightness settings steps: <b>";  message += cfg.brightness1; message += ", "; message += cfg.brightness2; message += ", "; message += cfg.brightness3; message += "</b> <a href=\"edititem?param=brightness\">[edit]</a><br />\r\n";
  message += "Date format: <b>"; message += cfg.date_format?"MM/DD":"dd.mm."; message += "</b> <a href=\"switch?param=date_format\">[change]</a><br />\r\n";
  message += "Display rotation: <b>";
  switch(cfg.display_rotation) {
    case 1: message += "buttons down"; break;
    case 3: message += "buttons up"; break;
    case 5: message += "mirror buttons up"; break;
    case 7: message += "mirror buttons down"; break;
  }
  message += "</b> <a href=\"switch?param=display_rotation\">[change]</a><br />\r\n";
  message += "Display inversion: <b>";
  switch(cfg.invert_display) {
    case -1: message += "do not change"; break;
    case 0: message += "false"; break;
    case 1: message += "true"; break;
  }
  message += "</b> <a href=\"switch?param=invert_display\">[change]</a><br />\r\n";
  message += "Temperature units: <b>";
  switch(cfg.temperature_unit) {
    case 1: message += "Celsius"; break;
    case 2: message += "Kelvin"; break;
    case 3: message += "Fahrenheit"; break;
  }
  message += "</b> <a href=\"switch?param=temperature_unit\">[change]</a><br />\r\n";
  message += "Developer mode: <b>"; message += cfg.dev_mode?"Enabled":"Disabled"; message += "</b> <a href=\"switch?param=dev_mode\">[change]</a><br />\r\n";
  message += "Internal Web Server: <b>"; message += cfg.disable_web_server?"Disabled":"Enabled"; message += "</b><br />\r\n";
  
  for(int i=0; i<10; i++) {
    if(cfg.wlanssid[i][0] != 0) {
      message += "[wlan"; message += i; message += "] <b>";
      message += "SSID='";
      message += cfg.wlanssid[i];
      if(cfg.wlanpass[i][0]!=0) {
        message += "', PASS='";
        message += cfg.wlanpass[i];
        message += "'</b> <a href=\"edititem?param=wlans\">[edit]</a><br />\r\n";
      } else {
        message += "', no password (open)</b> <a href=\"edititem?param=wlans\">[edit]</a><br />\r\n";
      }
    }
  }
/*
  char warning_music[64];
  char alarm_music[64];

  if (output26State=="off") {
    client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
  } else {
    client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
  }
*/  
  
  message += "<a href=\"/savecfg\"><strong>Save configuration to M5NS.INI file.</strong></a></p>\r\n";

  // firmware info and update
  message += "<p><b>Application firmware</b><br />\r\n";
  message += "Current version: "; message += M5NSversion; message += "<br />\r\n";
  message += "Latest version: "; message += webVer; 
  if(webVer > M5NSversion) {
    message += " ";
    message += "<a href=\"/update\"><b>click to update</b></a>";
  }
  message += "<br />\r\n";
  if(whatsNew.length()>0) {
    message += "<b>Last update information:</b><br />\r\n";
    message += whatsNew;
    message += "\r\n";
  }
  
  message += "</body>\r\n";
  message += "</html>\r\n";
  w3srv.send(200, "text/html", message);
}

void handleUpdate() {
  Serial.print("Updating firmware, please wait ... ");
  M5.Lcd.setBrightness(100);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(0, 18);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setFreeFont(FMB9);
  M5.Lcd.setTextDatum(TL_DATUM);
  M5.Lcd.println("FIRMWARE UPDATE");
  M5.Lcd.println();
  Serial.print("Free Heap: "); Serial.println(ESP.getFreeHeap());
  M5.Lcd.print("Free Heap: "); M5.Lcd.println(ESP.getFreeHeap());
  M5.Lcd.println();

  String message = "<!DOCTYPE HTML>\r\n";
  message += "<html>\r\n";
  message += "<head>\r\n"; 
  message += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n";
  message += "<meta http-equiv=\"refresh\" content=\"30;url=/\" />\r\n";
  message += "<style>\r\n";  
  message += "html { font-family: Segoe UI; display: inline-block; margin: 5px auto; text-align: left;}\r\n";
  message += "</style>\r\n";  
  message += "<title>M5 Nightscout - "; message += cfg.userName;  message += "</title>\r\n";
  message += "</head>\r\n";
  message += "<body>\r\n";
  message += "<h1>M5Stack Nightscout monitor for "; message += cfg.userName; message += "!</h1>\r\n";

  String webVer;
  HTTPClient http;
  WiFiClient client;
  if((WiFiMultiple.run() == WL_CONNECTED)) {
    http.begin("http://m5ns.goit.cz/update/update.inf");
    int httpCode = http.GET();
    if(httpCode > 0) {
      if(httpCode == HTTP_CODE_OK) {
        webVer = http.getString();
      }
    }
  }

  M5.Lcd.print("Current firmware: "); M5.Lcd.println(M5NSversion);
  M5.Lcd.print("Found firmware:"); M5.Lcd.println(webVer);
  
  if(webVer > M5NSversion) {
    message += "<p>Updating firmware to version ";
    message += webVer;
    message += ", please wait ... </p>\r\n";
    message += "<p>Device will restart automatically.</p>\r\n";
    message += "</body>\r\n";
    message += "</html>\r\n";
    w3srv.send(200, "text/html", message);

    M5.Lcd.println();
    M5.Lcd.println("Updating the firmware... ");
    M5.Lcd.println();
    httpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return ret = httpUpdate.update(client, "http://m5ns.goit.cz/update/M5_NightscoutMon.ino.bin");
    //t_httpUpdate_return ret = httpUpdate.update(client, "server", 80, "file.bin");

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        M5.Lcd.setTextColor(RED);
        M5.Lcd.println("UPDATE FAILED");
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        M5.Lcd.setTextColor(YELLOW);
        M5.Lcd.println("NO UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK, restarting ...");
        M5.Lcd.setTextColor(GREEN);
        M5.Lcd.println("UPDATED SUCCESSFULLY");
        M5.Lcd.println("Restarting ...");

        M5.update();
        delay(1000);
        ESP.restart();
        break;
    }    
  } else {
    message += "<p>Nothing to update. Firmware version ";
    message += webVer;
    message += " is current.</p>\r\n";
    message += "</body>\r\n";
    message += "</html>\r\n";
    w3srv.send(200, "text/html", message);

    Serial.println("Nothing to update");
    M5.Lcd.println();
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.println("NOTHING TO UPDATE");
  }
  M5.update();
  delay(2000);
  M5.Lcd.fillScreen(BLACK);
  draw_page();
}

void handleSwitchConfig() {
  String message = "<!DOCTYPE HTML>\r\n";
  message += "<html>\r\n";
  message += "<head>\r\n"; 
  message += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n";
  // redirect to root after 1 second
  message += "<meta http-equiv=\"refresh\" content=\"1;url=/\" />\r\n";
  message += "<style>\r\n";  
  message += "html { font-family: Segoe UI; display: inline-block; margin: 5px auto; text-align: left;}\r\n";
  message += "</style>\r\n";  
  message += "<title>M5 Nightscout - "; message += cfg.userName;  message += "</title>\r\n";
  message += "</head>\r\n";
  message += "<body>\r\n";
  message += "<h1>M5Stack Nightscout monitor for "; message += cfg.userName; message += "!</h1>\r\n";
  message += "<p>Switching configuration value.</p>\r\n";
  /*
  message += "\nArguments: ";
  message += w3srv.args();
  message += "\n";
  for (uint8_t i = 0; i < w3srv.args(); i++) {
    message += " " + w3srv.argName(i) + ": " + w3srv.arg(i) + "\n";
  }
  */
  for (uint8_t i = 0; i < w3srv.args(); i++) {
    if(String(w3srv.argName(i)).equals("param")) {
      if(String(w3srv.arg(i)).equals("show_mgdl")) {
        cfg.show_mgdl = !cfg.show_mgdl;
        // Serial.print("show_mgdl = "); Serial.println(cfg.show_mgdl);
      }
      if(String(w3srv.arg(i)).equals("sgv_only")) {
        cfg.sgv_only = !cfg.sgv_only;
      }
      if(String(w3srv.arg(i)).equals("default_page")) {
        cfg.default_page++;
        if(cfg.default_page>maxPage)        
          cfg.default_page = 0;
        dispPage = cfg.default_page;
        setPageIconPos(dispPage);
      }
      if(String(w3srv.arg(i)).equals("show_current_time")) {
        cfg.show_current_time = !cfg.show_current_time;
      }
      if(String(w3srv.arg(i)).equals("show_COB_IOB")) {
        cfg.show_COB_IOB = !cfg.show_COB_IOB;
      }
      if(String(w3srv.arg(i)).equals("snd_warning_at_startup")) {
        cfg.snd_warning_at_startup = !cfg.snd_warning_at_startup;
      }
      if(String(w3srv.arg(i)).equals("snd_alarm_at_startup")) {
        cfg.snd_alarm_at_startup = !cfg.snd_alarm_at_startup;
      }
      if(String(w3srv.arg(i)).equals("info_line")) {
        cfg.info_line++;
        if(cfg.info_line>2)
          cfg.info_line = 0;
      }
      if(String(w3srv.arg(i)).equals("date_format")) {
        cfg.date_format++;
        if(cfg.date_format>1)
          cfg.date_format = 0;
      }
      if(String(w3srv.arg(i)).equals("display_rotation")) {
        if(cfg.display_rotation == 1) {
          cfg.display_rotation = 3;
        } else {
          if(cfg.display_rotation == 3) {
            cfg.display_rotation = 5;
          } else {
            if(cfg.display_rotation == 5) {
              cfg.display_rotation = 7;
            } else {
              cfg.display_rotation = 1;
            }
          }
        }
        M5.Lcd.setRotation(cfg.display_rotation);
      }
      if(String(w3srv.arg(i)).equals("invert_display")) {
        cfg.invert_display++;
        if(cfg.invert_display>1)
          cfg.invert_display = -1;
        if(cfg.invert_display != -1)
          M5.Lcd.invertDisplay(cfg.invert_display);
      }
      if(String(w3srv.arg(i)).equals("temperature_unit")) {
        cfg.temperature_unit++;
        if(cfg.temperature_unit>3)
          cfg.temperature_unit = 1;
      }
      if(String(w3srv.arg(i)).equals("dev_mode")) {
        cfg.dev_mode = !cfg.dev_mode;
      }
    }
  }
  message += "</body>\r\n";
  message += "</html>\r\n";
  w3srv.send(200, "text/html", message);

  M5.Lcd.fillScreen(BLACK);
  draw_page();
}

void handleEditConfigItem() {
  String sgvUnits = cfg.show_mgdl?"mg/dL":"mmol/L";
  int decpl = cfg.show_mgdl?0:1;
  int mult = cfg.show_mgdl?18:1;

  String message = "<!DOCTYPE HTML>\r\n";
  message += "<html>\r\n";
  message += "<head>\r\n"; 
  message += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n";
  // message += "<meta http-equiv=\"refresh\" content=\"30;url=/\" />\r\n";
  message += "<style>\r\n";  
  message += "html { font-family: Segoe UI; display: inline-block; margin: 5px auto; text-align: left;}\r\n";
  message += "</style>\r\n";  
  message += "<title>M5 Nightscout - "; message += cfg.userName;  message += "</title>\r\n";
  message += "</head>\r\n";
  message += "<body>\r\n";
  message += "<h1>M5Stack Nightscout monitor for "; message += cfg.userName; message += "!</h1>\r\n";
  message += "<p>Edit configuration item.</p>\r\n";
  message += "<form action=\"/getedititem\" method=\"post\">\r\n";
  // char itemName[60];
  // char itemValue[250];
  if(String(w3srv.argName(0)).equals("param")) {
    if(String(w3srv.arg(0)).equals("userName")) {
      message += "M5Stack user name: \r\n";
      message += "<input type=\"text\" name=\"" + String(w3srv.arg(0)) + "\" value=\"" + String(cfg.userName) + "\" size=\"20\" maxlength=\"32\"><br>\r\n";
    }
    if(String(w3srv.arg(0)).equals("NSurl")) {
      message += "Nightscout site URL: \r\n";
      message += "<input type=\"text\" name=\"url\" value=\"" + String(cfg.url) + "\" size=\"32\" maxlength=\"128\"> \r\n";
      message += " (for example <i>sitename.herokuapp.com</i>)<br>\r\n";
      message += "Security token: \r\n";
      message += "<input type=\"text\" name=\"token\" value=\"" + String(cfg.token) + "\" size=\"32\" maxlength=\"32\"> \r\n";
      message += " (empty, if not used)<br>\r\n";
    }
    if(String(w3srv.arg(0)).equals("deviceName")) {
      message += "M5Stack device name: \r\n";
      message += "<input type=\"text\" name=\"deviceName\" value=\"" + String(cfg.deviceName) + "\" size=\"12\" maxlength=\"32\">.local \r\n";
      message += "(for internet browser access)<br>\r\n";
      message += "<font color=red>Warning: changes work only after save to M5NS.INI and reboot of the M5Stack device.</font><br>\r\n";
    }
    if(String(w3srv.arg(0)).equals("timeZoneDST")) {
      message += "Time zone offset in seconds: \r\n";
      message += "<input type=\"text\" name=\"timeZone\" value=\"" + String(cfg.timeZone) + "\" size=\"6\" maxlength=\"6\"> \r\n";
      message += "(for example 3600 for most of Europe)<br>\r\n";
      message += "(see <a href=\"https://www.epochconverter.com/timezones\">https://www.epochconverter.com/timezones</a>)<br>\r\n";
      message += "Daylight saving time offset: \r\n";
      message += "<input type=\"text\" name=\"dst\" value=\"" + String(cfg.dst) + "\" size=\"6\" maxlength=\"6\"> \r\n";
      message += "(usually 0 in winter and 3600 in summer)<br>\r\n";
      message += "<font color=red>Warning: changes work only after save to M5NS.INI and reboot of the M5Stack device.</font><br>\r\n";
    }
    if(String(w3srv.arg(0)).equals("restartAt")) {
      message += "Restart at time: \r\n";
      message += "<input type=\"text\" name=\"restart_at_time\" value=\"" + String(cfg.restart_at_time) + "\" size=\"5\" maxlength=\"5\"> \r\n";
      message += "(device restart time in 24h HH:MM format or NORES for no restart at any time)<br>\r\n";
      message += "Restart at logged number of errors: \r\n";
      message += "<input type=\"text\" name=\"restart_at_logged_errors\" value=\"" + String(cfg.restart_at_logged_errors) + "\" size=\"5\" maxlength=\"5\"> \r\n";
      message += "(0 = do not restart)<br>\r\n";
    }
    if(String(w3srv.arg(0)).equals("alarmTiming")) {
      message += "Snooze timeout: \r\n";
      message += "<input type=\"text\" name=\"snooze_timeout\" value=\"" + String(cfg.snooze_timeout) + "\" size=\"5\" maxlength=\"5\"> \r\n";
      message += "minutes<br>\r\n";
      message += "Repeat alarm/warning every \r\n";
      message += "<input type=\"text\" name=\"alarm_repeat\" value=\"" + String(cfg.alarm_repeat) + "\" size=\"5\" maxlength=\"5\"> \r\n";
      message += "minutes (0 = as fast as it goes)<br>\r\n";
    }
    if(String(w3srv.arg(0)).equals("dispColors")) {
      message += "<font color=\"#BB9900\">Display yellow bellow \r\n";
      message += "<input type=\"text\" name=\"yellow_low\" value=\"" + String(cfg.yellow_low*mult, decpl) + "\" size=\"5\" maxlength=\"5\"> \r\n";
      message += " " + sgvUnits + "</font><br>\r\n";
      message += "<font color=\"#BB9900\">Display yellow above \r\n";
      message += "<input type=\"text\" name=\"yellow_high\" value=\"" + String(cfg.yellow_high*mult, decpl) + "\" size=\"5\" maxlength=\"5\"> \r\n";
      message += " " + sgvUnits + "</font><br><br>\r\n";
      message += "<font color=\"Red\">Display red bellow \r\n";
      message += "<input type=\"text\" name=\"red_low\" value=\"" + String(cfg.red_low*mult, decpl) + "\" size=\"5\" maxlength=\"5\"> \r\n";
      message += " " + sgvUnits + "</font><br>\r\n";
      message += "<font color=\"Red\">Display red above \r\n";
      message += "<input type=\"text\" name=\"red_high\" value=\"" + String(cfg.red_high*mult, decpl) + "\" size=\"5\" maxlength=\"5\"> \r\n";
      message += " " + sgvUnits + "</font><br>\r\n";
    }
    if(String(w3srv.arg(0)).equals("sndAlarms")) {
      message += "Warning sound bellow \r\n";
      message += "<input type=\"text\" name=\"snd_warning\" value=\"" + String(cfg.snd_warning*mult, decpl) + "\" size=\"5\" maxlength=\"5\"> \r\n";
      message += " " + sgvUnits + "<br>\r\n";
      message += "Warning sound above \r\n";
      message += "<input type=\"text\" name=\"snd_warning_high\" value=\"" + String(cfg.snd_warning_high*mult, decpl) + "\" size=\"5\" maxlength=\"5\"> \r\n";
      message += " " + sgvUnits + "<br><br>\r\n";
      message += "Alarm sound bellow \r\n";
      message += "<input type=\"text\" name=\"snd_alarm\" value=\"" + String(cfg.snd_alarm*mult, decpl) + "\" size=\"5\" maxlength=\"5\"> \r\n";
      message += " " + sgvUnits + "<br>\r\n";
      message += "Alarm sound above \r\n";
      message += "<input type=\"text\" name=\"snd_alarm_high\" value=\"" + String(cfg.snd_alarm_high*mult, decpl) + "\" size=\"5\" maxlength=\"5\"> \r\n";
      message += " " + sgvUnits + "<br><br>\r\n";
      message += "Warning sound when no reading for \r\n";
      message += "<input type=\"text\" name=\"snd_no_readings\" value=\"" + String(cfg.snd_no_readings) + "\" size=\"5\" maxlength=\"5\"> \r\n";
      message += " minutes<br><br>\r\n";
      message += "Warning sound volume: \r\n";
      message += "<input type=\"text\" name=\"warning_volume\" value=\"" + String(cfg.warning_volume) + "\" size=\"5\" maxlength=\"5\"> %<br>\r\n";
      message += "Alarm sound volume: \r\n";
      message += "<input type=\"text\" name=\"alarm_volume\" value=\"" + String(cfg.alarm_volume) + "\" size=\"3\" maxlength=\"3\"> %<br>\r\n";
    }
    if(String(w3srv.arg(0)).equals("brightness")) {
      message += "Brightness settings steps:<br />\r\n";
      message += "Step 1: <input type=\"text\" name=\"brightness1\" value=\"" + String(cfg.brightness1) + "\" size=\"3\" maxlength=\"3\"> % (default)<br />\r\n";
      message += "Step 2: <input type=\"text\" name=\"brightness2\" value=\"" + String(cfg.brightness2) + "\" size=\"3\" maxlength=\"3\"> %<br />\r\n";
      message += "Step 3: <input type=\"text\" name=\"brightness3\" value=\"" + String(cfg.brightness3) + "\" size=\"3\" maxlength=\"3\"> %<br />\r\n";
    }
    if(String(w3srv.arg(0)).equals("wlans")) {
      message += "Wifi LAN SSIDs + passwords:<br />\r\n";
      for(int i=0; i<10; i++) {
        message += "[wlan"; message += i; message += "] ";
        message += "SSID: <input type=\"text\" name=\"wlanssid" + String(i) + "\" value=\"" + String(cfg.wlanssid[i]) + "\" size=\"12\" maxlength=\"32\"> , \r\n";
        message += "PASS: <input type=\"text\" name=\"wlanpass" + String(i) + "\" value=\"" + String(cfg.wlanpass[i]) + "\" size=\"12\" maxlength=\"64\">\r\n";
        if(i==0)
          message += " Do not use this [wlan0] row unless necessary, reserved for autoconfig.\r\n";
        message += "<br />\r\n";
      }
      message += "<font color=red>Warning: changes work only after save to M5NS.INI and reboot of the M5Stack device.</font><br>\r\n";
    }
  }
  message += "<br />\r\n";
  message += "<input type=\"submit\" name=\"Submit\" value=\"&nbsp;OK&nbsp;\">\r\n";
  message += "<input type=\"button\" name=\"Cancel\" value=\"Cancel\" onClick=\"window.location.href='/';\" />\r\n";
  message += "</form>\r\n";
  message += "</body>\r\n";
  message += "</html>\r\n";
  w3srv.send(200, "text/html", message);
}

void handleGetEditConfigItem() {
  // String sgvUnits = cfg.show_mgdl?"mg/dL":"mmol/L";
  // int decpl = cfg.show_mgdl?0:1;
  float mult = cfg.show_mgdl?18.0:1.0;
  String tmpStr;

  String message = "<!DOCTYPE HTML>\r\n";
  message += "<html>\r\n";
  message += "<head>\r\n"; 
  message += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n";
  message += "<meta http-equiv=\"refresh\" content=\"1;url=/\" />\r\n";
  message += "<style>\r\n";  
  message += "html { font-family: Segoe UI; display: inline-block; margin: 5px auto; text-align: left;}\r\n";
  message += "</style>\r\n";  
  message += "<title>M5 Nightscout - "; message += cfg.userName;  message += "</title>\r\n";
  message += "</head>\r\n";
  message += "<body>\r\n";
  message += "<h1>M5Stack Nightscout monitor for "; message += cfg.userName; message += "!</h1>\r\n";
  message += "<p>Updating edited items in M5Stack Nightscout monitor configuration (NOT saving to M5NS.INI file).</p>\r\n";
  for (uint8_t i = 0; i < w3srv.args(); i++) {
    if(String(w3srv.argName(i)).equals("userName")) {
      strncpy(cfg.userName, String(w3srv.arg(i)).c_str(), 32);
    }
    if(String(w3srv.argName(i)).equals("url")) {
      strncpy(cfg.url, String(w3srv.arg(i)).c_str(), 128);
    }
    if(String(w3srv.argName(i)).equals("token")) {
      strncpy(cfg.token, String(w3srv.arg(i)).c_str(), 32);
    }
    if(String(w3srv.argName(i)).equals("deviceName")) {
      strncpy(cfg.deviceName, String(w3srv.arg(i)).c_str(), 32);
    }
    if(String(w3srv.argName(i)).equals("timeZone")) {
      cfg.timeZone = String(w3srv.arg(i)).toInt();
    }
    if(String(w3srv.argName(i)).equals("dst")) {
      cfg.dst = String(w3srv.arg(i)).toInt();
    }
    if(String(w3srv.argName(i)).equals("restart_at_time")) {
      strncpy(cfg.restart_at_time, String(w3srv.arg(i)).c_str(), 10);
    }
    if(String(w3srv.argName(i)).equals("restart_at_logged_errors")) {
      cfg.restart_at_logged_errors = String(w3srv.arg(i)).toInt();
    }
    if(String(w3srv.argName(i)).equals("snooze_timeout")) {
      cfg.snooze_timeout = String(w3srv.arg(i)).toInt();
    }
    if(String(w3srv.argName(i)).equals("alarm_repeat")) {
      cfg.alarm_repeat = String(w3srv.arg(i)).toInt();
    }
    if(String(w3srv.argName(i)).equals("yellow_low")) {
      cfg.yellow_low = String(w3srv.arg(i)).toFloat()/mult;
    }
    if(String(w3srv.argName(i)).equals("yellow_high")) {
      cfg.yellow_high = String(w3srv.arg(i)).toFloat()/mult;
    }
    if(String(w3srv.argName(i)).equals("red_low")) {
      cfg.red_low = String(w3srv.arg(i)).toFloat()/mult;
    }
    if(String(w3srv.argName(i)).equals("red_high")) {
      cfg.red_high = String(w3srv.arg(i)).toFloat()/mult;
    }
    if(String(w3srv.argName(i)).equals("snd_warning")) {
      cfg.snd_warning = String(w3srv.arg(i)).toFloat()/mult;
    }
    if(String(w3srv.argName(i)).equals("snd_warning_high")) {
      cfg.snd_warning_high = String(w3srv.arg(i)).toFloat()/mult;
    }
    if(String(w3srv.argName(i)).equals("snd_alarm")) {
      cfg.snd_alarm = String(w3srv.arg(i)).toFloat()/mult;
    }
    if(String(w3srv.argName(i)).equals("snd_alarm_high")) {
      cfg.snd_alarm_high = String(w3srv.arg(i)).toFloat()/mult;
    }
    if(String(w3srv.argName(i)).equals("snd_no_readings")) {
      cfg.snd_no_readings = String(w3srv.arg(i)).toInt();
    }
    if(String(w3srv.argName(i)).equals("warning_volume")) {
      cfg.warning_volume = String(w3srv.arg(i)).toInt();
    }
    if(String(w3srv.argName(i)).equals("alarm_volume")) {
      cfg.alarm_volume = String(w3srv.arg(i)).toInt();
    }
    if(String(w3srv.argName(i)).equals("brightness1")) {
      if(lcdBrightness==cfg.brightness1) {
        // changing selected brightness, so update it
        lcdBrightness = String(w3srv.arg(i)).toInt();
        M5.Lcd.setBrightness(lcdBrightness);
      }
      cfg.brightness1 = String(w3srv.arg(i)).toInt();
    }
    if(String(w3srv.argName(i)).equals("brightness2")) {
      if(lcdBrightness==cfg.brightness2) {
        // changing selected brightness, so update it
        lcdBrightness = String(w3srv.arg(i)).toInt();
        M5.Lcd.setBrightness(lcdBrightness);
      }
      cfg.brightness2 = String(w3srv.arg(i)).toInt();
    }
    if(String(w3srv.argName(i)).equals("brightness3")) {
      if(lcdBrightness==cfg.brightness3) {
        // changing selected brightness, so update it
        lcdBrightness = String(w3srv.arg(i)).toInt();
        M5.Lcd.setBrightness(lcdBrightness);
      }
      cfg.brightness3 = String(w3srv.arg(i)).toInt();
    }
    if(String(w3srv.argName(i)).startsWith("wlanssid")) {
      tmpStr = String(w3srv.argName(i));
      tmpStr.remove(0, 8);
      int nr = tmpStr.toInt();
      message += "WLAN SSID [" + tmpStr + "] = " + String(w3srv.arg(i)) + " (" + String(w3srv.arg(i)).length() + ")<br />\r\n";
      strncpy(cfg.wlanssid[nr], String(w3srv.arg(i)).c_str(), 32);
    }
    if(String(w3srv.argName(i)).startsWith("wlanpass")) {
      tmpStr = String(w3srv.argName(i));
      tmpStr.remove(0, 8);
      int nr = tmpStr.toInt();
      message += "WLAN PASS [" + tmpStr + "] = " + String(w3srv.arg(i)) + " (" + String(w3srv.arg(i)).length() + ")<br />\r\n";
      strncpy(cfg.wlanpass[nr], String(w3srv.arg(i)).c_str(), 64);
    }
  }
  message += "</body>\r\n";
  message += "</html>\r\n";
  w3srv.send(200, "text/html", message);

  M5.Lcd.fillScreen(BLACK);
  draw_page();
}

void handleSaveConfig() {
  String sgvUnits = cfg.show_mgdl?"mg/dL":"mmol/L";
  int decpl = cfg.show_mgdl?0:1;
  int mult = cfg.show_mgdl?18:1;
  String tmpStr;
    
  String message = "<!DOCTYPE HTML>\r\n";
  message += "<html>\r\n";
  message += "<head>\r\n"; 
  message += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\r\n";
  message += "<meta http-equiv=\"refresh\" content=\"2;url=/\" />\r\n";
  message += "<style>\r\n";  
  message += "html { font-family: Segoe UI; display: inline-block; margin: 5px auto; text-align: left;}\r\n";
  message += "</style>\r\n";  
  message += "<title>M5 Nightscout - "; message += cfg.userName;  message += "</title>\r\n";
  message += "</head>\r\n";
  message += "<body>\r\n";
  message += "<h1>M5Stack Nightscout monitor for "; message += cfg.userName; message += "!</h1>\r\n";
  message += "<p>Saving configuration to M5NS.INI file.</p>\r\n";
  message += "<p>Backup copy of current M5NS.INI should be blaced in M5NS.BAK file.</p>\r\n";
  message += "</body>\r\n";
  message += "</html>\r\n";
  
  M5.Lcd.drawJpgFile(SD, cfg.bootPic);

  File srcFil, dstFil;

  Serial.println("Creating backup copy of M5NS.INI to M5NS.BAK");
  
  if(!SD.remove("/M5NS.BAK")) {
    Serial.println("Error removing M5NS.BAK");
  }
  srcFil = SD.open("/M5NS.INI", FILE_READ);
  if(!srcFil) {
    Serial.println("Error opening M5NS.INI for read");
  } else {
    dstFil = SD.open("/M5NS.BAK", FILE_WRITE);
    if(!dstFil) {
      Serial.println("Error opening M5NS.BAK for write");
    } else {
      if(srcFil.size()>0) {
        size_t n; 
        uint8_t buf[256];
        while ((n = srcFil.read(buf, sizeof(buf))) > 0) {
          dstFil.write(buf, n);
        }
        srcFil.close();
        dstFil.close();
        Serial.println("INI to BAK should be copied");
      } else {
        Serial.println("Source file M5NS.INI is empty.");
      }
    }
  }
  
  if(!SD.remove("/M5NS.INI")) {
    Serial.println("Error removing M5NS.BAK");
  }
  dstFil = SD.open("/M5NS.INI", FILE_WRITE);
  if(!dstFil) {
    Serial.println("Error opening M5NS.INI for write");
  } else {
    Serial.println("Writing configuration to M5NS.INI");
    dstFil.print("[config]\r\n");
    dstFil.print("nightscout = "); dstFil.print(cfg.url); dstFil.print("\r\n");
    dstFil.print("token = "); dstFil.print(cfg.token); dstFil.print("\r\n");
    dstFil.print("bootpic = "); dstFil.print(cfg.bootPic); dstFil.print("\r\n");
    dstFil.print("name = "); dstFil.print(cfg.userName); dstFil.print("\r\n");
    dstFil.print("device_name = "); dstFil.print(cfg.deviceName); dstFil.print("\r\n");
    dstFil.print("time_zone = "); dstFil.print(cfg.timeZone); dstFil.print("\r\n");
    dstFil.print("dst = "); dstFil.print(cfg.dst); dstFil.print("\r\n");
    dstFil.print("show_mgdl = "); dstFil.print(cfg.show_mgdl); dstFil.print("\r\n");
    dstFil.print("show_current_time = "); dstFil.print(cfg.show_current_time); dstFil.print("\r\n");
    dstFil.print("show_COB_IOB = "); dstFil.print(cfg.show_COB_IOB); dstFil.print("\r\n");
    dstFil.print("default_page = "); dstFil.print(cfg.default_page); dstFil.print("\r\n");
    dstFil.print("restart_at_time = "); dstFil.print(cfg.restart_at_time); dstFil.print("\r\n");
    dstFil.print("restart_at_logged_errors = "); dstFil.print(cfg.restart_at_logged_errors); dstFil.print("\r\n");
    dstFil.print("\r\n");
    dstFil.print("snooze_timeout = "); dstFil.print(cfg.snooze_timeout); dstFil.print("\r\n");
    dstFil.print("alarm_repeat = "); dstFil.print(cfg.alarm_repeat); dstFil.print("\r\n");
    dstFil.print("\r\n");
    dstFil.print("yellow_low = "); dstFil.print(String(cfg.yellow_low*mult, decpl)); dstFil.print("\r\n");
    dstFil.print("yellow_high = "); dstFil.print(String(cfg.yellow_high*mult, decpl)); dstFil.print("\r\n");
    dstFil.print("red_low = "); dstFil.print(String(cfg.red_low*mult, decpl)); dstFil.print("\r\n");
    dstFil.print("red_high = "); dstFil.print(String(cfg.red_high*mult, decpl)); dstFil.print("\r\n");
    dstFil.print("\r\n");
    dstFil.print("snd_warning = "); dstFil.print(String(cfg.snd_warning*mult, decpl)); dstFil.print("\r\n");
    dstFil.print("snd_alarm = "); dstFil.print(String(cfg.snd_alarm*mult, decpl)); dstFil.print("\r\n");
    dstFil.print("snd_warning_high = "); dstFil.print(String(cfg.snd_warning_high*mult, decpl)); dstFil.print("\r\n");
    dstFil.print("snd_alarm_high = "); dstFil.print(String(cfg.snd_alarm_high*mult, decpl)); dstFil.print("\r\n");
    dstFil.print("snd_no_readings = "); dstFil.print(cfg.snd_no_readings); dstFil.print("\r\n");
    dstFil.print("snd_warning_at_startup = "); dstFil.print(cfg.snd_warning_at_startup); dstFil.print("\r\n");
    dstFil.print("snd_alarm_at_startup = "); dstFil.print(cfg.snd_alarm_at_startup); dstFil.print("\r\n");
    dstFil.print("\r\n");
    dstFil.print("warning_volume = "); dstFil.print(cfg.warning_volume); dstFil.print("\r\n");
    dstFil.print("alarm_volume = "); dstFil.print(cfg.alarm_volume); dstFil.print("\r\n");
    dstFil.print("\r\n");
    dstFil.print("brightness1 = "); dstFil.print(cfg.brightness1); dstFil.print("\r\n");
    dstFil.print("brightness2 = "); dstFil.print(cfg.brightness2); dstFil.print("\r\n");
    dstFil.print("brightness3 = "); dstFil.print(cfg.brightness3); dstFil.print("\r\n");
    dstFil.print("\r\n");
    dstFil.print("sgv_only = "); dstFil.print(cfg.sgv_only); dstFil.print("\r\n");
    dstFil.print("info_line = "); dstFil.print(cfg.info_line); dstFil.print("\r\n");
    dstFil.print("date_format = "); dstFil.print(cfg.date_format); dstFil.print("\r\n");
    dstFil.print("display_rotation = "); dstFil.print(cfg.display_rotation); dstFil.print("\r\n");
    if(cfg.invert_display!=-1) {
      dstFil.print("invert_display = "); dstFil.print(cfg.invert_display); dstFil.print("\r\n");
    }
    dstFil.print("temperature_unit = "); dstFil.print(cfg.temperature_unit); dstFil.print("\r\n");
    dstFil.print("disable_web_server = "); dstFil.print(cfg.disable_web_server); dstFil.print("\r\n");
    dstFil.print("\r\n");
    dstFil.print("developer_mode = "); dstFil.print(cfg.dev_mode); dstFil.print("\r\n");
    dstFil.print("\r\n");
    for(int i=0; i<10; i++) {
      if(cfg.wlanssid[i][0] != 0) {
        dstFil.print("[wlan"); dstFil.print(i); dstFil.print("]\r\n");
        dstFil.print("SSID = "); dstFil.print(cfg.wlanssid[i]); dstFil.print("\r\n");
        if(cfg.wlanpass[i][0]!=0) {
          dstFil.print("PASS = "); dstFil.print(cfg.wlanpass[i]); dstFil.print("\r\n");
        }
      }
    }
    dstFil.flush();
    dstFil.close();
  }
  Serial.println("New M5NS.INI should be saved");

  w3srv.send(200, "text/html", message);
  
  delay(100);
  M5.Lcd.fillScreen(BLACK);
  draw_page();
}

void handleNotFound() {
  String message = "M5Stack Nighscout monitor ERROR 404 File Not Found\n\n";
  message += "URI: ";
  message += w3srv.uri();
  message += "\nMethod: ";
  message += (w3srv.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += w3srv.args();
  message += "\n";
  for (uint8_t i = 0; i < w3srv.args(); i++) {
    message += " " + w3srv.argName(i) + ": " + w3srv.arg(i) + "\n";
  }
  w3srv.send(404, "text/plain", message);
}
