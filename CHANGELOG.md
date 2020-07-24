# M5Stack Nightscout monitor project Changelog

## Revisions


### 24 July 2020

* Added SHT30 compatibility, so now temperature and humidity works with newer version of BTC TICKER stand and ENV.II Unit.  

### 12 May 2020

* Delta displayed in correct M5NS.INI units regardles from Nightscout settings. (romkuru)  
* Corrected "bellow->below" typo in web interface. (guydavies)  

### 12 April 2020

* WiFi passwords are now hidden on all web pages and forms.  
* "snooze" part completely reworked.  
* Possibility to multiply alarm/warning sound snooze time by multiple presses of middle button. The button has to be pressed in less than 2s after the last press. The steps are 1x-2x-3x-4x-OFF and again.  
* Snooze (middle button) from one device works on all devices on the same network subnet with the same user (Nightscout URL). UDP broadcast is distrubuted in local network subnet.  

### 13 February 2020

* Improved time handling when Epoch in milliseconds has decimal places (CGMBLEKit).
* Added version number to the latest (log) page.  
* Some minor diagnostics monitor/log updates.  

### 09 February 2020

* Support for OpenAPS loop (set info_line = 3 in M5NS.INI).  
* Corrected error in token implementation for IOB, COB, Loop, OpenAPS, basal.  
* Corrected an error in shifting SGV history graph buffer.  
* Commented out some unused variables  
* Renamed WiFiMulti to WiFiMultiple. (Dominik Dzienia)  
* Some externals moved to extern.h  (Dominik Dzienia)  
* Sources are now possible to compile in VSCode & Platform.IO, which is about 4 times faster, has better syntax highlighting, ... Just rename .ino to .cpp (Dominik Dzienia)  
* Cleanup of README.md, changelog refactored to CHANGELOG.md (Dominik Dzienia)  

### 26 December 2019

* HTTP Server now supports full configuration edit and save.  
* WebConfig sources moved to separate files.  
* Backup copy of `M5NS.INI` is copied to M5NS.BAK during save.  

### 20 October 2019

This is mostly test release with new "geek" features. Should be mostly for people with specific troubles and early adopters.

* Maximum password length extended to 63 characters.
* Possibility to connect to open WiFi network (without password).  
device_name key added to `M5NS.INI` (default is M5NS).  
* mDNS added, you can connect to M5 Nightscout monitor by device_name.local (default M5NS.local).  
* mDNS name and IP address is displayed on Error log page.  
* Experimental web server added. Display configuration and check the current and the latest firmware version.  
* Online update option from internal web page.  

> Please note, there is no security implemented yet. Internal web page is on unsecured port 80. Updates are going from server port 80, no SLL used currently. Use on your own risk.  

### 10 October 2019

* Key invert_display (default = -1 when key is not present) added to `M5NS.INI`. You should not use it in `M5NS.INI` unless you have a problem with display inversion. Set invert_display = 0 or invert_display = 1 if you have troubles to call M5.Lcd.invertDisplay(0) or M5.Lcd.invertDisplay(1). Under normal circumstances 0 should be normal display and 1 should be inverted display.  

### 23 September 2019

* Key sgv_only (default 0) added to `M5NS.INI`. You should set it to 1 if you use xDrip, Spike or similar to filter out calibrations etc.  
* Explicit M5.Lcd.invertDisplay(0) added to try to prevent inverted display.  

### 21 September 2019

* Added support for Dexcom by using Sugarmate connection workaround. Thanks to Patrick Sonnerat.  
* Only SGV entries are now queried, so no more troubles with calibration. Thanks to Sulka Haro.  
* Fast page switching. No need to wait for NS data (does not work while accessing Nightscout = while blue WiFi icon displayed).  
* New page with analog clock and Temperature/Humidity. Display environment values requires DHT12 - ENV Unit or BTC Standing Base.  
* New key in `M5NS.INI` temperature_unit = 1 for CELSIUS, 2 for KELVIN, 3 for FAHRENHEIT. Can be omitted (default is Celsius).  
* Display rotation possibility added. New key in `M5NS.INI` display_rotation = 1 (buttons down, default, can be omitted), 3 = buttons up, 5 = mirror buttons up, 7 = mirror buttons down.  
* US date format added. New key in `M5NS.INI` date_format = 0 (dd.mm., default, can be omitted), 1 = MM/DD.  
* JSON query update for some Bluetooth Glucose Meters.  

### 20 June 2019

