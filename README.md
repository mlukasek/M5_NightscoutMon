# M5_NightscoutMon
## M5Stack Nightscout monitor

###### M5Stack Nightscout monitor<br/>Copyright (C) 2018, 2019 Martin Lukasek <martin@lukasek.cz>
###### This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
###### This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
###### You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. 
###### This software uses some 3rd party libraries:<br/>IniFile by Steve Marple <stevemarple@googlemail.com> (GNU LGPL v2.1)<br/>ArduinoJson by Benoit BLANCHON (MIT License)
<br/>

### Revisions:

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
Initial GitHub commit<br/><br/>

### M5Stack Nightscout Monitor

M5Stack is small development kit based on ESP32. It is in a nice plastic box, equipped with color display, micro SD card slot, 3 buttons, speaker and internal battery. It is great to monitor and display something, so I used it to monitor my daughter's glycemia. It is nice, comfortable and cheap solution.

We use Abbott Freestyle Libre, together with MiaoMiao transmitter. It transmits the glycemia values to her mobile phone (we used to use Spike on iPhone, but you can also use Tomato on iPhone or xDrip, Glimp and more on Android). The daughter’s mobile phone then sends data to the Nightscout, where we (parents) watch it using the web application or a mobile phone (Spike, Nightscout, …). We use the small M5Stack monitor with my software to monitor the glycemia during nights in our sleeping room, my wife has one in the kitchen and we take one always on our trips. It has different backlight intensity for night usage and alarms for hypoglycemia situations.

A lot of people were asking for the software. I am not a professional programmer, I am a hardware engineer, but I work more with tables, numbers and e-mails recent years, so I was a little bit shy about the code. Now, when Spike has troubles, something that makes alarms during nights come handy, so I decided to release the code as and Open Source.

If you know what to do with it, just download the code, modify it any way it suits you and use it in M5Stack. If you do not know how to handle the source code, just find someone who can. It is easy, just open it in Arduino IDE, download M5Stack libraries, Arduino JSON library and build it to you M5Stack. M5Stack is in more versiona, the Core is just enough. Current price on Aliexpress is about 27.60 USD https://www.aliexpress.com/item/M5Stack-Official-Stock-Offer-ESP32-Basic-Core-Development-Kit-Extensible-Micro-Control-Wifi-BLE-IoT-Prototype/32837164440.html

You will need an microSD card too. It has to be formatted to FAT32 format and you have to put at least M5NS.INI file with configuration to the root of SD card. It is a good ideal to put WiFi_symbol.jpg and M5_NightscoutMon.jpg files to the SD card root too. You can replace M5_NightscoutMon.jpg with any picture you want to customize your experience.

You have to update M5NS.INI file to your requirements. As a minimum you will need to update your Nightscout URL and you’re your WiFi SSID and password.<br/>
<br/>

### The syntax of M5NS.INI configuration file

[config]

nightscout = yoursite.herokuapp.com – use your Nightscout site URL

bootpic = /M5_NightscoutMon.jpg – boot picture

name = YourName – display name

time_zone = 3600 – the time zone where you live in seconds (3600 is GMT+1, Prague)

dst = 3600 – your daylight saving time offset (3600s is +1 hour)

show_mgdl = 0 – use 0 to display values in mmol/L or 1 for mg/dl

show_current_time = 1 - show currnet time instead of last valid data time

show_COB_IOB = 1 - show COB and IOB, values are grayed out if COB/IOB value is 0



snooze_timeout = 30 - sound snooze time in minutes after press of the middle button

alarm_repeat = 5 - sound repeat interval in minutes (alarm repeats only if alarm/warnign conditions are met)



- following values are in mmol/L or mg/dl depending on the previous "show_mgdl" key value

yellow_low = 4.5 – glycemia values under this value will be displayed in yellow color

yellow_high = 9 – glycemia values over this value will be displayed in yellow color

red_low = 3.9 – glycemia values under this value will be displayed in red color

red_high = 11 – glycemia values over this value will be displayed in red color

snd_warning = 3.7 – softer high beep sound will beep every 5 mins when under this value

snd_alarm = 3.0 – strong alarm will sound every 5 min when under this value

snd_warning_high = 14.0 – softer high beep sound will beep every 5 mins when over this value

snd_alarm_high = 20.0 – strong alarm will sound every 5 min when over this value

snd_no_readings = 20 – softer high beep sound will beep every 5 mins when time in minutes when last data was read is over this value

warning_volume = 20 - volume of warning sound in range 0-100 (0=no sound, 1=silent, 100=max volume)

alarm_volume = 100 - volume of alarm sound in range 0-100 (0=no sound, 1=silent, 100=max volume)



info_line = 1 - 0 = sensor info, 1 = button function icons, 2 = loop info + basal



brightness1 = 50 – the first (default) brightness value

brightness2 = 100 – the second brightness value

brightness3 = 10 – the third brightness value



[wlan1] – you can connect up to 3 WiFi access point, no need to setup all, one is enough

ssid = ssid1

pass = pass1


[wlan2]

ssid = ssid2

pass = pass2


[wlan3]

ssid = ssid3

pass = pass3<br/>
<br/>

### Display

You will see “Nightscout” and user name from config file in upper left corner. You will see date when last valid date were uploaded and read from Nightscout bellow the name. Right from this is box with time difference from last valid data. 5 mins is OK, 5-15 mins is warning and the box will have white background, more then 15 mins is error and the box background will be red.

Glycemia value and its direction is bellow the date. There is a mini graph form 10 last values right of the glycemia value.

Last line contains data source.<br/>
<br/>

### Buttons

The left button changes the backlight in the 3 steps defined in M5NS.INI file.

The middle button wakes M5Stack from sleep.

The right button puts M5Stack to the sleep as it is a little bit tricky do double click standard power button.<br/>
<br/>

### Donations

If you find my project useful, you can donate me few bucks for further development or buy me a glass of wine 😉

https://paypal.me/8bity
