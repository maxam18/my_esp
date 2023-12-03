/* My ESP GoodDisplay E-ink SPI functions
 * File: me_gd_spi.h
 * Started: Sun Oct  1 10:55:13 MSK 2023
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2023 ..."
 */

#ifndef _ME_GD_SPI_H
#define _ME_GD_SPI_H

#include "driver/spi_master.h"

#include <me_str.h>
#include <me_gd_model.h>

#define CLOCK_SPEED_HZ (4000000)

#define me_gd_dev_cmd_null      { 0x00, me_str_null }
typedef struct 
{
    uint8_t     cmd;
    me_str_t    data;
} me_gd_dev_cmd_t;

typedef struct 
{
    me_gd_dev_cmd_t     poweroff;
    me_gd_dev_cmd_t     sleep;
    me_gd_dev_cmd_t     reset;
    me_gd_dev_cmd_t     refresh;
    me_gd_dev_cmd_t     update;
    me_gd_dev_cmd_t    *init;
} me_gd_dev_cmd_set_t;

typedef struct
{
    uint8_t                 pin_sck;
    uint8_t                 pin_dta;
    uint8_t                 pin_cs;
    uint8_t                 pin_dc;
    uint8_t                 pin_rst;
    uint8_t                 pin_bsy;
    me_gd_model_enum_t      model;
} me_gd_dev_conf_t;

typedef struct 
{
    uint8_t                 pin_dc;
    uint8_t                 pin_busy;
    uint8_t                 pin_reset;
    uint8_t                 busy_level;

    spi_device_handle_t     spi_dev;
    me_gd_dev_cmd_set_t    *cmd_set;
} me_gd_dev_t;

extern me_gd_dev_cmd_set_t  me_gd_models_cmd_set[];

esp_err_t me_gd_dev_draw(me_gd_dev_t *dev, uint8_t *data, size_t len);
esp_err_t me_gd_dev_init(me_gd_dev_t *dev, me_gd_dev_conf_t *cf);
esp_err_t me_gd_dev_part(me_gd_dev_t *dev, int x_start, int y_start, int width, int height, uint8_t *data);

#endif

