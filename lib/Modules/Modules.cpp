#include "Modules.h"

Ticker enable_reset_flag;
av_packet_t msg_packet;
av_ecu_t av_ecu_flag = av_ecu_t::menu, run_flag = av_ecu_t::wait_to_start;
state_t sensor_flag = state_t::wait;
int8_t pot_sel = 0, old_pot = -1;
unsigned long curr = 0;
bool peer_registred = false;
bool interrupt = false, sd_device_init = false;
bool esp_now_ok = false, conf_30 = false, conf_100 = false;
bool sel_30 = false, sel_100 = false; // variaveis para parar o loop de cada sensor
bool reset_avr = true;
unsigned long time_30 = 0, time_100 = 0, time101 = 0;

void printAddress()
{
    // Put ESP32 into Station Mode
    WiFi.mode(WIFI_MODE_STA);
    Serial.printf("Mac address: ");
    Serial.println(WiFi.macAddress());

    // Disconnect from Wifi
    WiFi.disconnect();
}

int8_t potSelect(uint8_t pin)
{
  uint16_t read_val = analogRead(pin);
  long option = map(read_val, 0, 4095, 1, 0);
  return (int8_t)option;
}

/* ======================================== 0 METERS FUNCTIONS ========================================= */
void init_AVR_ECU_0meters_communication()
{
    bool _lcd_print = true;
    pinMode(LED_BUILTIN, OUTPUT);
    printAddress();

    /* Init the LCD module */
    init_lcd();

    if (!init_esp_now())
    {
        digitalWrite(LED_BUILTIN, HIGH);
        error_message();
        delay(500);
        esp_restart();
    }

    if (register_receive_callback(Callback_receiver_0meters) && register_transmitter_callback(Callback_transmitter_0meters))
        Serial.println("AV ECU OK!");
    else
        return;

    WiFi.macAddress(msg_packet.mac_address);
    msg_packet.command_for_state_machine = state_machine_command_t::check_module;
    do
    {
        sent_to_all(&msg_packet, sizeof(av_packet_t));
        delay(120);
    } while (!esp_now_ok || !conf_30 || !conf_100);

    ok_message();
    sd_device_init = init_sd((uint8_t)(SD_CS));
    SD_status(sd_device_init);

    msg_packet.id = module_t::metros_0;
    pinMode(B_SEL, INPUT_PULLUP);
    pinMode(B_CANCEL, INPUT_PULLUP);
    pinMode(SENSOR_0m, INPUT_PULLUP);
    av_ecu_flag = av_ecu_t::menu;
    run_flag = av_ecu_t::wait_to_start;

    while (1)
    {
        switch (av_ecu_flag)
        {
            case av_ecu_t::menu:
            {
                if (_lcd_print)
                {
                    intro_text();
                    _lcd_print = false;
                }

                if (!digitalRead(B_SEL))
                {
                    vTaskDelay(120);
                    while (!digitalRead(B_SEL))
                        vTaskDelay(1);
                    start_the_av_run(millis());
                    av_ecu_flag = av_ecu_t::__start_run__;
                }

                if (!digitalRead(B_CANCEL) && reset_avr)
                {
                    msg_packet.command_for_state_machine = state_machine_command_t::reset_;
                    sent_to_all(&msg_packet, sizeof(av_packet_t));
                    msg_packet.command_for_state_machine = state_machine_command_t::do_nothing;
                    esp_restart();
                }
                
                break;
            }

            case av_ecu_t::__start_run__:
            {
                switch (run_flag)
                {
                    case av_ecu_t::wait_to_start:
                    {
                        time101 = time_100 = time_30 = 0;
                        start_the_av_run();

                        if (!digitalRead(B_CANCEL))
                        {
                            msg_packet.command_for_state_machine = state_machine_command_t::cancel;
                            av_ecu_flag = av_ecu_t::menu;
                            msg_packet.command_for_state_machine = state_machine_command_t::do_nothing;
                            reset_avr = false;
                            _lcd_print = true;
                            enable_reset_flag.once(2.0f, enable_reset);
                        }

                        if (digitalRead(SENSOR_0m))
                        {
                            msg_packet.command_for_state_machine = state_machine_command_t::start_run;
                            sent_to_all(&msg_packet, sizeof(av_packet_t));
                            save_tcurr_time(millis());
                            run_flag = av_ecu_t::lcd_display;
                        }

                        break;
                    }

                    case av_ecu_t::lcd_display:
                    {
                        bool i = false;
                        while (!sel_30 && !i)
                        {
                            printRun(millis()/*30m*/, millis()/*100m*/);

                            if (!digitalRead(B_CANCEL))
                            {
                                msg_packet.command_for_state_machine = state_machine_command_t::cancel;
                                sent_to_all(&msg_packet, sizeof(av_packet_t));
                                msg_packet.command_for_state_machine = state_machine_command_t::do_nothing;
                                _lcd_print = true;
                                sel_100 = sel_30 = false;
                                old_pot = -1;
                                av_ecu_flag = av_ecu_t::menu;
                                run_flag = av_ecu_t::wait_to_start;
                                reset_avr = false;
                                _lcd_print = true;
                                enable_reset_flag.once(2.0f, enable_reset);
                                i = true;
                            }
                        }

                        if (!i)
                            save_t30(time_30);

                        while (!sel_100 && !i)
                        {
                            printRun(millis()/*100m*/);
                            
                            if (!digitalRead(B_CANCEL))
                            {
                                msg_packet.command_for_state_machine = state_machine_command_t::cancel;
                                sent_to_all(&msg_packet, sizeof(av_packet_t));
                                msg_packet.command_for_state_machine = state_machine_command_t::do_nothing;
                                _lcd_print = true;
                                sel_100 = sel_30 = false;
                                old_pot = -1;
                                av_ecu_flag = av_ecu_t::menu;
                                run_flag = av_ecu_t::wait_to_start;
                                reset_avr = false;
                                _lcd_print = true;
                                enable_reset_flag.once(2.0f, enable_reset);
                                i = true;
                            }
                        }

                        if (!i)
                        {
                            save_t100(time_100);
                            run_flag = av_ecu_t::end_run;
                        }

                        break;
                    }

                    case av_ecu_t::end_run:
                    {
                        save_speed(time_100, time101);

                        if (!digitalRead(B_SEL))
                        {
                            vTaskDelay(120);
                            while(!digitalRead(B_SEL))
                                vTaskDelay(1);
                            run_flag = av_ecu_t::save_run;
                            vTaskDelay(100);
                        }

                        if (!digitalRead(B_CANCEL))
                        {
                            _lcd_print = true;
                            sel_100 = sel_30 = false;
                            old_pot = -1;
                            av_ecu_flag = av_ecu_t::menu;
                            run_flag = av_ecu_t::wait_to_start;
                            msg_packet.command_for_state_machine = state_machine_command_t::cancel;
                            sent_to_all(&msg_packet, sizeof(av_packet_t));
                            reset_avr = false;
                            _lcd_print = true;
                            enable_reset_flag.once(2.0f, enable_reset);                               
                        }

                        break;
                    }

                    case av_ecu_t::save_run:
                    {
                        if (sd_device_init)
                        {
                            pot_sel = potSelect(POT);

                            if (pot_sel != old_pot)
                            {
                                select_sd(pot_sel);
                                old_pot = pot_sel;   
                            }
                            
                            if (!digitalRead(B_SEL))
                            {
                                if (pot_sel == 0)
                                    sd_save_text(save_AV_Data);
                                else
                                    sd_save_text();

                                _lcd_print = true;
                                sel_100 = sel_30 = false;
                                old_pot = -1;
                                av_ecu_flag = av_ecu_t::menu;
                                run_flag = av_ecu_t::wait_to_start;
                                msg_packet.command_for_state_machine = state_machine_command_t::cancel;
                                sent_to_all(&msg_packet, sizeof(av_packet_t));
                            }

                            if (!digitalRead(B_CANCEL))
                            {
                                _lcd_print = true;
                                sel_100 = sel_30 = false;
                                old_pot = -1;
                                av_ecu_flag = av_ecu_t::menu;
                                run_flag = av_ecu_t::wait_to_start;
                                msg_packet.command_for_state_machine = state_machine_command_t::cancel;
                                sent_to_all(&msg_packet, sizeof(av_packet_t));                               
                            }
                        }

                        else
                        {
                            SD_status(sd_device_init);
                            _lcd_print = true;
                            sel_100 = sel_30 = false;
                            old_pot = -1;
                            av_ecu_flag = av_ecu_t::menu;
                            run_flag = av_ecu_t::wait_to_start;
                            msg_packet.command_for_state_machine = state_machine_command_t::cancel;
                            sent_to_all(&msg_packet, sizeof(av_packet_t));
                        }

                        break;
                    }
                }
            }
        }
    }
}