* Split of Nightscout read and display code (this should allow simpler user display code update and more different "faces" from users).  
* New concept of display pages (different display designs, information, faces).  
* Switch page by short press of the right button.  
* Power OFF by the right button long press (4 seconds).  
* Right button works also as power ON after power off by this button.  
* New page added with large simple info (large BG, clock, delta + arrow and  few icons only).  
* New `M5NS.INI` key "default_page" added (default 0).  
* Smaller WiFi symbol (now as blue WiFi icon in 2 sizes for the 2 different Nightscout queries).  
* Buttons do not work during Nightscout communication (blue WiFi symbol displayed).  
* Bigger delta value even with COB+IOB values displayed (COB: and IOB: shortened to C: and I: ).  
* Errors now logged silently (log can be displayed as the last page).  
* Warning triangle icon added to show that errors are in the log (up to 5 errors - grey, more - yellow). Only last 10 errors can be displayed.  
* New `M5NS.INI` key "restart_at_time" added (default no restart) to restart M5Stack regularly at predefined time to reconnect to WiFi access point and clear possible other errors. No startup sound during soft restart, snooze state reapplied, errors cleared.  
* New `M5NS.INI` key "restart_at_logged_errors" added (default no restart) to restart M5Stack after predefined amount of errors logged in error log to reconnect to WiFi access point and clear possible other errors. No startup sound during soft restart, snooze state reapplied, errors cleared.  
* Right button power icon changed to door icon to better express the page change/power off functions.  

### 12 June 2019

* More silent speaker. Found the way how to switch off adc1 after sound play.  
* Added token key in `M5NS.INI` to allow connection to secured Nightscout sites (thanks to Peter Leimbach).  
* Added keys snd_warning_at_startup and snd_alarm_at_startup to play warning/alarm sound test during startup (1 = play, 0 = do not play).  

### 09 June 2019

* More WiFi APs possible. Now you can create section [wlan0], [wlan1], up to [wlan9] in `M5NS.INI`.  
* Added SD card info for better error handling.  
* Added empty Nightscout check. No restarts repeat if Nightscout is empty.  
* Wait for NTP time synchronization.  

### 07 June 2019

* Added check for http/https in Nightscout URL in `M5NS.INI`  
* Increased default warning volume to 50.  
* Added last 2 weeks revisions to README.  

### 02 June 2019

* Large DELTA value displayed if no COB/IOB on display.  
* Sample `M5NS.INI` file now has default values in mg/dL.  
* Corrected volume bug, it did not work at all. Now accepts values from M5NI.INI correctly.  

### 30 May 2019 

* Added battery icon. This feature works only on newer M5Stack units. Removed seconds from time to make more place for possibly more icons.  

### 23 May 2019

* properties were not defined on Nightscout.  

### 18 May 2019

* Added button function icons (set `M5NS.INI` key info_line = 1). This is now default option.  
* Added loop and basal info (set `M5NS.INI` key info_line = 2).  
* Original sensor information available when `M5NS.INI` key info_line = 0  
* Small changes to silence background hiss as much as possible.  

### 12 May 2019

* BG/calibration/unknown entries are now filtered.  
* Missed reading sound alert added. You can adjust it by snd_no_readings key in `M5NS.INI` (default 20 minutes).  
* Added possibility to change warning sound volume by warning_volume key in `M5NS.INI` (0-100, default=20, 0=silent).  
* Added possibility to change alarm sound volume by alarm_volume key in `M5NS.INI` (0-100, default=100, 0=silent).  
* Reorganized left upper part of display to get space for COB and IOB display.  
* Added show_COB_IOB key to `M5NS.INI`. If show_COB_IOB = 1 then carbs and insulin on board are displayed. Set 1 (ON) by default.  
* COB and IOB are grey if 0 and white if any carbs or IU on board.  
* Added key show_current_time to `M5NS.INI`. If show_current_time = 1 (default now) then current clock is displayed instead of last sensor reding time.  

### 2 May 2019

* Snooze alarm function introduced and placed on the middle button.  
* New `M5NS.INI` key snooze_timeout (default 30 min) to specify time for how long should be sound alarm silent after press of the middle button.  
* New `M5NS.INI` key alarm_repeat to specify time (default 5 min) when sound alarm should repeat if its reason remains.  
* Corrected bug with alarm sometimes repeating twice.  
* WiFi symbol moved to the source code. External SD file is no more needed.  
* Configuration file `M5NS.INI` handling moved to separate source files.  

### 27 Apr 2019 - 2

Larger JSONDocument size for xDrip and possibly other Nightscout upload application compatibility.  
A little bit better HTTP error handling and error printing to the M5Stack screen.  

* When `show_mgdl = 1`, then all values in `M5NS.INI` have to be in mg/dL instead of mmol/L.  
* Updated device detection for xDrip.  

### 27 Apr 2019

* Only one query to Nightscout for minigraph as well as the last value. Faster code execution, less traffic.  

### 20 Apr 2019 - 2

* Added the main source code M5_NightscoutMon.ino to GitHub. Sorry I forgot in initial commit ;-)  

### 20 Apr 2019

* Initial GitHub commit  
