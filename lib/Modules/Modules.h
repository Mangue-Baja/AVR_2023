#ifndef MODULES_H
#define MODULES_H

#include <driver/gpio.h>
#include <WiFi.h>
#include "AV_espnow.h"
#include "packets.h"
#include "LCD.h"
#include "sd_device.h"

#define POT         GPIO_NUM_26 // Pino do potenciometro
#define B_SEL       GPIO_NUM_5  // Pino do botão para inicar o AVR
#define SENSOR_0m   GPIO_NUM_15 // Pino do sensor de 0 metros
#define SENSOR_30m  GPIO_NUM_5 // Pino do sensor de 30 metros
#define SENSOR_100m GPIO_NUM_5  // Pino do sensor de 100 metros
#define SENSOR_101m GPIO_NUM_18 // Pino do sensor de 101 metros

void printAddress(void);
int8_t potSelect(uint8_t pin, uint8_t num_options);

/* 0 meters Functions */
void init_AVR_ECU_0meters_communication(void);
void Callback_transmitter_0meters(const uint8_t* macAddr, esp_now_send_status_t status);
void Callback_receiver_0meters(const uint8_t* macAddr, const uint8_t* data, int len);

/* 30 meters Functions */
void init_30meters_communication(void);
void ISR_30m(void);
void Callback_for_30meters(const uint8_t* macAddr, const uint8_t* data, int len);

/* 100 meters Functions */
void init_100meters_communication(void);
void ISR_100m(void);
void Callback_for_100meters(const uint8_t* macAddr, const uint8_t* data, int len);

/* Bridge Functions */
void init_bridge_communication(void);
void Bridge_callback(const uint8_t* macAddr, const uint8_t* data, int len);

#endif