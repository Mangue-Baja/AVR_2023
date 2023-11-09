#ifndef SOFT_DEFS
#define SOFT_DEFS

#include <Arduino.h>

#define DEBOUCE_TIME 150 

typedef enum {INIT, WAIT, SD_BEGIN, MENU, RUN} state; 
typedef enum {START_} run_state;

uint8_t st = INIT;
uint8_t ss_r = START_;

unsigned long int t_curr = 0;
volatile unsigned long int t_30 = 0, t_100 = 0, t_101 = 0;
float vel = 0;

String str_30 = "00:000",
       str_100 = "00:000",
       str_101 = "00:000",
       str_vel = "00.00 km/h";

bool esp_now_ok = false;
bool conf_30 = false, conf_100 = false;

int8_t pot_sel = 0, old_pot = -1;

#endif      