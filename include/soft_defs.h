#ifndef SOFT_DEFS
#define SOFT_DEFS

#include <Arduino.h>

typedef enum {INIT, WAIT, BEGIN, MENU, RUN, CONFIG, ST_30, ST_100_101} state;

uint8_t st = INIT;

uint8_t old_pot = -1, pot_sel = 0;
uint8_t pos[4];

bool listen_30 = false;
bool listen_100_101 = false;

#endif      