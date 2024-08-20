/* My ESP W2812 RGB LED strip controller based on RMT
 * File: me_w2812.c
 * Started: Mon Aug 19 22:38:32 MSK 2024
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2024 ..."
 */

#include "driver/rmt_common.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"
#include "me_w2812.h"

#ifndef CONFIG_ME_W2812_RESOLUTION
#define RMT_LED_STRIP_RESOLUTION_HZ     10000000
#else
#define RMT_LED_STRIP_RESOLUTION_HZ     CONFIG_ME_W2812_RESOLUTION
#endif

// reset code duration defaults to 50us
#define ME_W2812_RESET_TICKS    (RMT_LED_STRIP_RESOLUTION_HZ / 1000000 * 50 / 2)
// T0H=0.3us
#define ME_W2812_T0H_TICKS      (0.3 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000)
#define ME_W2812_T1L_TICKS      ME_W2812_T0H_TICKS
// T0L=0.9us
#define ME_W2812_T0L_TICKS      (0.9 * RMT_LED_STRIP_RESOLUTION_HZ / 1000000)
#define ME_W2812_T1H_TICKS      ME_W2812_T0L_TICKS


static size_t me_w2812_encode_rgb(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *data, size_t data_size, rmt_encode_state_t *rstate)
{
    me_w2812_encoder_t     *handle = __containerof(encoder, me_w2812_encoder_t, base_encoder);
    rmt_encoder_handle_t    bytes_encoder = handle->bytes_encoder;
    rmt_encoder_handle_t    copy_encoder  = handle->copy_encoder;
    rmt_encode_state_t      session_state = RMT_ENCODING_RESET;
    rmt_encode_state_t      state         = RMT_ENCODING_RESET;
    size_t                  encoded_syms  = 0;

    switch( handle->state )
    {
        case 0:
            encoded_syms += bytes_encoder->encode(bytes_encoder, channel
                                        , data, data_size, &session_state);
            
            if( session_state & RMT_ENCODING_COMPLETE )
                handle->state = 1;

            if( session_state & RMT_ENCODING_MEM_FULL )
                state |= RMT_ENCODING_MEM_FULL;

        break;
        case 1:
            encoded_syms += copy_encoder->encode(copy_encoder, channel
                                        , &handle->reset_code
                                        , sizeof(handle->reset_code)
                                        , &session_state);
            if( session_state & RMT_ENCODING_COMPLETE )
            {
                handle->state = RMT_ENCODING_RESET; // back to the initial encoding session
                state |= RMT_ENCODING_COMPLETE;
            }
            if( session_state & RMT_ENCODING_MEM_FULL )
                state |= RMT_ENCODING_MEM_FULL;
        break;
    }
    *rstate = state;
    return encoded_syms;
}


static esp_err_t me_w2812_encoder_delete(rmt_encoder_t *encoder)
{
    me_w2812_encoder_t     *handle = __containerof(encoder, me_w2812_encoder_t, base_encoder);

    rmt_del_encoder(handle->bytes_encoder);
    rmt_del_encoder(handle->copy_encoder);
    free(handle);

    return ESP_OK;
}


static esp_err_t me_w2812_encoder_reset(rmt_encoder_t *encoder)
{
    me_w2812_encoder_t     *handle = __containerof(encoder, me_w2812_encoder_t, base_encoder);

    rmt_encoder_reset(handle->bytes_encoder);
    rmt_encoder_reset(handle->copy_encoder);
    handle->state = RMT_ENCODING_RESET;

    return ESP_OK;
}


