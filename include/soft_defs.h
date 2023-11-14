#ifndef SOFT_DEFS
#define SOFT_DEFS

#include <Arduino.h>

#define DEBOUCE_TIME 150 

typedef enum {INIT, WAIT, SD_BEGIN, MENU, RUN} state; 
typedef enum {START_, LCD_DISPLAY, WAIT_30, WAIT_100, END_RUN, SAVE_RUN} run_state;

uint8_t ss_t = INIT;
uint8_t ss_r = START_;

unsigned long int t_curr = 0;
volatile unsigned long int t_30 = 0, t_100 = 0, t_101 = 0;
float vel = 0;

typedef struct {
    uint8_t flag = 0x00;
    volatile unsigned long int tt_30 = 0, tt_100 = 0, tt_101 = 0;
} packet_t;

packet_t packet;

String str_30 = "00:000",
       str_100 = "00:000",
       str_101 = "00:000",
       str_vel = "00.00 km/h";

bool esp_now_ok = false;
bool conf_30 = false, conf_100 = false;

uint8_t pos[2];
int8_t pot_sel = 0, old_pot = -1;

#endif      