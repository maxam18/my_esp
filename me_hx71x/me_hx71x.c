/* My ESP HX710B adc
 * File: me_hx71x.c
 * Started: Sat Mar 25 22:25:12 MSK 2023
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2023 ..."
 * 
 * https://www.electroschematics.com/pressure-sensor-guide/
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <esp_system.h>
#include <driver/gpio.h>

#include <me_hx71x.h>

#if CONFIG_IDF_TARGET_ESP32
#include <esp32/rom/ets_sys.h>
#elif CONFIG_IDF_TARGET_ESP32S2
#include <esp32s2/rom/ets_sys.h>
#elif CONFIG_IDF_TARGET_ESP32S3
#include <esp32s3/rom/ets_sys.h>
#elif CONFIG_IDF_TARGET_ESP32C3
#include <esp32c3/rom/ets_sys.h>
#else
#error "Target unknown"
#endif

#define me_hx71x_delay()          ets_delay_us(10)

/*
static void print_bits(int32_t val)
{
    int         i = 7, n;
    char        buf[7*4+6], *p;
    int32_t     bits = val;

    p = buf + 7*4+6 - 1;
    while( i-- )
    {
        n = 4;
        while( n-- )
        {
            *p-- = '0' + (bits & 1);
            bits >>= 1;
        }
        *p-- = ' ';
    }

    fprintf( stderr, "VAL: %d, bits: %.*s\n", val, (int)(7*4+6), buf);
}
*/

esp_err_t me_hx71x_read(me_hx71x_t *hx, int32_t *retval)
{
    int        i;
    int32_t    val;

    if( gpio_get_level(hx->dta) )
        return ESP_ERR_INVALID_STATE;

    i   = 25;
    val = 0;
    while( i-- )
    {
        gpio_set_level(hx->clk, 1);
        me_hx71x_delay();
        gpio_set_level(hx->clk, 0);

        val = (val << 1) | gpio_get_level(hx->dta);
    }

    val = (val >> 1);

    if( val >> 23 )
        val |= 0xFF000000;

    *retval = val;

    return ESP_OK;
}

esp_err_t me_hx71x_init(me_hx71x_t **hx, int pin_clk, int pin_dta)
{
    gpio_config_t   io_conf = {
        .mode           = GPIO_MODE_INPUT,
        .intr_type      = GPIO_INTR_DISABLE,
        .pull_down_en   = 0,
        .pull_up_en     = 0,
        .pin_bit_mask   = (1ULL<<pin_dta)
    };

    if( *hx == NULL )
    {
        if( (*hx = malloc(sizeof(me_hx71x_t))) == NULL)
            return ESP_ERR_NO_MEM;
    }

    if( gpio_config(&io_conf) != ESP_OK )
        return ESP_ERR_INVALID_ARG;
    
    io_conf.mode            = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask    = (1ULL<<pin_clk);

    if( gpio_config(&io_conf) != ESP_OK )
        return ESP_ERR_INVALID_ARG;

    (*hx)->dta = pin_dta;
    (*hx)->clk = pin_clk;

    return ESP_OK;
}
