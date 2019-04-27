# M5_NightscoutMon
M5Stack Nightscout monitor


Revisions:

*** 27 Apr 2019 - 2 ***
Larger JSONDocument size for xDrip and possibly other Nightscout upload application compatibility.
A little bit better HTTP error handling and error printing to the M5Stack screen.
When "show_mgdl = 1", then all values in M5NS.INI have to be in mg/dL instead of mmol/L.
Updated device detection for xDrip.

*** 27 Apr 2019 ***
Only one query to Nightscout for minigraph as well as the last value. Faster code execution, less traffic.

*** 20 Apr 2019 - 2 ***
Added the main source code M5_NightscoutMon.ino to GitHub. Sorry I forgot in initital commit ;-)

*** 20 Apr 2019 ***
Initial GitHub commit



M5Stack Nightscout Monitor


M5Stack is small development kit based on ESP32. It is in a nice plastic box, equipped with color display, micro SD card slot, 3 buttons, speaker and internal battery. It is great to monitor and display something, so I used it to monitor my daughter's glycemia. It is nice, comfortable and cheap solution.

We use Abbott Freestyle Libre, together with MiaoMiao transmitter. It transmits the glycemia values to her mobile phone (we used to use Spike on iPhone, but you can also use Tomato on iPhone or xDrip, Glimp and more on Android). The daughter’s mobile phone then sends data to the Nightscout, where we (parents) watch it using the web application or a mobile phone (Spike, Nightscout, …). We use the small M5Stack monitor with my software to monitor the glycemia during nights in our sleeping room, my wife has one in the kitchen and we take one always on our trips. It has different backlight intensity for night usage and alarms for hypoglycemia situations.

A lot of people were asking for the software. I am not a professional programmer, I am a hardware engineer, but I work more with tables, numbers and e-mails recent years, so I was a little bit shy about the code. Now, when Spike has troubles, something that makes alarms during nights come handy, so I decided to release the code as and Open Source.

If you know what to do with it, just download the code, modify it any way it suits you and use it in M5Stack. If you do not know how to handle the source code, just find someone who can. It is easy, just open it in Arduino IDE, download M5Stack libraries, Arduino JSON library and build it to you M5Stack. M5Stack is in more versiona, the Core is just enough. Current price on Aliexpress is about 27.60 USD https://www.aliexpress.com/item/M5Stack-Official-Stock-Offer-ESP32-Basic-Core-Development-Kit-Extensible-Micro-Control-Wifi-BLE-IoT-Prototype/32837164440.html

You will need an microSD card too. It has to be formatted to FAT32 format and you have to put at least M5NS.INI file with configuration to the root of SD card. It is a good ideal to put WiFi_symbol.jpg and M5_NightscoutMon.jpg files to the SD card root too. You can replace M5_NightscoutMon.jpg with any picture you want to customize your experience.

You have to update M5NS.INI file to your requirements. As a minimum you will need to update your Nightscout URL and you’re your WiFi SSID and password. 

The syntax of M5NS.INI is following:

[config]

nightscout = yoursite.herokuapp.com – use your Nightscout site URL

bootpic = /M5_NightscoutMon.jpg – boot picture

name = YourName – display name

time_zone = 3600 – the time zone where you live in seconds (3600 is GMT+1, Prague)

dst = 3600 – your daylight saving time offset (3600s is +1 hour)


show_mgdl = 0 – use 0 to display values in mmol/L or 1 for mg/dl


- following values are in mmol/L or mg/dl depending on the previous "show_mgdl" key value

yellow_low = 4.5 – glycemia values under this value will be displayed in yellow color

yellow_high = 9 – glycemia values over this value will be displayed in yellow color

red_low = 3.9 – glycemia values under this value will be displayed in red color

red_high = 11 – glycemia values over this value will be displayed in red color

snd_warning = 3.7 – high beep sound will beep every 5 mins when under this value

snd_alarm = 3.0 – strong alarm will sound every 5 min when under this value


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

pass = pass3


Display

You will see “Nightscout” and user name from config file in upper left corner. You will see date when last valid date were uploaded and read from Nightscout bellow the name. Right from this is box with time difference from last valid data. 5 mins is OK, 5-15 mins is warning and the box will have white background, more then 15 mins is error and the box background will be red.

Glycemia value and its direction is bellow the date. There is a mini graph form 10 last values right of the glycemia value.

Last line contains data source.


Buttons

The left button changes the backlight in the 3 steps defined in M5NS.INI file.

The middle button wakes M5Stack from sleep.

The right button puts M5Stack to the sleep as it is a little bit tricky do double click standard power button.


Donations

If you find my project useful, you can donate me few bucks for further development or buy me a glass of wine 😉

https://paypal.me/8bity
