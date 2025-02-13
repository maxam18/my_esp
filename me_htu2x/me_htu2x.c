/* My ESP HTU2x sensor lib
 * File: me_htu2x.c
 * Started: Thu Feb 13 14:43:56 MSK 2025
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2025 ..."
 */

#include <esp_system.h>
#include <freertos/FreeRTOS.h>

#include <me_htu2x.h>
#include <stdint.h>

/* SYNC - HOLD SCK line while preparing data */
#define HTU2x_REG_TEMP_SYNC    0xE3
#define HTU2x_REG_HUMID_SYNC   0xE5
/* SYNC - NO HOLD of SCK line while preparing  */
#define HTU2x_REG_TEMP_ASYNC   0xF3
#define HTU2x_REG_HUMID_ASYNC  0xF5

#define HTU2x_REG_USER_READ    0xE7
#define HTU2x_REG_USER_WRITE   0xE6
#define HTU2x_REG_SOFT_RESET   0xFE

#define HTU2x_USER_HIGHRES     0x02 /* 14 bits and OTP disable */

esp_err_t me_htu2x_read(me_htu2x_config_t *c, double *temp, double *relh)
{
    esp_err_t   err;
	uint8_t     buf[4];
    int32_t     i32;

    buf[0] = HTU2x_REG_TEMP_ASYNC;
    err = me_i2c_write(c->port, c->addr, buf, 1);
    if( err != ESP_OK )
        return err;

    vTaskDelay(pdMS_TO_TICKS(100));

    err = me_i2c_read(c->port, c->addr, buf, 3);
    if( err != ESP_OK )
    {
        return err;
    }

	i32   = (buf [0] << 8 | buf [1]) & 0xFFFC;
    *temp = -46.85 + (175.72 * ((double)i32/65535.0));

    buf[0] = HTU2x_REG_HUMID_ASYNC;
    err = me_i2c_write(c->port, c->addr, buf, 1);;
    if( err != ESP_OK )
        return err;

    vTaskDelay(pdMS_TO_TICKS(100));

    err = me_i2c_read(c->port, c->addr, buf, 3);
    if( err != ESP_OK )
        return err;

    i32   = (buf [0] << 8 | buf [1]) & 0xFFFC;
	*relh = -6.0 + (125.0 * ((double)i32/65535.0));

    return err;
}


esp_err_t me_htu2x_init(me_htu2x_config_t *c)
{
    esp_err_t   err;
    uint8_t     buf[2];

    err = me_i2c_ping(c->port, c->addr);
    if( err != ESP_OK )
        return err;

/* device soft reset */
    buf[0] = HTU2x_REG_SOFT_RESET;
    err = me_i2c_write(c->port, c->addr, buf, 1);
    if( err != ESP_OK )
        return err;

    vTaskDelay(pdMS_TO_TICKS(100));

    buf[0] = HTU2x_REG_USER_WRITE;
    buf[1] = HTU2x_USER_HIGHRES;
    err = me_i2c_write(c->port, c->addr, buf, 2);

    return err;
}

