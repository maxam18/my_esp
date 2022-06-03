/* ADS111x ADC
 * File: me_ads111x.c
 * Started: Tue Jun 15 12:28:23 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */

#include "me_ads111x.h"
#include "me_debug.h"


static double _FSR[] = { 6144, 4096, 2048, 1024, 512, 256, 256, 256 };


esp_err_t me_ads111x_init(me_ads111x_conf_t *c)
{
    esp_err_t       err;
    uint8_t         res[2];

    me_debug( "ads_init", "chip: %d, port: %d, addr: 0x%02X, set: 0x%02X"
                        , c->chip, c->port, c->addr, c->set);
/*
    uint8_t         req[3];
    if( (c->set & (~ME_ADS111x_CONF_MODE_MASK)) == ME_ADS111x_CONF_MODE_CONT )
    {
        me_debug( "ads", "Setting continous mode");
        // wake up and set continous mode 
        me_ads111x_set_conf(c, OS, SNGL);
        req[0] = ME_ADS111x_REG_CONF;
        req[1] = c->set >> 8;
        req[2] = c->set & 0xFF;

        err = me_i2c_write(c->port, c->addr, req, 3);
        if( err != ESP_OK )
            return err;
    }
*/
    c->reg = ME_ADS111x_REG_CONF;
    err = me_i2c_req_resp(c->port, c->addr, &c->reg, 1, res, 2);

    c->cur = (res[0] << 8) | res[1];

    me_debug( "ads", "Configuration. Status: '%s', Read: 0x%04X, Set: 0x%04X"
                 , esp_err_to_name(err), c->cur, c->set );

    return err;
}


esp_err_t me_ads111x_get_raw(me_ads111x_conf_t *c, int16_t *raw)
{
    esp_err_t   err;
    u_int8_t    req[3], res[2];

    if( c->set != c->cur )
    {
        me_debug( "ads", "Conf req 0x%04X, last 0x%04X", c->set, c->cur);

        req[0] = ME_ADS111x_REG_CONF;
        req[1] = c->set >> 8;
        req[2] = c->set & 0xFF;

        err = me_i2c_write(c->port, c->addr, req, 3);
        if( err != ESP_OK )
            return err;
        
        c->cur = c->set;
        c->reg = ME_ADS111x_REG_CONF;

        /* wait for about two measurements (SPS-16) */
        //vTaskDelay(1+2000/16/portTICK_PERIOD_MS);
        vTaskDelay(200/portTICK_PERIOD_MS);
    }

    if( c->reg == ME_ADS111x_REG_CONV )
        err = me_i2c_read(c->port, c->addr, res, 2);
    else
    {
        c->reg = ME_ADS111x_REG_CONV;
        err = me_i2c_req_resp(c->port, c->addr, &c->reg, 1, res, 2);
    }
    *raw = (res[0]<<8) | res[1];

    return err;
}


esp_err_t me_ads111x_get_raw_ads1115(me_ads111x_conf_t *c, int16_t *raw, uint16_t pga, uint16_t mux)
{
    c->set = (c->set & ME_ADS111x_CONF_PGA_MASK) | pga;
    c->set = (c->set & ME_ADS111x_CONF_MUX_MASK) | mux;
    return me_ads111x_get_raw(c, raw);
}


esp_err_t me_ads111x_get_value(me_ads111x_conf_t *c, double *value)
{
    esp_err_t   err;
    int16_t     raw;
    uint8_t     fsr;

    err = me_ads111x_get_raw(c, &raw);
    if( err != ESP_OK )
        return err;

    fsr = (c->cur >> 9) & 0x0007;

    *value = (double)raw * _FSR[fsr] / 32767.0;

    return ESP_OK;
}

esp_err_t me_ads111x_get_value_ads1115(me_ads111x_conf_t *c, double *value, uint16_t pga, uint16_t mux)
{
    esp_err_t   err;
    int16_t     raw;
    uint8_t     fsr;

    err = me_ads111x_get_raw_ads1115(c, &raw, pga, mux);
    if( err != ESP_OK )
        return err;

    fsr = (c->cur >> 9) & 0x0007;

    *value = (double)raw * _FSR[fsr] / 32767.0;

    return ESP_OK;
}
