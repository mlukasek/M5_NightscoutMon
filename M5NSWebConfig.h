#ifndef _M5NSWEBCONFIG_H
#define _M5NSWEBCONFIG_H

#include <Adafruit_NeoPixel.h>
extern Adafruit_NeoPixel pixels;

#include "microdot.h"
extern MicroDot MD;

void handleRoot();
void handleUpdate();
void handleSwitchConfig();
void handleEditConfigItem();
void handleGetEditConfigItem();
void handleSaveConfig();
void handleClearConfigFlash();
void handleNotFound();

#endif
