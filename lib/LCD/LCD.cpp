#include "LCD.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);
volatile unsigned long int t_30 = 0, t_100 = 0, t_101 = 0;
float vel = 0;
unsigned long int t_curr = 0;
String str_30 = "00:000",
       str_100 = "00:000",
       str_101 = "00:000",
       str_vel = "00.00 km/h";

void init_lcd()
{
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.print(F("Iniciando..."));
    delay(500);
}

void error_message()
{
    lcd.print("ESP-NOW ERROR");
    delay(2000);
    lcd.clear();
    lcd.print("Reiniciando");
    delay(1000);
    lcd.print('.');
    delay(1000);
    lcd.print('.');
    delay(1000);
    lcd.print('.');
}

void ok_message()
{
    lcd.clear();
    lcd.print(F("ESP-NOW ok!"));
    delay(500);
    lcd.clear();
}

void SD_status(bool status)
{
    String n = status ? "SD instalado" : "Nao ha SD";

    lcd.print(n);
    delay(120 * 5);
    lcd.clear();
}

void intro_text()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("MANGUE AV - 4x4"));
    lcd.setCursor(0, 1);
    lcd.print(F("   RUN"));
    lcd.setCursor(0, 1);
    lcd.print(" ");
    lcd.write('>');
}

void start_the_av_run(unsigned long t)
{
    lcd.clear();
    t_30 = 0;
    t_100 = 0;
    t_101 = 0;
    vel = 0;
    t_curr = t;
}

void start_the_av_run()
{
    t_30 = 0;
    t_100 = 0;
    t_101 = 0;
    vel = 0;
    str_vel = "00.00 km/h";
    printRun();
}

void save_tcurr_time(unsigned long t)
{
    t_curr = t;
}

void printRun(unsigned long t1, unsigned long t2)
{
    t_30 = t1 - t_curr;
    t_100 = t2 - t_curr;
    printRun();
}

void printRun(unsigned long t1)
{
    t_100 = t1 - t_curr;
    printRun();
}

void printRun()
{
    str_30 = format_time(t_30);
    str_100 = format_time(t_100);
    str_101 = format_time(t_101);

    lcd.setCursor(0, 0);
    lcd.print(' ' + str_30 + "  " + str_100 + "    ");
    lcd.setCursor(0, 1);
    lcd.print("   " + str_vel + "        ");
}

String format_time(unsigned long int t1)
{
    if (t1 < 10000)
        return '0' + String(t1 / 1000) + ':' + String(t1 % 1000);
    else
        return String(t1 / 1000) + ':' + String(t1 % 1000);
}

void save_t30(unsigned long t)
{
    t_30 = t;
    printRun();
}

void save_t100(unsigned long t)
{
    t_100 = t;
    printRun();
}

void save_speed(unsigned long t1, unsigned long t2)
{
    if (vel == 0)
    {
        vel = (float)((t2 - t1) / 1000.0);
        vel = (float)(3.6 / vel);
        str_vel = String(vel, 2) + "km/h";
    }
    printRun();
}

void sd_save_text(bool i)
{
    if (i)
        lcd.print(F("  Salvando...  "));
    else
    {
        lcd.print(F("Voltando"));
        delay(120 * 3);
        lcd.clear();
    }
    
}

void select_sd(uint8_t pos1, uint8_t pos2)
{
    lcd.setCursor(0, 0);
    lcd.print(F(" DESEJA SALVAR? "));
    lcd.setCursor(0, 1);
    lcd.print(F("   SIM    NAO   "));
    lcd.setCursor(pos1, pos2);
    lcd.write('>');
}

void save_in_SDcard(function_Sdsave f)
{
    f(t_30, t_100, vel);
}
