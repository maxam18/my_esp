/* SH1106 OLED
 * File: me_sh1106.c
 * Started: Sun 17 Nov 2019 01:17:48 PM MSK
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2019 ..."
 */

#include <string.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/task.h"

#include "sdkconfig.h" // generated by "make menuconfig"


#include "me_sh1106.h"
#include "me_sh1106_defs.h"
#include "font8x8_basic.h"

esp_err_t me_sh1106_init(me_sh1106_conf_t *c)
{
    uint8_t     req[] = {
                          ME_SH1106_CMD_STREAM
                        , ME_SH1106_CMD_DISPLAY_OFF
                        , ME_SH1106_CMD_SET_CHARGE_PUMP_CTRL
                        , ME_SH1106_CMD_SET_CHARGE_PUMP_ON
                        , ME_SH1106_CMD_SET_SEGMENT_REMAP_OFF
                        , ME_SH1106_CMD_SET_COM_SCAN_MODE
                        , ME_SH1106_CMD_DISPLAY_ON
                        , 0x00, 0x10, 0xB0
                        , ME_SH1106_CMD_SET_DISPLAY_START_LINE
                        , ME_SH1106_CMD_SET_DISPLAY_OFFSET, 0x00
                    };

    if( c->flip == ME_SH1106_FLIP )
        req[4] = ME_SH1106_CMD_SET_SEGMENT_REMAP_ON;

    return me_i2c_write(c->port, c->addr, req, sizeof(req));
}

esp_err_t me_sh1106_display_clear(me_sh1106_conf_t *c)
{
    esp_err_t   err;
	uint8_t     req[3+132], i;

    bzero(req, sizeof(req));
    req[0] = ME_SH1106_CMD_SINGLE;
    req[2] = ME_SH1106_REG_DATA_STREAM;

	for( i = 0; i < 8; i++ )
    {
        req[1] = 0xB0 | i;
        err = me_i2c_write(c->port, c->addr, req, sizeof(req));
        if( err != ESP_OK )
            break;
	}

    return err;
}


esp_err_t me_sh1106_contrast(me_sh1106_conf_t *c, int val)
{
	uint8_t     req[] = {
                          ME_SH1106_CMD_STREAM
                        , ME_SH1106_CMD_SET_CONTRAST
                        , 0
                    };

    req[2] = val & 0xFF;

    return me_i2c_write(c->port, c->addr, req, sizeof(req));
}

static esp_err_t reset_row(me_sh1106_conf_t *c, uint8_t row)
{
    uint8_t     req[] = { ME_SH1106_CMD_STREAM
                        , 0x08, 0x10, 0xB0 
                    };

    if( row > ME_SH1106_MAX_ROWS )
        return ESP_ERR_INVALID_ARG;

    req[3] |= row;

    return me_i2c_write(c->port, c->addr, req, sizeof(req));
}

static esp_err_t me_sh1106_8x8_row(me_sh1106_conf_t *c, char *str, int row)
{
    esp_err_t   err = ESP_OK;
    uint8_t     req[1+132], *rp, *end, ch;

    if( c->flip )
        row = (ME_SH1106_MAX_ROWS - row) - 1;

    err = reset_row(c, row);
    if( err != ESP_OK )
        return err;
    
    end   = req + sizeof(req);
    rp    = req;
    *rp++ = ME_SH1106_REG_DATA_STREAM;
    
	while( rp < end )
    {
        ch = (uint8_t)*str;
        if( !ch )
            ch = ' ';
        else
            str++;

        memcpy(rp, font8x8_basic_tr[ch], 8);

        rp += 8;
    }

    return me_i2c_write(c->port, c->addr, req, rp - req);
}


esp_err_t me_sh1106_8x8_string(me_sh1106_conf_t *c, char *str, int row)
{
    return me_sh1106_8x8_row(c, str, row);
}

