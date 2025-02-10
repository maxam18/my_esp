/* My ESP W2812 RGB LED strip controller based on RMT
 * File: me_w2812.h
 * Started: Mon Aug 19 22:38:39 MSK 2024
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2024 ..."
 */

#ifndef _ME_W2812_H
#define _ME_W2812_H

#include <esp_err.h>
#include <stdint.h>
#include "driver/rmt_encoder.h"

typedef struct me_w2812_encoder_s {
    rmt_encoder_t           base_encoder;
    rmt_encoder_t          *bytes_encoder;
    rmt_encoder_t          *copy_encoder;
    int                     state;
    rmt_symbol_word_t       reset_code;
} me_w2812_encoder_t;

typedef struct me_w2812_rgb_s
{
    uint8_t             green;
    uint8_t             red;
    uint8_t             blue;
} me_w2812_rgb_t;

typedef struct me_w2812_s
{
    uint8_t                 num_leds;
    gpio_num_t              gpio_num;

    me_w2812_rgb_t         *leds;
    me_w2812_encoder_t     *encoder;
    rmt_channel_handle_t    channel;
} me_w2812_t;

esp_err_t me_w2812_init(me_w2812_t *led);
esp_err_t me_w2812_update(me_w2812_t *led);
esp_err_t me_w2812_deinit(me_w2812_t *led);

void me_w2812_set_led(me_w2812_t *led, uint8_t start, uint8_t count, uint8_t r, uint8_t g, uint8_t b);
void me_w2812_set_all(me_w2812_t *led, uint8_t r, uint8_t g, uint8_t b);
void me_w2812_set_vlevel(me_w2812_t *led, int level, int max, uint8_t r, uint8_t g, uint8_t b);

#endif
