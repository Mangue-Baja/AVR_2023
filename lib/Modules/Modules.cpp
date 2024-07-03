#include "Modules.h"

av_packet_t msg_packet;
uint8_t sensor_flag = 0;
unsigned long curr = 0;
bool peer_registred = false;
bool interrupt = false;

void printAddress()
{
    // Put ESP32 into Station Mode
    WiFi.mode(WIFI_MODE_STA);
    Serial.printf("Mac address: ");
    Serial.println(WiFi.macAddress());

    // Disconnect from Wifi
    WiFi.disconnect();
}

/* ======================================== 30 METERS FUNCTIONS ========================================= */
void init_30meters_communication()
{
    pinMode(LED_BUILTIN, OUTPUT);
    printAddress();

    if (!init_esp_now())
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1500);
        esp_restart();
    }

    if (register_receive_callback(Callback_for_30meters))
        Serial.println("30 metros OK!");

    msg_packet.id = module_t::metros_30;
    attachInterrupt(digitalPinToInterrupt(SENSOR_30m), ISR_30m, FALLING);
    sensor_flag = 0;

    while (1)
    {
        switch (sensor_flag)
        {
        case 1:
            curr = millis();
            sensor_flag = 2;
            break;

        case 2:
            attachInterrupt(digitalPinToInterrupt(SENSOR_30m), ISR_30m, FALLING);

            while (!interrupt)
                msg_packet.time = millis() - curr;

            while (!interrupt)
                vTaskDelay(1);

            if (interrupt)
            {
                msg_packet.command_for_state_machine = state_machine_command_t::end_run_30m;
                msg_packet.time = msg_packet.time; // redundancia feita de proposito, vai que de erro magicamente
                sent_to_single(&msg_packet, sizeof(av_packet_t), msg_packet.mac_address);
                sensor_flag = 0;
                interrupt = false;
                msg_packet.command_for_state_machine = state_machine_command_t::do_nothing;
            }

            break;

        default:
            detachInterrupt(digitalPinToInterrupt(SENSOR_30m));
            vTaskDelay(50);
            break;
        }
    }
}

void ISR_30m()
{
    msg_packet.time = millis() - curr;

    interrupt = true;
    detachInterrupt(digitalPinToInterrupt(SENSOR_30m));
}

void Callback_for_30meters(const uint8_t *macAddr, const uint8_t *data, int len)
{
    av_packet_t recv;
    memcpy(&recv, (av_packet_t *)data, len);

    if (recv.id == module_t::metros_0)
    {
        if (recv.command_for_state_machine == state_machine_command_t::check_module)
        {
            if (!peer_registred)
            {
                if (register_peer(recv.mac_address))
                {
                    memcpy(msg_packet.mac_address, recv.mac_address, 6);
                    peer_registred = true;
                    Serial.println("Peer registred");
                }
            }
            msg_packet.command_for_state_machine = state_machine_command_t::flag_30m;
            sent_to_single(&recv, sizeof(av_packet_t), msg_packet.mac_address);
        }

        if (recv.command_for_state_machine == state_machine_command_t::start_run)
        {
            sensor_flag = 1;
        }

        if (recv.command_for_state_machine == state_machine_command_t::cancel)
        {
            sensor_flag = 0;
        }
    }
}

/* ======================================== 100 METERS FUNCTIONS ======================================== */
void init_100meters_communication()
{
    pinMode(LED_BUILTIN, OUTPUT);
    printAddress();

    if (!init_esp_now())
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1500);
        esp_restart();
    }

    if (register_receive_callback(Callback_for_100meters))
        Serial.println("100 metros OK!");

    msg_packet.id = module_t::metros_100;
    attachInterrupt(digitalPinToInterrupt(SENSOR_100m), ISR_100m, FALLING);
    pinMode(SENSOR_101m, INPUT);
    sensor_flag = 0;

    while (1)
    {
        switch (sensor_flag)
        {
        case 1:
            curr = millis();
            sensor_flag = 2;
            break;

        case 2:
            attachInterrupt(digitalPinToInterrupt(SENSOR_100m), ISR_100m, FALLING);

            while (!interrupt)
                msg_packet.time = millis() - curr;

            while (!interrupt)
                vTaskDelay(1);

            if (interrupt)
            {
                while (!digitalRead(SENSOR_101m))
                    msg_packet.timer2 = millis() - curr;
                if (digitalRead(SENSOR_101m))
                    msg_packet.timer2 = millis() - curr;

                msg_packet.command_for_state_machine = state_machine_command_t::end_run_100m;
                msg_packet.time = msg_packet.time;     // redundancia feita de proposito, vai que de erro magicamente
                msg_packet.timer2 = msg_packet.timer2; // redundancia feita de proposito, vai que de erro magicamente
                sent_to_single(&msg_packet, sizeof(av_packet_t), msg_packet.mac_address);
                sensor_flag = 0;
                interrupt = false;
                msg_packet.command_for_state_machine = state_machine_command_t::do_nothing;
            }

            break;

        default:
            detachInterrupt(digitalPinToInterrupt(SENSOR_30m));
            vTaskDelay(50);
            break;
        }
    }
}

void ISR_100m()
{
    msg_packet.time = millis() - curr;

    interrupt = true;
    detachInterrupt(digitalPinToInterrupt(SENSOR_100m));
}

void Callback_for_100meters(const uint8_t *macAddr, const uint8_t *data, int len)
{
    av_packet_t recv;
    memcpy(&recv, (av_packet_t *)data, len);

    if (recv.id == module_t::metros_0)
    {
        if (recv.command_for_state_machine == state_machine_command_t::check_module)
        {
            if (!peer_registred)
            {
                if (register_peer(recv.mac_address))
                {
                    memcpy(msg_packet.mac_address, recv.mac_address, 6);
                    peer_registred = true;
                    Serial.println("Peer registred");
                }
            }
            msg_packet.command_for_state_machine = state_machine_command_t::flag_100m;
            sent_to_single(&recv, sizeof(av_packet_t), msg_packet.mac_address);
        }

        if (recv.command_for_state_machine == state_machine_command_t::start_run)
        {
            sensor_flag = 1;
        }

        if (recv.command_for_state_machine == state_machine_command_t::cancel)
        {
            sensor_flag = 0;
        }
    }
}

/* ========================================== BRIDGE FUNCTIONS ========================================== */
void init_bridge_communication()
{
    pinMode(LED_BUILTIN, OUTPUT);
    printAddress();

    if (!init_esp_now())
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1500);
        esp_restart();
    }

    if (register_receive_callback(Bridge_callback))
        Serial.println("Bridge OK!");

    while (1)
    {
        vTaskDelay(100);
    }
}

void Bridge_callback(const uint8_t *macAddr, const uint8_t *data, int len)
{
    av_packet_t recv;
    memcpy(&recv, (av_packet_t *)data, len);

    if (sent_to_all(&recv, sizeof(av_packet_t)))
        Serial.println("Sucesso ao enviar a mensagem");
}
