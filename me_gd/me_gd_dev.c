/* My ESP GoodDisplay E-ink SPI functions
 * File: me_gd_spi.c
 * Started: Sun Oct  1 10:55:19 MSK 2023
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2023 ..."
 */

#include "include/me_gd_dev.h"
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "hal/spi_types.h"

#include <esp_err.h>

#include <me_debug.h>
#include <me_gd_dev.h>

// HSPI_HOST is only defined for ESP32 and ESP32-S3
#ifndef HSPI_HOST
#define HSPI_HOST SPI2_HOST
#endif

#ifdef CONFIG_ME_GD_DEBUG/*{{{*/
void debug_cmd(me_gd_dev_cmd_t *cmd)
{
    char    buf[128], *p;
    u_char *c;
    uint8_t n, len;

    p  = buf;
    n  = sprintf(p, "CMD: 0x%02X", cmd->cmd);
    p += n;
    len = cmd->data.len;
    if( len )
    {
        n  = sprintf(p, "; DATA:");
        p += n;
        c  = cmd->data.data;
        while( len-- )
        {
            n  = sprintf(p, " 0x%02X", *c++);
            p += n;
            if( p - buf > 80 )
            {
                n  = sprintf(p, "... %d", len);
                p += n;
                break;
            }
        }
    }
    me_debug("GDDEV", "%.*s", p - buf, buf);
}
#else
#define debug_cmd(X)
#endif/*}}}*/

static esp_err_t dev_send(spi_device_handle_t spi_dev, uint8_t *data, size_t len);
static esp_err_t dev_wait_busy(me_gd_dev_t *dev);
static esp_err_t dev_wakeup(me_gd_dev_t *dev, me_gd_dev_cmd_t *cmd);
static esp_err_t dev_sleep(me_gd_dev_t *dev);
static esp_err_t dev_draw(me_gd_dev_t *dev, uint8_t *data, size_t len, char is_fast);
static esp_err_t dev_reset(me_gd_dev_t *dev);
static esp_err_t dev_write(me_gd_dev_t *dev, me_gd_dev_cmd_t *cmd);

#if CONFIG_DEV_CLEAR /* {{{ */
static esp_err_t set_mem(me_gd_dev_t *dev, int bitmap_len)
{
    esp_err_t       err;
    me_gd_dev_cmd_t init_ram = me_gd_dev_cmd_null;

    init_ram.data.len =  bitmap_len;
    init_ram.data.data = malloc(init_ram.data.len);
    memset(init_ram.data.data, 0xFF, init_ram.data.len);

    init_ram.cmd = 0x24;
    err = dev_write(dev, &init_ram);
    if( err == ESP_OK )
    {
        init_ram.cmd = 0x26;
        err = dev_write(dev, &init_ram);
    }

    free(init_ram.data.data);

    return err;
}


static esp_err_t dev_clear(me_gd_dev_t *dev, int bitmap_len)
{
    esp_err_t       err;

    err = dev_wakeup(dev, dev->cmd_set->init);
    if( err == ESP_OK )
        return err;
    
    err = set_mem(dev, bitmap_len);
    if( err == ESP_OK )
        return err;

    err = dev_write(dev, &dev->cmd_set->refresh);
    if( err != ESP_OK )
        return err;
    
    err = dev_sleep(dev);

    return err;
}
#else
#define dev_clear(A,B)  err
#endif /* }}} */


static esp_err_t dev_send(spi_device_handle_t spi_dev, uint8_t *data, size_t len)/*{{{*/
{
    spi_transaction_t   t;

    memset(&t, 0, sizeof(t));
    t.length = len*8;
    t.tx_buffer = data;

    return spi_device_polling_transmit(spi_dev, &t);
}/*}}}*/


static esp_err_t dev_wait_busy(me_gd_dev_t *dev)/*{{{*/
{
    int loop = 1000;

    while( loop-- )
        if( gpio_get_level(dev->pin_busy) == dev->busy_level )
            vTaskDelay(pdMS_TO_TICKS(20));
        else
            break;

    return (loop < 0) ? ESP_FAIL : ESP_OK;
}/*}}}*/


static esp_err_t dev_wakeup(me_gd_dev_t *dev, me_gd_dev_cmd_t *cmd)/*{{{*/
{
    esp_err_t   err;

    err = dev_reset(dev);
    if( err != ESP_OK )
        return err;

    while( cmd->cmd )
    {
        err = dev_write(dev, cmd);

        if( cmd->data.len == 0 && err == ESP_OK )
            err = dev_wait_busy(dev);

        if( err != ESP_OK )
            break;
        cmd++;
    }

    return err;
}/*}}}*/


static esp_err_t dev_sleep(me_gd_dev_t *dev)/*{{{*/
{
    esp_err_t   err;

    if( dev->cmd_set->poweroff.data.data )
    {
        err = dev_write(dev, &dev->cmd_set->poweroff);
        if( err == ESP_OK )
            err = dev_wait_busy(dev);
    }
    err = dev_write(dev, &dev->cmd_set->sleep);

    vTaskDelay(pdMS_TO_TICKS(100));
    return err;
}/*}}}*/