esp_err_t me_w2812_init(me_w2812_t *led)
{
    esp_err_t               err;
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
        .gpio_num = led->gpio_num,
        .mem_block_symbols = 64, // increase the block size can make the LED less flickering
        .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
        .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
    };
    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = {
            .level0    = 1,
            .duration0 = ME_W2812_T0H_TICKS, // T0H=0.3us
            .level1    = 0,
            .duration1 = ME_W2812_T0L_TICKS, // T0L=0.9us
        },
        .bit1 = {
            .level0    = 1,
            .duration0 = ME_W2812_T1H_TICKS, // T1H=0.9us
            .level1    = 0,
            .duration1 = ME_W2812_T1L_TICKS, // T1L=0.3us
        },
        .flags.msb_first = 1 // WS2812 transfer bit order: G7...G0R7...R0B7...B0
    };
    rmt_copy_encoder_config_t copy_encoder_config = {};

    led->encoder = calloc(1, sizeof(me_w2812_encoder_t));
    led->leds = calloc(1, sizeof(me_w2812_rgb_t)*led->num_leds);
    if( !led->leds || !led->encoder )
    {
        err = ESP_ERR_NO_MEM;
        goto err_init;
    }

    led->channel        = NULL;
    led->encoder->bytes_encoder  = NULL;
    led->encoder->copy_encoder   = NULL;

    err = rmt_new_tx_channel(&tx_chan_config, &led->channel);
    if( err != ESP_OK )
        goto err_init;

    led->encoder->base_encoder.encode = me_w2812_encode_rgb;
    led->encoder->base_encoder.del    = me_w2812_encoder_delete;
    led->encoder->base_encoder.reset  = me_w2812_encoder_reset;
    // different led strip might have its own timing requirements, following parameter is for WS2812
    err = rmt_new_bytes_encoder(&bytes_encoder_config, &led->encoder->bytes_encoder);
    if( err != ESP_OK )
        goto err_init;
    
    err = rmt_new_copy_encoder(&copy_encoder_config, &led->encoder->copy_encoder);
    if( err != ESP_OK )
        goto err_init;

    led->encoder->reset_code.level0    = 0;
    led->encoder->reset_code.duration0 = ME_W2812_RESET_TICKS;
    led->encoder->reset_code.level1    = 0;
    led->encoder->reset_code.duration1 = ME_W2812_RESET_TICKS;

    return ESP_OK;

err_init:

    if( led->encoder )
    {
        if (led->encoder->bytes_encoder)
            rmt_del_encoder(led->encoder->bytes_encoder);

        if (led->encoder->copy_encoder)
            rmt_del_encoder(led->encoder->copy_encoder);

        free(led->encoder);
    }
    if (led->leds)
        free(led->encoder);

    return err;
}


esp_err_t me_w2812_deinit(me_w2812_t *led)
{
    if( led->encoder )
        me_w2812_encoder_delete(&led->encoder->base_encoder);

    if (led->leds)
        free(led->leds);

    return ESP_OK;
}


esp_err_t me_w2812_update(me_w2812_t *led)
{
    esp_err_t               err;
    rmt_transmit_config_t   tx_config = {
        .loop_count = 0, // no transfer loop
    };

    err = rmt_enable(led->channel);
    if( err != ESP_OK )
        return err;

    err = rmt_transmit(led->channel, &led->encoder->base_encoder, (uint8_t *)led->leds
                        , sizeof(me_w2812_rgb_t)*led->num_leds, &tx_config);
    if( err != ESP_OK )
        return err;

    err = rmt_tx_wait_all_done(led->channel, portMAX_DELAY);

    err = rmt_disable(led->channel);
    if( err != ESP_OK )
        return err;

    return ESP_OK;
}


void me_w2812_set_led(me_w2812_t *led, uint8_t start, uint8_t count, uint8_t r, uint8_t g, uint8_t b)
{
    assert(led->leds);

    while( count-- )
    {
        if( start >= led->num_leds )
            break;

        led->leds[start].red   = r;
        led->leds[start].green = g;
        led->leds[start].blue  = b;

        start++;
    }
}


void me_w2812_set_all(me_w2812_t *led, uint8_t r, uint8_t g, uint8_t b)
{
    me_w2812_set_led(led, 0, led->num_leds, r, g, b);
}