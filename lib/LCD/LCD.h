#ifndef LCD_H
#define LCD_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

typedef bool (*function_Sdsave)(unsigned long, unsigned long, float);

void init_lcd(void);
void error_message(void);
void ok_message(void);
void SD_status(bool status);
void intro_text(void);
void start_the_av_run(unsigned long t);
void start_the_av_run(void);
void save_tcurr_time(unsigned long t);
void printRun(unsigned long t1, unsigned long t2);
void printRun(unsigned long t1);
void printRun(void);
String format_time(unsigned long int t1);
void save_t30(unsigned long t);
void save_t100(unsigned long t);
void save_speed(unsigned long t1, unsigned long t2);
void sd_save_text(function_Sdsave sv_dt);
void sd_save_text(void);
void select_sd(uint8_t pos1);

#endif