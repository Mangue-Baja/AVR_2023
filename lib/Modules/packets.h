#ifndef PACKETS_H
#define PACKETS_H

#include <sys/_stdint.h>

typedef enum 
{
    metros_0 = 0,
    metros_30 = 1,
    metros_100 = 2,
    bridge = 3
} module_t;

typedef enum
{
    do_nothing = 0xff,
    check_module = 0x00,
    cancel = 0x03,
    start_run = 0x04,
    flag_30m = 0x05,
    end_run_30m = uint8_t(~0x05),
    flag_100m = 0x06,
    end_run_100m = uint8_t(~0x06)
} state_machine_command_t;

typedef struct 
{
    uint8_t id;
    uint8_t command_for_state_machine = state_machine_command_t::do_nothing;
    uint8_t mac_address[6];
    unsigned long time;
    unsigned long timer2 = 0;
} av_packet_t;

typedef enum
{
    wait = 0,
    __setup__ = 1,
    run = 2
} state_t;

typedef enum
{
    menu = 0,
    __start_run__ = 1,
    wait_to_start = 2,
    lcd_display = 3,
    end_run = 4,
    save_run = 5
} av_ecu_t;

#endif