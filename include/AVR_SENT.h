#ifndef AVR_SENT_H
#define AVR_SENT_H

#include <Arduino.h>
#include <esp_now.h>

class AVR_SENT
{
    public:
        AVR_SENT(const uint8_t *Address);
        ~AVR_SENT();
        void formatMacAddress(const uint8_t* macAddr, char* info, int maxLength);
        int Register_Peer(esp_now_peer_info Info);
        bool Sent_Data(const uint8_t msg);
        bool Sent_Data(const uint8_t msg, esp_now_peer_info Info, bool address = false);
    private:
        uint8_t Address_HOST[6];
    protected:
        uint8_t BroadcastAdress[6];
};

AVR_SENT::AVR_SENT(const uint8_t *Address)
{
    memcpy(&this->Address_HOST, (uint8_t *)&Address, sizeof(Address));

    // Broadcast a message to every device in range
    memset(&BroadcastAdress, 0xff, sizeof(BroadcastAdress));
}

AVR_SENT::~AVR_SENT() {/**/}

int AVR_SENT::Register_Peer(esp_now_peer_info Info)
{
    // Register peer
    memcpy(Info.peer_addr, Address_HOST, 6);
    Info.channel = 0;
    Info.encrypt = false;

    // Add peer
    if(esp_now_add_peer(&Info)!=ESP_OK)
    {
        Serial.println("Failed to add peer");

        return -1;
    }

    return 0;
}

bool AVR_SENT::Sent_Data(const uint8_t msg)
{
    esp_now_peer_info_t BroadInfo = {};
    memcpy(&BroadInfo.peer_addr, BroadcastAdress, 6);

    if(!esp_now_is_peer_exist(BroadcastAdress)) { esp_now_add_peer(&BroadInfo); }

    return Sent_Data(msg, BroadInfo);
}

bool AVR_SENT::Sent_Data(const uint8_t msg, esp_now_peer_info Info, bool address)
{
    esp_err_t result = esp_now_send(address ? Address_HOST : BroadcastAdress, (uint8_t *)&msg, sizeof(msg));

    return result==ESP_OK ? true : false;
}

void AVR_SENT::formatMacAddress(const uint8_t* macAddr, char* info, int maxLength)
{
    // Formats MAC Address
    snprintf(info, maxLength, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}


#endif