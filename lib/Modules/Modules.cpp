#include "Modules.h"

/* ========================================== BRIDGE FUNCTIONS ========================================== */
void init_bridge_communication()
{
    pinMode(LED_BUILTIN, OUTPUT);
    // Put ESP32 into Station Mode
    WiFi.mode(WIFI_MODE_STA);
    Serial.printf("Mac address: ");
    Serial.println(WiFi.macAddress());

    // Disconnect from Wifi
    WiFi.disconnect();
    
    if (!init_esp_now())
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1500);
        esp_restart();
    }

    register_receive_callback(Bridge_callback);

    while (1)
    {
        vTaskDelay(100);
    }
}

void Bridge_callback(const uint8_t* macAddr, const uint8_t* data, int len)
{
    int teste;
    memcpy(&teste, (int*)data, len);

    if (sent_to_all(&teste, sizeof(teste)))
        Serial.println("Sucesso ao enviar a mensagem");
}
