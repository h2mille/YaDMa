/*
*
* Copyright (C) 2018, Hugo van Santen
* All rights reserved.
*
* Please see the LICENSE file for more information.
*
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ESP8266WiFi.h>

#ifndef A116_H
#define A116_H

class a116{
  public:
    static void servo_write(uint8_t id,uint8_t  cmd,uint8_t packet_size, uint8_t* data);
    static void servo_read_ram(uint8_t id, uint8_t address,uint8_t size,uint8_t* output);
    static void servo_write_ram(uint8_t id, uint8_t address,uint8_t size,uint8_t* input);
    static void set_color(uint8_t id, bool white, bool blue, bool green, bool red);
    static void servo_reboot(uint8_t id);
    static uint16_t get_position(uint8_t id);
    static uint8_t get_state(uint8_t id);
    static void servo_move(uint8_t id,uint16_t angle);
  private:
};

#endif