void enable_reset()
{
    reset_avr = true;
}

void Callback_transmitter_0meters(const uint8_t *macAddr, esp_now_send_status_t status)
{
    Serial.print("Last packet send status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sucess" : "Failed");

    esp_now_ok = status == ESP_NOW_SEND_SUCCESS;
}

void Callback_receiver_0meters(const uint8_t *macAddr, const uint8_t *data, int len)
{
    av_packet_t recv;
    memcpy(&recv, (av_packet_t*)data, len);    

    if (recv.id == module_t::metros_30)
    {
        if (recv.command_for_state_machine == state_machine_command_t::flag_30m)
            conf_30 = true;
        if (recv.command_for_state_machine == state_machine_command_t::flag_100m)
            conf_100 = true;
        if (recv.command_for_state_machine == state_machine_command_t::end_run_30m)
        {
            sel_30 = true;
            time_30 = recv.time;
        }
        
        if (recv.command_for_state_machine == state_machine_command_t::end_run_100m)
        {
            sel_100 = true;
            time_100 = recv.time;
            time101 = recv.timer2;
        }
    }
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
    else
        return;

    msg_packet.id = module_t::metros_30;
    attachInterrupt(digitalPinToInterrupt(SENSOR_30m), ISR_30m, FALLING);
    sensor_flag = state_t::wait;

    while (1)
    {
        switch (sensor_flag)
        {
        case state_t::__setup__:
            curr = millis();
            sensor_flag = state_t::run;
            break;

        case state_t::run:
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
                sensor_flag = state_t::wait;
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
    memcpy(&recv, (av_packet_t*)data, len);

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
            sent_to_single(&msg_packet, sizeof(av_packet_t), msg_packet.mac_address);
            recv.id = module_t::metros_30;
            sent_to_all(&recv, sizeof(av_packet_t)); // send to 100m
        }

        if (recv.command_for_state_machine == state_machine_command_t::start_run)
        {
            recv.id = module_t::metros_30;
            sent_to_all(&recv, sizeof(av_packet_t));
            sensor_flag = state_t::__setup__;
        }

        if (recv.command_for_state_machine == state_machine_command_t::cancel)
        {
            recv.id = module_t::metros_30;
            sent_to_all(&recv, sizeof(av_packet_t));
            sensor_flag = state_t::wait;
        }

        if (recv.command_for_state_machine == state_machine_command_t::reset_)
        {
            recv.id = module_t::metros_30;
            sent_to_all(&recv, sizeof(av_packet_t));
            sensor_flag = state_t::wait;
            esp_restart();
        }
    }

    if (recv.id == module_t::metros_100)
    {
        recv.id = module_t::metros_30;
        sent_to_single(&recv, sizeof(av_packet_t), msg_packet.mac_address);
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
    else
        return;

    msg_packet.id = module_t::metros_100;
    attachInterrupt(digitalPinToInterrupt(SENSOR_100m), ISR_100m, FALLING);
    pinMode(SENSOR_101m, INPUT_PULLUP);
    sensor_flag = state_t::wait;

    while (1)
    {
        switch (sensor_flag)
        {
        case state_t::__setup__:
            curr = millis();
            sensor_flag = state_t::run;
            break;

        case state_t::run:
            attachInterrupt(digitalPinToInterrupt(SENSOR_100m), ISR_100m, FALLING);

            while (!interrupt)
                msg_packet.time = millis() - curr;

            while (!interrupt)
                vTaskDelay(1);

            if (interrupt)
            {
                while (digitalRead(SENSOR_101m))
                    msg_packet.timer2 = millis() - curr;
                if (!digitalRead(SENSOR_101m))
                    msg_packet.timer2 = millis() - curr;

                msg_packet.command_for_state_machine = state_machine_command_t::end_run_100m;
                msg_packet.time = msg_packet.time;     // redundancia feita de proposito, vai que de erro magicamente
                msg_packet.timer2 = msg_packet.timer2; // redundancia feita de proposito, vai que de erro magicamente
                sent_to_all(&msg_packet, sizeof(av_packet_t));
                sensor_flag = state_t::wait;
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
    memcpy(&recv, (av_packet_t*)data, len);

    if (recv.id == module_t::metros_30)
    {
        if (recv.command_for_state_machine == state_machine_command_t::check_module)
        {
            msg_packet.command_for_state_machine = state_machine_command_t::flag_100m;
            sent_to_all(&msg_packet, sizeof(av_packet_t));
        }

        if (recv.command_for_state_machine == state_machine_command_t::start_run)
            sensor_flag = state_t::__setup__;

        if (recv.command_for_state_machine == state_machine_command_t::cancel)
            sensor_flag = state_t::wait;
        if (recv.command_for_state_machine == state_machine_command_t::reset_)
        {
            sensor_flag = state_t::wait;
            esp_restart();
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
    else
        return;

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
