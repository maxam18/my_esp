/* My ESP HTU2x sensor lib
 * File: me_htu2x.h
 * Started: Thu Feb 13 14:44:02 MSK 2025
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2025 ..."
 */

#ifndef _ME_HTU2X_H
#define _ME_HTU2X_H

#include <sdkconfig.h>
#include <esp_err.h>
#include <me_i2c.h>

#define ME_HTU2x_I2C_ADDR     0x40

typedef struct
{
    i2c_port_t      port;
    int8_t          addr;
} me_htu2x_config_t;

esp_err_t me_htu2x_init(me_htu2x_config_t *cfg);

esp_err_t me_htu2x_read_temp(me_htu2x_config_t *c, double *val);
esp_err_t me_htu2x_read_humid(me_htu2x_config_t *c, double *val);

#endif
