#ifndef __MICRODOT_H
#define __MICRODOT_H

#include "Arduino.h"
#include "Wire.h"

class MicroDot{

public:
  uint8_t buf[6];
  MicroDot(uint8_t address=0x61);
  void begin();
  void clear();
  void setBrightness(uint8_t brightness);
  void writeDigit(uint8_t digit, uint8_t ch);
  void writeString(const char* buf);

private:
  uint8_t aDig12, aDig34, aDig56;
  void update(uint8_t address);
  void col2RowConv(byte digit, uint8_t* row);
};

#endif
