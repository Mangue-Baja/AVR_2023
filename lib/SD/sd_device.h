#ifndef SDDEVICE_H
#define SDDEVICE_H

#include <Arduino.h>
#include <driver/gpio.h>
#include <SD.h>

#define SD_CS GPIO_NUM_4 // Pino CS do modulo SD

bool init_sd(void);
void save_Data(unsigned long _t30, unsigned long _t100, float _v);

#endif