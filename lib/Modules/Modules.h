#ifndef MODULES_H
#define MODULES_H

#include <WiFi.h>
#include "AV_espnow.h"

/* Bridge Functions */
void init_bridge_communication(void);
void Bridge_callback(const uint8_t* macAddr, const uint8_t* data, int len);

#endif