static esp_err_t dev_draw(me_gd_dev_t *dev, uint8_t *data, size_t len, char is_fast) /* {{{ */
{
    esp_err_t           err;
    me_gd_dev_cmd_t     cmd = { dev->cmd_set->update.cmd, { data, len }};
    me_gd_dev_cmd_t    *init = is_fast ? dev->cmd_set->init_fast : dev->cmd_set->init;

    if( init )
    {
        err = dev_wakeup(dev, init);
        if( err != ESP_OK )
            return err;
    }

    err = dev_write(dev, &cmd);
    if( err != ESP_OK )
        return err;

    err = dev_write(dev, &dev->cmd_set->refresh);
    if( err != ESP_OK )
        return err;

    err = dev_sleep(dev);

    return err;
} /* }}} */


static esp_err_t dev_reset(me_gd_dev_t *dev)/*{{{*/
{
    vTaskDelay(pdMS_TO_TICKS(20));//At least 20ms delay
    gpio_set_level(dev->pin_reset, 0); // Module reset
    vTaskDelay(pdMS_TO_TICKS(40));//At least 40ms delay 
    gpio_set_level(dev->pin_reset, 1);
    vTaskDelay(pdMS_TO_TICKS(50));//At least 50ms delay 

    if( dev->cmd_set->reset.cmd )
    {
        dev_wait_busy(dev);
        dev_write(dev, &dev->cmd_set->reset);
    }

    return dev_wait_busy(dev);
}/*}}}*/


static esp_err_t dev_write(me_gd_dev_t *dev, me_gd_dev_cmd_t *cmd)/*{{{*/
{
    esp_err_t   err;

    debug_cmd(cmd);
	gpio_set_level(dev->pin_dc, 0);  // D/C#   0:command  1:data  
    err = dev_send(dev->spi_dev, &cmd->cmd, 1);

    if( err == ESP_OK && cmd->data.data )
    {
	    gpio_set_level(dev->pin_dc, 1);  // D/C#   0:command  1:data
        err = dev_send(dev->spi_dev, cmd->data.data, cmd->data.len);
    }

    return err;
}/*}}}*/


esp_err_t me_gd_dev_draw(me_gd_dev_t *dev, uint8_t *data, size_t len)/*{{{*/
{
    return dev_draw(dev, data, len, 0);
}/*}}}*/


esp_err_t me_gd_dev_draw_fast(me_gd_dev_t *dev, uint8_t *data, size_t len)/*{{{*/
{
    return dev_draw(dev, data, len, 1);
}/*}}}*/


esp_err_t me_gd_dev_init(me_gd_dev_t *dev, me_gd_dev_conf_t *cf)/*{{{*/
{
    esp_err_t                       err         = ESP_OK;
    spi_bus_config_t                spi_cfg     = {
       .mosi_io_num     = cf->pin_dta,
       .miso_io_num     = -1,
       .sclk_io_num     = cf->pin_sck,
       .quadwp_io_num   = -1,
       .quadhd_io_num   = -1,
       .max_transfer_sz = 0,
       .flags           = 0
    };
    spi_device_interface_config_t   spi_dev_cfg = {
        .spics_io_num   = cf->pin_cs,
        .clock_speed_hz = CLOCK_SPEED_HZ,
        .mode           = 0,
        .queue_size     = 1,
        .flags          = SPI_DEVICE_NO_DUMMY
    };
    gpio_config_t io_in_conf = {
        .mode           = GPIO_MODE_INPUT,
        .intr_type      = GPIO_INTR_DISABLE,
        .pull_down_en   = 0,
        .pull_up_en     = 0,
        .pin_bit_mask   = (1ULL << cf->pin_bsy)
    };
    gpio_config_t io_out_conf = {
        .mode           = GPIO_MODE_OUTPUT,
        .intr_type      = GPIO_INTR_DISABLE,
        .pull_down_en   = 0,
        .pull_up_en     = 0,
        .pin_bit_mask   = (1ULL << cf->pin_rst) | (1ULL << cf->pin_dc) | (1ULL << cf->pin_cs)
    };

    err = gpio_config(&io_in_conf);
    if( err != ESP_OK )
        goto init_err;

    err = gpio_config(&io_out_conf);
    if( err != ESP_OK )
        goto init_err;

    spi_cfg.max_transfer_sz = 4 + (me_gd_models[cf->model].height
                                    * me_gd_models[cf->model].width
                                    * me_gd_models[cf->model].bpp) / 8;
    if( spi_cfg.max_transfer_sz < 4096 )
        spi_cfg.max_transfer_sz = 0;

    err = spi_bus_initialize(HSPI_HOST, &spi_cfg, 1);
    if( err != ESP_OK )
        goto init_err;

    err = spi_bus_add_device(HSPI_HOST, &spi_dev_cfg, &dev->spi_dev);
    if( err != ESP_OK )
        goto init_err;

    dev->busy_level = me_gd_models[cf->model].busy_level;
    dev->pin_busy   = cf->pin_bsy;
    dev->pin_dc     = cf->pin_dc;
    dev->pin_reset  = cf->pin_rst;
    dev->cmd_set    = (me_gd_models_cmd_set+cf->model);

    err = dev_clear(dev, ((me_gd_models[cf->model].height
                        * me_gd_models[cf->model].width
                        * me_gd_models[cf->model].bpp) / 8));

init_err:
    return err;
}/*}}}*/


