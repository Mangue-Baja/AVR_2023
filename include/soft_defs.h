#ifndef SOFT_DEFS
#define SOFT_DEFS

#include <Arduino.h>

#define DEBOUCE_TIME 150 

typedef enum { INIT, WAIT, MENU } state; 

uint8_t st = INIT;

#endif      