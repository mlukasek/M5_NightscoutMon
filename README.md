# M5_NightscoutMon
## M5Stack Nightscout monitor

##### M5Stack Nightscout monitor<br/>Copyright (C) 2018, 2019 Martin Lukasek <martin@lukasek.cz>
###### This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
###### This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
###### You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. 
###### This software uses some 3rd party libraries:<br/>IniFile by Steve Marple (GNU LGPL v2.1)<br/>ArduinoJson by Benoit BLANCHON (MIT License)<br/>IoT Icon Set by Artur Funk (GPL v3)<br/>DHT12 by Bobadas (Public domain)<br/><br/>Additions to the code:<br/>Peter Leimbach (Nightscout token)<br/>Patrick Sonnerat (Dexcom Sugarmate connection)<br/>Sulka Haro (Nightscout API queries help)

<img width="240" src="https://raw.githubusercontent.com/mlukasek/M5_NightscoutMon/master/images/M5NS_mon_2019-06-20_page1.jpg">&nbsp;&nbsp;<img width="240" src="https://raw.githubusercontent.com/mlukasek/M5_NightscoutMon/master/images/M5NS_mon_2019-06-20_page2.jpg">&nbsp;&nbsp;<img width="240" src="https://raw.githubusercontent.com/mlukasek/M5_NightscoutMon/master/images/M5NS_mon_2019-06-20_page3.jpg">


