#ifndef SDDEVICE_H
#define SDDEVICE_H

#include <Arduino.h>
#include <SD.h>

bool init_sd(uint8_t pin);
bool save_AV_Data(unsigned long _t30, unsigned long _t100, float _v);

#endif