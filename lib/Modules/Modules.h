#ifndef MODULES_H
#define MODULES_H

#include <WiFi.h>
#include <Ticker.h>
#include "AV_espnow.h"
#include "hardware_defs.h"
#include "packets.h"
#include "LCD.h"
#include "sd_device.h"

void printAddress(void);
int8_t potSelect(uint8_t pin);

/* 0 meters Functions */
void init_AVR_ECU_0meters_communication(void);
void enable_reset(void);
void Callback_transmitter_0meters(const uint8_t *macAddr, esp_now_send_status_t status);
void Callback_receiver_0meters(const uint8_t *macAddr, const uint8_t *data, int len);

/* 30 meters Functions */
void init_30meters_communication(void);
void ISR_30m(void);
void Callback_for_30meters(const uint8_t *macAddr, const uint8_t *data, int len);

/* 100 meters Functions */
void init_100meters_communication(void);
void ISR_100m(void);
void Callback_for_100meters(const uint8_t *macAddr, const uint8_t *data, int len);

/* Bridge Functions */
void init_bridge_communication(void);
void Bridge_callback(const uint8_t *macAddr, const uint8_t *data, int len);

#endif