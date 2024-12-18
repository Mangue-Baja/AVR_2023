#ifndef HARDWARE_DEFS_H
#define HARDWARE_DEFS_H

#include <driver/gpio.h>

/* Modulo em 0m */
#define SD_CS       GPIO_NUM_2  // Pino CS do modulo SD
#define POT         GPIO_NUM_35 // Pino do potenciometro
#define B_SEL       GPIO_NUM_14 // Pino do botão para inicar o AVR
#define B_CANCEL    GPIO_NUM_13 // Pino do botão para resetar ou cancelar o AVRs
#define SENSOR_0m   GPIO_NUM_15 // Pino do sensor de 0 metros

/* Modulo em 30m */
#define SENSOR_30m  GPIO_NUM_5  // Pino do sensor de 30 metros

/* Modulo em 100m */
#define SENSOR_100m GPIO_NUM_5  // Pino do sensor de 100 metros
#define SENSOR_101m GPIO_NUM_18 // Pino do sensor de 101 metros

#endif