### Contents
[Latest software revisions](#revisions)  
[What is this good for?](#m5stack-nightscout-monitor-1)  
[Syntax of M5NS.INI configuration file](#the-syntax-of-m5nsini-configuration-file)  
[Display](#display)  
[Buttons](#buttons)  
[Battery](#battery)  
[Installation and support](#installation-and-support)  
[Donations - support the project](#donations) :+1:


### Revisions:

#### *** 10 October 2019 ***
Key invert_display (default = -1 when key is not present) added to M5NS.INI. You should not use it in M5NS.INI unless you have a problem with display inversion. Set invert_display = 0 or invert_display = 1 if you have troubles to call M5.Lcd.invertDisplay(0) or M5.Lcd.invertDisplay(1). Under normal circumstances 0 should be normal display and 1 should be inverted display.


#### *** 23 September 2019 ***
Key sgv_only (default 0) added to M5NS.INI. You should set it to 1 if you use xDrip, Spike or similar to filter out calibrations etc.
Explicit M5.Lcd.invertDisplay(0) added to try to prevent inverted display.

#### *** 21 September 2019 ***
Added support for Dexcom by using Sugarmate connection workaround. Thanks to Patrick Sonnerat.  
Only SGV entries are now queried, so no more troubles with calibration. Thanks to Sulka Haro.  
Fast page switching. No need to wait for NS data (does not work while accessing Nightscout = while blue WiFi icon displayed).  
New page with analog clock and Temperature/Humidity. Display environment values requires DHT12 - ENV Unit or BTC Standing Base.  
New key in M5NS.INI temperature_unit = 1 for CELSIUS, 2 for KELVIN, 3 for FAHRENHEIT. Can be omitted (default is Celsius).  
Display rotation possibility added. New key in M5NS.INI display_rotation = 1 (buttons down, default, can be omitted), 3 = buttons up, 5 = mirror buttons up, 7 = mirror buttons down.  
US date format added. New key in M5NS.INI date_format = 0 (dd.mm., default, can be omitted), 1 = MM/DD.  
JSON query update for some Bluetooth Glucose Meters.    

#### *** 20 June 2019 ***
Split of Nightscout read and display code (this should allow simpler user display code update and more different "faces" from users).  
New concept of display pages (different display designs, information, faces).  
Switch page by short press of the right button.  
Power OFF by the right button long press (4 seconds).  
Right button works also as power ON after power off by this button.  
New page added with large simple info (large BG, clock, delta + arrow and  few icons only).  
New M5NS.INI key "default_page" added (default 0).  
Smaller WiFi symbol (now as blue WiFi icon in 2 sizes for the 2 different Nightscout queries).  
Buttons do not work during Nightscout communication (blue WiFi symbol displayed).  
Bigger delta value even with COB+IOB values displayed (COB: and IOB: shortened to C: and I: ).  
Errors now logged silently (log can be displayed as the last page).  
Warning triangle icon added to show that errors are in the log (up to 5 errors - grey, more - yellow). Only last 10 errors can be displayed.  
New M5NS.INI key "restart_at_time" added (default no restart) to restart M5Stack regularly at predefined time to reconnect to WiFi access point and clear possible other errors. No startup sound during soft restart, snooze state reapplied, errors cleared.  
New M5NS.INI key "restart_at_logged_errors" added (default no restart) to restart M5Stack after predefined amount of errors logged in error log to reconnect to WiFi access point and clear possible other errors. No startup sound during soft restart, snooze state reapplied, errors cleared.  
Right button power icon changed to door icon to better express the page change/power off functions.  

#### *** 12 June 2019 ***
More silent speaker. Found the way how to switch off adc1 after sound play.  
Added token key in M5NS.INI to allow connection to secured Nightscout sites (thanks to Peter Leimbach).  
Added keys snd_warning_at_startup and snd_alarm_at_startup to play warning/alarm sound test during startup (1 = play, 0 = do not play).  

#### *** 09 June 2019 ***
More WiFi APs possible. Now you can create section [wlan0], [wlan1], up to [wlan9] in M5NS.INI.  
Added SD card info for better error handling.  
Added empty Nightscout check. No restarts repeat if Nightscout is empty.  
Wait for NTP time synchronization.  

#### *** 07 June 2019 ***
Added check for http/https in Nightscout URL in M5NS.INI  
Increased default warning volume to 50.  
Added last 2 weeks revisions to README.  

#### *** 02 June 2019 ***
Large DELTA value displayed if no COB/IOB on display.  
Sample M5NS.INI file now has default values in mg/dL.  
Corrected volume bug, it did not work at all. Now accepts values from M5NI.INI correctly.  

#### *** 30 May 2019 ***
Added battery icon. This feature works only on newer M5Stack units. Removed seconds from time to make more place for possibly more icons.  

#### *** 23 May 2019 ***
Fixed restart bug if some properties were not defined on Nightscout.  

#### *** 18 May 2019 ***
Added button function icons (set M5NS.INI key info_line = 1). This is now default option.  
Added loop and basal info (set M5NS.INI key info_line = 2).  
Original sensor information available when M5NS.INI key info_line = 0  
Small changes to silence background hiss as much as possible.  

#### *** 12 May 2019 ***
BG/calibration/unknown entries are now filtered.  
Missed reading sound alert added. You can adjust it by snd_no_readings key in M5NS.INI (default 20 minutes).  
Added possibility to change warning sound volume by warning_volume key in M5NS.INI (0-100, default=20, 0=silent).  
Added possibility to change alarm sound volume by alarm_volume key in M5NS.INI (0-100, default=100, 0=silent).  
Reorganized left upper part of display to get space for COB and IOB display.  
Added show_COB_IOB key to M5NS.INI. If show_COB_IOB = 1 then carbs and insulin on board are displayed. Set 1 (ON) by default.  
COB and IOB are grey if 0 and white if any carbs or IU on board.  
Added key show_current_time to M5NS.INI. If show_current_time = 1 (default now) then current clock is displayed instead of last sensor reding time.  

#### *** 2 May 2019 ***
Snooze alarm function introduced and placed on the middle button.  
New M5NS.INI key snooze_timeout (default 30 min) to specify time for how long should be sound alarm silent after press of the middle button.  
New M5NS.INI key alarm_repeat to specify time (default 5 min) when sound alarm shoud repeat if its reason remains.  
Corrected bug with alarm sometimes repeating twice.  
WiFi symbol moved to the source code. External SD file is no more needed.  
Configuration file M5NS.INI handling moved to separate source files.  

#### *** 27 Apr 2019 - 2 ***
Larger JSONDocument size for xDrip and possibly other Nightscout upload application compatibility.  
A little bit better HTTP error handling and error printing to the M5Stack screen.  
When "show_mgdl = 1", then all values in M5NS.INI have to be in mg/dL instead of mmol/L.  
Updated device detection for xDrip.  

#### *** 27 Apr 2019 ***
Only one query to Nightscout for minigraph as well as the last value. Faster code execution, less traffic.  

#### *** 20 Apr 2019 - 2 ***
Added the main source code M5_NightscoutMon.ino to GitHub. Sorry I forgot in initial commit ;-)  

#### *** 20 Apr 2019 ***
Initial GitHub commit  
<br/>

### M5Stack Nightscout Monitor

M5Stack is small development kit based on ESP32. It is in a nice plastic box, equipped with color display, micro SD card slot, 3 buttons, speaker and internal battery. It is great to monitor and display something, so I used it to monitor my daughter's glycemia. It is nice, comfortable and cheap solution.

We use Abbott Freestyle Libre, together with MiaoMiao transmitter. It transmits the glycemia values to her mobile phone (we used to use Spike on iPhone, but you can also use Tomato on iPhone or xDrip, Glimp and more on Android). The daughter’s mobile phone then sends data to the Nightscout, where we (parents) watch it using the web application or a mobile phone (Spike, Nightscout, …). We use the small M5Stack monitor with my software to monitor the glycemia during nights in our sleeping room, my wife has one in the kitchen and we take one always on our trips. It has different backlight intensity for night usage and alarms for hypo/hyper glycemia situations.

**In short it displays information from Nightscout site and provides visual and sound alarms if needed.**

Some people were asking for the software. I am not a professional programmer, I am a hardware engineer, but I work more with tables, numbers and e-mails recent years, so I was a little bit shy about the code. As there is quite a lot of people interested in something that shows current values and makes alarms during nights, I decided to release the code as an Open Source.

If you know what to do with it, just download the code, modify it any way it suits you and use it in M5Stack. If you do not know how to handle the source code, just find someone who can. It is easy, just open it in Arduino IDE, download M5Stack libraries, Arduino JSON library and build it to you M5Stack. M5Stack is in more versions, the Core is just enough. Current price on Aliexpress is about 27.95 USD https://www.aliexpress.com/item/M5Stack-Official-Stock-Offer-ESP32-Basic-Core-Development-Kit-Extensible-Micro-Control-Wifi-BLE-IoT-Prototype/32837164440.html

**You will need a microSD card.** It has to be formatted to FAT32 format and you have to put at least M5NS.INI file with configuration to the root of the microSD card. It is a good idea to put M5_NightscoutMon.jpg files to the SD card root too. You can replace M5_NightscoutMon.jpg with any 320x240 pixels picture if you want to customize your experience.

**You have to update M5NS.INI file to your requirements. As a minimum requirement you will need to update your Nightscout URL and your WiFi SSID and password.**<br/>
<br/>

### The syntax of M5NS.INI configuration file

[config]

nightscout = yoursite.herokuapp.com _– use your Nightscout site URL_  
token = security token _- token for access secured Nightscout site, if public Nightscout site is used, delete token line and do not use it_  
bootpic = /M5_NightscoutMon.jpg _– boot picture_  
name = YourName _– display name_  
time_zone = 3600 _– the time zone where you live in seconds (3600 is GMT+1, Prague)_  
dst = 3600 _– your daylight saving time offset (3600s is +1 hour)_  
show_mgdl = 0 _– use 0 to display values in mmol/L or 1 for mg/dl_  
sgv_only = 0 _- set to 1 if you use xDrip, Spike or similar to filter out calibrations etc_  
default_page = 0 _– page number displayed after startup_  
show_current_time = 1 _– show current time instead of last valid data time_  
restart_at_time = HH:MM _– time in HH:MM format when the device will restart_  
restart_at_logged_errors = 0 _- restart device after particular number of errors in the log (0 = do not restart)_  
show_COB_IOB = 1 _– show COB and IOB, values are grayed out if COB/IOB value is 0_  
snooze_timeout = 30 _- sound snooze time in minutes after press of the middle button_  
alarm_repeat = 5 _- sound repeat interval in minutes (alarm repeats only if alarm/warning conditions are met)_  
info_line = 1 _- 0 = sensor info, 1 = button function icons, 2 = loop info + basal_  
temperature_unit = 1 _- 1 = CELSIUS, 2 = KELVIN, 3 = FAHRENHEIT. Can be omitted (default is Celsius)._  
display_rotation = 1 _- 1 = buttons down (default, can be omitted), 3 = buttons up, 5 = mirror buttons up, 7 = mirror buttons down_  
date_format = 0 _- 0 = dd.mm. (default, can be omitted), 1 = MM/DD_  
brightness1 = 50 _– the first (default) brightness value_  
brightness2 = 100 _– the second brightness value_  
brightness3 = 10 _– the third brightness value_

_- following values are in mmol/L or mg/dL depending on the previous setting of "show_mgdl" key value_

yellow_low = 4.5 _– glycemia values under this value will be displayed in yellow color_  
yellow_high = 9 _– glycemia values over this value will be displayed in yellow color_  
red_low = 3.9 _– glycemia values under this value will be displayed in red color_  
red_high = 11 _– glycemia values over this value will be displayed in red color_  
snd_warning = 3.7 _– softer high beep sound will beep every 5 mins when under this value_  
snd_alarm = 3.0 _– strong alarm will sound every 5 min when under this value_  
snd_warning_high = 14.0 _– softer high beep sound will beep every 5 mins when over this value_  
snd_alarm_high = 20.0 _– strong alarm will sound every 5 min when over this value_  
snd_no_readings = 20 _– softer high beep sound will beep every 5 mins when time in minutes when last data was read is over this value_  
snd_warning_at_startup = 1 _- play test warning sound with its volume during startup (1 = play, 0 =  do not play)_  
snd_alarm_at_startup = 0 _- play test alarm sound with its volume during startup (1 = play, 0 =  do not play)_  
warning_volume = 20 _- volume of warning sound in range 0-100 (0=no sound, 1=silent, 100=max volume)_  
alarm_volume = 100 _- volume of alarm sound in range 0-100 (0=no sound, 1=silent, 100=max volume)

[wlan1] _– you can connect up to 9 WiFi access point, no need to setup all, one is enough_  
ssid = ssid1  
pass = pass1

[wlan2]  
ssid = ssid2  
pass = pass2

[wlan3]  
...<br/>
<br/>

### Display

There are currently 3 display pages, that can be changed by a short press of the right button. The button does not work when the device communicated with Nightscout (small blue WiFi icon is displayed).

**Page 0 - default page**  
What you see on the display is a matter of your M5NS.INI and Nightscout configuration. By default you will see current time, date and user name from config file in upper left corner. You will see IOB (Insulin On Board) an COB (Carbs On Board) values bellow the name. Right from the IOB/COB value is current BG delta (difference from the last reading, trend where the BG is going). Box with time difference from last valid data from Nighscout is displayed in upper right corner. Delay of 5 mins is OK and the box has grey color, 5-15 mins is warning and the box will have white background, more than 15 mins without a valid data is error and the box background will be red. There is also possibility to set sound warning on data delay in M5NS.INI.

Glycemia (BG) value and its direction are bellow the basic information. There is a mini graph created from 10 last BG values right of the glycemia value. The mini graph shows only values in range 3-12 mmol/L (48-216 mg/dL) and values bellow/above this will be displayed on mini graph as the min/max values.

There are also icons in upper side of display. Plug or battery icon displays battery status (on newer devices only). Red clock indicated silent "snooze" mode activated. Grey or yellow (more than 5 errors) shows that there are errors in error log (last page). Blue WiFi icon in 2 sizes (2 different queries) indicated that MStack is downloading data from Nightscout.

Last line contains icons for buttons or data source or Loop information (depending on M5NS.INI key "info_line").  

**Page 1 - simple page with large BG value**  
Long distance visible page with large BG value and larger clock. Top line contains current time, icons, delta value a BG direction arrow.

Bottom info line is the same as on page 0.

**Page 2 - analog clock with temperature and humidity**  
Large analog clock with date. BG value in top left corner with time difference from the last reading. BG delta value in top right corner and arrow pointing chinge direction bellow it.

This screen contatins environmental values form DHT12 sensor. Requires ENV Unit or BTC Standing Base connected to MStack. The temperature is in lower left corner and humidity in lower right corner. Temperature unit can be set in M5NS.INI.

Bottom info line is the same as on page 0.

**Error Log page**
Displays last 10 record errors and total number of errors record since last restart. Occasional communication errors, such as "timeout" or "connection refused" are normal and happen sometimes due to WiFi and internet connection and Nightscout site status. If you have a lot of errors every few minutes, consider to move your M5Stack to a different place or extend WiFi signal.  
<br/>

### Buttons

The left button changes the backlight in the 3 steps defined in M5NS.INI file.

The middle button snoozes M5Stack for defined time to prevent alarm or warning sound. You can "snooze" it even before the alarm is activated. Snooze active is indicated by red clock icon in status icon area.

Short press of the right button switches display pages. Long press (4 seconds) puts M5Stack to the sleep as it is a little bit tricky to double click standard red power button.  
<br/>

### Battery

Battery icon feature works only on newer M5Stack units. Here are my battery measurements and battery icon statuses:

**internal battery total 43 minutes**  
plug icon = full charge or plugged in ... 1 min  
gray full battery icon ... 13 min  
white 2/3 battery icon ... 20 min  
yellow 1/3 battery icon ... 8 min  
red empty battery icon ... 1 min  

**extended battery total 7:49 hrs:min**  
plug icon = full charge or plugged in ... 53 min  
gray full battery icon ... 2 hrs 59 min  
white 2/3 battery icon ... 1 hr 55 min  
yellow 1/3 battery icon ... 1 hr 48 min  
red empty battery icon ... 14 min  
<br/>

### Installation and support

Installation can be done by download or git clone the code to the Arduino IDE environment. You have to download ESP32 board and M5Stack libraries, Arduino JSON library and build it to you M5Stack.

Another easier possibility is to download latest [M5Burner release](https://github.com/mlukasek/M5_NightscoutMon/releases). It is Windows executable with binary M5Stack firmware included. Just unzip it, start the M5Burner.exe, choose COM port where your M5Stack is connected and burn the firmware. Simple. Remember to add you microSD card with M5NS.INI configuration file.

There is a Facebook group [M5STACK NIGHTSCOUT](https://www.facebook.com/groups/606295776549008/) where you can get support and installation guides in several languages prepared by members of the M5Stack Nightscout community. Please check the [Files section](https://www.facebook.com/groups/606295776549008/files/) first and search the group before asking questions. A lot of questions have been answered already. Big thanks to Patrick Sonnerat, Didier Frétigné, Peter Leimbach and more...  
<br/>

### Donations

If you find my project useful, I will appreciate if you donate me few bucks for further development or buy me a glass of beer or wine :wine_glass:.

https://paypal.me/8bity

I also collect and repair vintage [8-bit computers](http://www.8bity.cz/), so if you have some really old computer, I could give it a new home and care :smile:.
