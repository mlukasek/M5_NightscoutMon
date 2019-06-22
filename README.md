# M5_NightscoutMon
## M5Stack Nightscout monitor

##### M5Stack Nightscout monitor<br/>Copyright (C) 2018, 2019 Martin Lukasek <martin@lukasek.cz>
###### This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
###### This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
###### You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. 
###### This software uses some 3rd party libraries:<br/>IniFile by Steve Marple <stevemarple@googlemail.com> (GNU LGPL v2.1)<br/>ArduinoJson by Benoit BLANCHON (MIT License)<br/>IoT Icon Set by Artur Funk (GPL v3)  
###### Additions to the code:<br/>Peter Leimbach (Nightscout token)


### Contents
[Latest software revisions](#revisions)  
[What is this good for?](#m5stack-nightscout-monitor-1)  
[Syntax of M5NS.INI configuration file](#the-syntax-of-m5nsini-configuration-file)  
[Dipsplay](#display)  
[Buttons](#buttons)  
[Donations - support the project](#donations) :+1:


### Revisions:

#### *** 20 June 2019 ***
Split of Nightscout read and display code (this should allow simpler user display code update and more different "faces" from users).<br/>
New concept of display pages (different display designs, information, faces).<br/>
Switch page by short press of the right button.<br/>
Power OFF by the right button long press (4 seconds).<br/>
Right button works also as power ON after power off by this button.<br/>
New page added with large simple info (large BG, clock, delta + arrow and  few icons only).<br/>
New M5NS.INI key "default_page" added (default 0).<br/>
Smaller WiFi symbol (now as blue WiFi icon in 2 sizes for the 2 different Nightscout queries).<br/>
Buttons do not work during Nightscout communication (blue WiFi symbol displayed).<br/>
Bigger delta value even with COB+IOB values displayed (COB: and IOB: shortened to C: and I: ).<br/>
Errors now logged silently (log can be displayed as the last page).<br/>
Warning triangle icon added to show that errors are in the log (up to 5 errors - grey, more - yellow). Only last 10 errors can be displayed.<br/>
New M5NS.INI key "restart_at_time" added (default no restart) to restart M5Stack regularly at predefined time to reconnect to WiFi access point and clear possible other errors. No startup sound during soft restart, snooze state reapplied, errors cleared.<br/>
New M5NS.INI key "restart_at_logged_errors" added (default no restart) to restart M5Stack after predefined amount of errors logged in error log to reconnect to WiFi access point and clear possible other errors. No startup sound during soft restart, snooze state reapplied, errors cleared.<br/>
Right button power icon changed to door icon to better express the page change/power off functions.<br/>

#### *** 12 June 2019 ***
More silent speaker. Found the way how to switch off adc1 after sound play.<br/>
Added token key in M5NS.INI to allow connection to secured Nightscout sites (thanks to Peter Leimbach).<br/>
Added keys snd_warning_at_startup and snd_alarm_at_startup to play warning/alarm sound test during startup (1 = play, 0 = do not play).<br/>

#### *** 09 June 2019 ***
More WiFi APs possible. Now you can create section [wlan0], [wlan1], up to [wlan9] in M5NS.INI.<br/>
Added SD card info for better error handling.<br/>
Added empty Nightscout check. No restarts repeat if Nightscout is empty.<br/>
Wait for NTP time synchronization.<br/>

#### *** 07 June 2019 ***
Added check for http/https in nightscout URL in M5NS.INI<br/>
Increased deafult warning volume to 50.<br/>
Added last 2 weeks revisions to README.<br/>

#### *** 02 June 2019 ***
Large DELTA value displayed if no COB/IOB on display.<br/>
Sample M5NS.INI dile now has default values in mg/dL.<br/>
Corrected volume bug, it did not work at all. Now accepts values from M5NI.INI correctly.<br/>

#### *** 30 May 2019 ***
Added battery icon. This feature works only on newer M5Stack units. Removed seconds from time to make more place for possibly more icons.<br/>

#### *** 23 May 2019 ***
Fixed restart bug if some properties were not defined on Nightscout.<br/>

#### *** 18 May 2019 ***
Added button function icons (set M5NS.INI key info_line = 1). This is now default option.<br/>
Added loop and basal info (set M5NS.INI key info_line = 2)<br/>
Original sensor information available when M5NS.INI key info_line = 0<br/>
Small changes to silence background hiss as much as possible.<br/>

#### *** 12 May 2019 ***
BG/calibration/unknown entries are now filtered.<br/>
Missed reading sound alert added. You can adjust it by snd_no_readings key in M5NS.INI (default 20 minutes).<br/>
Added possibility to change warning sound volume byt warning_volume key in M5NS.INI (0-100, default=20, 0=silent).<br/>
Added possibility to change alarm sound volume byt alarm_volume key in M5NS.INI (0-100, default=100, 0=silent).<br/>
Reorganized left upper part of display to get space for COB and IOB display.<br/>
Added show_COB_IOB key to M5NS.INI. If show_COB_IOB = 1 then carbs and insulin on board are displayed. Set 1 (ON) by default.<br/>
COB and IOB are grey if 0 and white if any carbs or IU on board.<br/>
Added key show_current_time to M5NS.INI. If show_current_time = 1 (default now) then current clock is displayed instead of last sensor reding time.<br/>

#### *** 2 May 2019 ***
Snooze alarm function introduced and placed on the middle button.<br/>
New M5NS.INI key snooze_timeout (default 30 min) to specify time for how long should be sound alarm silent after press of the middle button.<br/>
New M5NS.INI key alarm_repeat to specify time (default 5 min) when sound alarm shoud repeat if its reason remains.<br/>
Corrected bug with alarm sometimes repeating twice.<br/>
WiFi symbol moved to the source code. External SD file is no more needed.<br/>
Configuration file M5NS.INI handling moved to separate source files.<br/>

#### *** 27 Apr 2019 - 2 ***
Larger JSONDocument size for xDrip and possibly other Nightscout upload application compatibility.<br/>
A little bit better HTTP error handling and error printing to the M5Stack screen.<br/>
When "show_mgdl = 1", then all values in M5NS.INI have to be in mg/dL instead of mmol/L.<br/>
Updated device detection for xDrip.<br/>

#### *** 27 Apr 2019 ***
Only one query to Nightscout for minigraph as well as the last value. Faster code execution, less traffic.<br/>

#### *** 20 Apr 2019 - 2 ***
Added the main source code M5_NightscoutMon.ino to GitHub. Sorry I forgot in initital commit ;-)<br/>

#### *** 20 Apr 2019 ***
Initial GitHub commit<br/>
<br/>

### M5Stack Nightscout Monitor

M5Stack is small development kit based on ESP32. It is in a nice plastic box, equipped with color display, micro SD card slot, 3 buttons, speaker and internal battery. It is great to monitor and display something, so I used it to monitor my daughter's glycemia. It is nice, comfortable and cheap solution.

We use Abbott Freestyle Libre, together with MiaoMiao transmitter. It transmits the glycemia values to her mobile phone (we used to use Spike on iPhone, but you can also use Tomato on iPhone or xDrip, Glimp and more on Android). The daughter’s mobile phone then sends data to the Nightscout, where we (parents) watch it using the web application or a mobile phone (Spike, Nightscout, …). We use the small M5Stack monitor with my software to monitor the glycemia during nights in our sleeping room, my wife has one in the kitchen and we take one always on our trips. It has different backlight intensity for night usage and alarms for hypo/hyper glycemia situations.

Some people were asking for the software. I am not a professional programmer, I am a hardware engineer, but I work more with tables, numbers and e-mails recent years, so I was a little bit shy about the code. As there is guite a lot of people insterested in something that shows current values and makes alarms during nights, I decided to release the code as an Open Source.

If you know what to do with it, just download the code, modify it any way it suits you and use it in M5Stack. If you do not know how to handle the source code, just find someone who can. It is easy, just open it in Arduino IDE, download M5Stack libraries, Arduino JSON library and build it to you M5Stack. M5Stack is in more versions, the Core is just enough. Current price on Aliexpress is about 27.95 USD https://www.aliexpress.com/item/M5Stack-Official-Stock-Offer-ESP32-Basic-Core-Development-Kit-Extensible-Micro-Control-Wifi-BLE-IoT-Prototype/32837164440.html

**You will need an microSD card.** It has to be formatted to FAT32 format and you have to put at least M5NS.INI file with configuration to the root of the microSD card. It is a good idea to put M5_NightscoutMon.jpg files to the SD card root too. You can replace M5_NightscoutMon.jpg with any 320x240 pixels picture if you want to customize your experience.

**You have to update M5NS.INI file to your requirements. As a minimum reuirement you will need to update your Nightscout URL and you’re your WiFi SSID and password.**<br/>
<br/>

### The syntax of M5NS.INI configuration file

[config]

nightscout = yoursite.herokuapp.com _– use your Nightscout site URL_  
token = security token _- token for acces secured Nightscout site, if public Nightscout site is used, delete token line and do not use it_  
bootpic = /M5_NightscoutMon.jpg _– boot picture_  
name = YourName _– display name_  
time_zone = 3600 _– the time zone where you live in seconds (3600 is GMT+1, Prague)_  
dst = 3600 _– your daylight saving time offset (3600s is +1 hour)_  
show_mgdl = 0 _– use 0 to display values in mmol/L or 1 for mg/dl_  
default_page = 0 _– page number displayed after startup_  
show_current_time = 1 _– show currnet time instead of last valid data time_  
restart_at_time = HH:MM _– time in HH:MM format when the device will restart_  
restart_at_logged_errors = 0 _- restart device after particular number of errors in the log (0 = do not restart)_  
show_COB_IOB = 1 _– show COB and IOB, values are grayed out if COB/IOB value is 0_  
snooze_timeout = 30 _- sound snooze time in minutes after press of the middle button_  
alarm_repeat = 5 _- sound repeat interval in minutes (alarm repeats only if alarm/warnign conditions are met)_  
info_line = 1 _- 0 = sensor info, 1 = button function icons, 2 = loop info + basal_  
brightness1 = 50 _– the first (default) brightness value_  
brightness2 = 100 _– the second brightness value_  
brightness3 = 10 _– the third brightness value_

- following values are in mmol/L or mg/dL depending on the previous "show_mgdl" key value

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
What you see on the display is a matter of your M5NS.INI and Nightscout configuration. By default you will see current time, date and user name from config file in upper left corner. You will see IOB (Insulin On Board) an COB (Carbs On Board) values bellow the name. Right from the IOB/COB valuew is current BG delta (difference from the last reading, trend where the BG is going). Box with time difference from last valid data from Nighscout is displayed in upper tight corner. Delay of 5 mins is OK and the box has grey color, 5-15 mins is warning and the box will have white background, more then 15 mins without a valid data is error and the box background will be red. There is also possibility to set sound warning on data delay in M5NS.INI.

Glycemia (BG) value and its direction is bellow the basic information. There is a mini graph form 10 last values right of the glycemia value. The minigraph shows only values in range 3-12 mmol/L (48-216 mg/dL) and values bellow/above this will be displayed on minigraph as the min/max values.

Last line contains icons for buttons or data source or Loop information (depending on M5NS.INI key "info_line").<br/>
<br/>

### Buttons

The left button changes the backlight in the 3 steps defined in M5NS.INI file.

The middle button wakes M5Stack from sleep.

The right button puts M5Stack to the sleep as it is a little bit tricky do double click standard power button.<br/>
<br/>

### Donations

If you find my project useful, you can donate me few bucks for further development or buy me a glass of wine 😉

https://paypal.me/8bity
