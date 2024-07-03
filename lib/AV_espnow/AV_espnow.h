#ifndef AV_ESPNOW_H
#define AV_ESPNOW_H

#include <Arduino.h>
#include <esp_now.h>

typedef void (*esp_now_callback)(const uint8_t *, const uint8_t *, int);

bool init_esp_now();
bool register_peer(uint8_t* mac);
bool register_receive_callback(esp_now_callback receive_callback);

bool sent_to_all(void *AnyMessage, int size);
bool sent_to_single(void *AnyMessage, int size, uint8_t* address_from_receive);

#endif