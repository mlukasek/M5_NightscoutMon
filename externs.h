#ifndef _EXTERNS_H_
#define _EXTERNS_H_

#include <Arduino.h>
#include <WebServer.h>
#include "M5NSconfig.h"
#include <WiFiMulti.h>

extern WebServer w3srv;
extern tConfig cfg;
extern struct NSinfo ns;
extern WiFiMulti WiFiMultiple;
extern boolean mDNSactive;
extern int8_t getBatteryLevel();
extern void draw_page();
extern String M5NSversion;
extern int dispPage;
extern int maxPage;
extern void setPageIconPos(int page);
extern uint8_t lcdBrightness;

#endif
