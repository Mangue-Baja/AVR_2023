#include "AV_espnow.h"

bool init_esp_now()
{
    return esp_now_init() == ESP_OK ? true : false;
}

void register_receive_callback(esp_now_callback receive_callback)
{
    esp_now_register_recv_cb(receive_callback);
}

bool sent_to_all(void *AnyMessage, int size)
{
    // Broadcast a message to every device in range
    uint8_t BroadcastAdress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    esp_now_peer_info_t BroadInfo = {};

    memcpy(&BroadInfo.peer_addr, BroadcastAdress, 6);

    if(!esp_now_is_peer_exist(BroadcastAdress)) 
        esp_now_add_peer(&BroadInfo);

    // Send message
    esp_err_t result = esp_now_send(BroadcastAdress, (uint8_t *)AnyMessage, size);

    return result==ESP_OK ? true : false;
}
