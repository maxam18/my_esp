/* My ESP MBP180 driver
 * File: me_bmp180.c
 * Started: Sun Apr  2 12:57:09 MSK 2023
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2023 ..."
 */

#include <stdio.h>
#include <me_bmp180.h>

#define BMP180_CHIP_ID				0x55

#define BMP180_REG_ID				0xD0
#define BMP180_REG_RESET			0xE0
#define BMP180_REG_CALIB			0xAA
#define BMP180_REG_CTRL 			0xF4
#define BMP180_REG_MEAS 			0xF6

#define BMP180_CMD_RESET			0xB6
#define BMP180_CMD_MEAS	    	    0x20
#define BMP180_CMD_MEAS_TEMP	    (BMP180_CMD_MEAS | 0x0E)
#define BMP180_CMD_MEAS_PRESS	    (BMP180_CMD_MEAS | 0x14 | (ME_BMP180_OVERSAMPLING<<6))

typedef struct 
{
    int16_t    temperature;
    int32_t    pressure;
} bmp180_uc_t;

esp_err_t bmp180_meas_read(me_bmp180_conf_t *conf, bmp180_uc_t *uc)
{
    uint8_t     req[2];
    uint8_t     res[3];
    esp_err_t   err;
    
    // Command to measure temperature
    req[0] = BMP180_REG_CTRL;
    req[1] = BMP180_CMD_MEAS_TEMP;
	err    = me_i2c_write(conf->port, conf->addr, req, 2);
    if( err != ESP_OK )
		return err;

    vTaskDelay(2*pdMS_TO_TICKS(5));

    req[0] = BMP180_REG_MEAS;
	err  = me_i2c_req_resp(conf->port, conf->addr, req, 1, res, 2);
    if( err != ESP_OK )
        return err;
    
    uc->temperature = (res[0] << 8) | res[1];
    
    // Command to measure pressure
    req[0] = BMP180_REG_CTRL;
    req[1] = BMP180_CMD_MEAS_PRESS;
	err    = me_i2c_write(conf->port, conf->addr, req, 2);
    if( err != ESP_OK )
		return err;

    vTaskDelay(2*pdMS_TO_TICKS(5+5*ME_BMP180_OVERSAMPLING)); // max wait time

    req[0] = BMP180_REG_MEAS;
	err  = me_i2c_req_resp(conf->port, conf->addr, req, 1, res, 3);
    if( err != ESP_OK )
        return err;
    
    uc->pressure = (   ((uint32_t)res[0] << 16) 
                     | ((uint32_t)res[1] << 8) 
                     | ((uint32_t)res[2])) >> (8-ME_BMP180_OVERSAMPLING); 

    return ESP_OK;
}


esp_err_t me_bmp180_read(me_bmp180_conf_t *c, me_bmp180_readings_t *r)
{
    esp_err_t       err;   
    bmp180_uc_t     uc;
    int32_t         x1, x2, x3, b3, b5, b6, pressure;
    uint32_t        b4, b7;

    err = bmp180_meas_read(c, &uc);
    if( err != ESP_OK )
        return err;

    x1 = (((int32_t)uc.temperature - (int32_t)c->calib.ac6) * ((int32_t)c->calib.ac5))>>15;

    if( x1 == 0 && c->calib.md == 0 )
        return ESP_ERR_INVALID_RESPONSE;
    
    x2 = ((int32_t)c->calib.mc<<11)/(x1+c->calib.md);

    b5 = x1 + x2;
    r->temperature = ((b5+8)>>4);

    // pressure
    b6 = b5 - 4000;
    x1 = b6 * b6 >> 12;
    x1 *= c->calib.b2;
    x1 >>= 11;
    x2 = (c->calib.ac2 * b6);
    x2 >>= 11;
    x3 = x1 + x2;
    b3 = ((((int32_t)c->calib.ac1*4 + x3)<<ME_BMP180_OVERSAMPLING) + 2) >> 2;

    x1 = (c->calib.ac3 * b6) >> 13;
    x2 = (c->calib.b1 * ((b6 * b6) >> 12))>>16;
    x3 = (x1 + x2 + 2) >> 2;
    b4 = (c->calib.ac4 * (uint32_t)(x3+32768)) >> 15;
    
    if( b4 == 0 )
        return ESP_ERR_INVALID_RESPONSE;

    b7 = ((uint32_t)(uc.pressure - b3)) * (50000 >> ME_BMP180_OVERSAMPLING);
    if( b7 < 0x80000000 )
    {
        pressure = (b7 << 1) / b4;
    } else {
        pressure = (b7 / b4) << 1;
    }

    x1 = pressure >> 8;
    x1 = (x1 * x1 * 3038) >> 16;
    x2 = (pressure * (-7357)) >>16;
    
    r->pressure = pressure + ((x1 + x2 + 3791) >> 4);

    return ESP_OK;
}


esp_err_t me_bmp180_info(me_bmp180_conf_t *conf, uint8_t *data)
{
    uint8_t req[] = { BMP180_REG_ID };
/**
 *        data[0] - CHIP ID
 *        data[1] - VERSION
 */

    return me_i2c_req_resp(conf->port, conf->addr, req, 1, data, 2);
}


esp_err_t me_bmp180_init(me_bmp180_conf_t *conf)
{
	esp_err_t	err;
    uint8_t    	req[6];
	uint8_t    	res[26];

    err = me_bmp180_info(conf, res);
    if( err == ESP_OK )
	{
		if( res[0] != BMP180_CHIP_ID )
			return ESP_ERR_INVALID_VERSION;
	} else
        return err;

    // Do the soft reset
    req[0] = BMP180_REG_RESET;
    req[1] = BMP180_CMD_RESET;
	err    = me_i2c_write(conf->port, conf->addr, req, 2);
    if( err != ESP_OK )
		return err;

	vTaskDelay(200/portTICK_PERIOD_MS);

	/* read calibration data */
    req[0] = BMP180_REG_CALIB;
	err  = me_i2c_req_resp(conf->port, conf->addr, req, 1, res, 22);
    if( err != ESP_OK )
        return err;

    conf->calib.ac1 = (res[0] << 8) | res[1];
    conf->calib.ac2 = (res[2] << 8) | res[3];
    conf->calib.ac3 = (res[4] << 8) | res[5];
    conf->calib.ac4 = (res[6] << 8) | res[7];
    conf->calib.ac5 = (res[8] << 8) | res[9];
    conf->calib.ac6 = (res[10] << 8) | res[11];
    conf->calib.b1  = (res[12] << 8) | res[13];
    conf->calib.b2  = (res[14] << 8) | res[15];
    conf->calib.mb  = (res[16] << 8) | res[17];
    conf->calib.mc  = (res[18] << 8) | res[19];
    conf->calib.md  = (res[20] << 8) | res[21];

	return ESP_OK;
}

