/* My ESP multi encoder
 * File: me_ncoder.h
 * Started: Tue Mar 22 11:59:08 MSK 2022
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2022 ..."
 */

#ifndef _ME_NCODER_H
#define _ME_NCODER_H

#include <esp_err.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

typedef enum { 
      ME_NCODER_RELEASED, ME_NCODER_RELEASED_LONG
    , ME_NCODER_PRESSED, ME_NCODER_PRESSED_LONG
    , ME_NCODER_ROTATED_LEFT, ME_NCODER_ROTATED_RIGHT
} me_ncoder_state_t;

typedef struct {
    me_ncoder_state_t   state;
    uint8_t             button;
} me_ncoder_event_t;

esp_err_t me_ncoder_init(uint8_t *pins, uint8_t len, uint8_t pina, uint8_t pinb, QueueHandle_t *queue);

#endif
