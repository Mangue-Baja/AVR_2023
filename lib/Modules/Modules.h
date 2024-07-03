#ifndef MODULES_H
#define MODULES_H

#include <driver/gpio.h>
#include <WiFi.h>
#include "AV_espnow.h"
#include "packets.h"

#define SENSOR_30m GPIO_NUM_15 // Pino do sensor de 30 metros

/* 30 meters Functions */
void init_30meters_communication(void);
void ISR_30m(void);
void Callback_for_30meters(const uint8_t* macAddr, const uint8_t* data, int len);

/* Bridge Functions */
void init_bridge_communication(void);
void Bridge_callback(const uint8_t* macAddr, const uint8_t* data, int len);

#endif