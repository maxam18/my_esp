/* BME280 i2c sensor reader
 * File: app_bme.c
 * Started: Fri Nov 16 10:04:58 MSK 2018
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2018 ..."
 */

#include "esp_err.h"
#include <me_bme280.h>

/**\name BME280 chip identifier */
#define BME280_CHIP_ID  			0x60

/**\name Register Address (Table 18) */
#define BME280_REG_HUM_LSB			0xFE
#define BME280_REG_HUM_MSB			0xFD
#define BME280_REG_TEMP_XLSB		0xFC
#define BME280_REG_TEMP_LSB			0xFB
#define BME280_REG_TEMP_MSB			0xFA
#define BME280_REG_PRESS_XLSB		0xF9
#define BME280_REG_PRESS_LSB		0xF8
#define BME280_REG_PRESS_MSB		0xF7
#define BME280_REG_CONFIG			0xF5
#define BME280_REG_CTRL_MEAS		0xF4
#define BME280_REG_STATUS			0xF3
#define BME280_REG_CTRL_HUM			0xF2
#define BME280_REG_CALIB_26_41 	    0xE1
#define BME280_REG_RESET			0xE0
#define BME280_REG_ID				0xD0
#define BME280_REG_CALIB_00_25		0x88

/**\name Sensor commands */
#define BME280_CMD_RESET			0xB6


/**\name Sensor power modes */
#define	BME280_SLEEP_MODE			0x00
#define	BME280_FORCED_MODE			0x01
#define	BME280_NORMAL_MODE			0x03

/**\name StandBy time for NORMAL MODE */
#define BME280_T_SB_10				0xC0
#define BME280_T_SB_20				0xE0
#define BME280_T_SB_125				0x40
#define BME280_T_SB_250				0x60
#define BME280_T_SB_500				0x80
#define BME280_T_SB_1000			0xA0

/**\name Status */
#define BME280_STATUS_MEASURING     0x08 /* bit 3 */
#define BME280_STATUS_UPDATING      0x01 /* bit 0 */
#define BME280_STATUS_BUSY          (0x08 | 0x01)


/**
 * @brief temperature compensation 
 *        Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
 *        temp carries t_fine temperature as global value
 */
static int32_t compensate_temperature(me_bme280_calib_t *c, me_bme280_readings_t *d)
{
	int32_t v1, v2;
    v1 = ((((d->temperature>>3) - ((int32_t)c->dig_T1<<1))) 
		  * ((int32_t)c->dig_T2)) >> 11;
    v2 = (((((d->temperature>>4) - ((int32_t)c->dig_T1)) * ((d->temperature>>4) 
		   - ((int32_t)c->dig_T1))) >> 12) 
		  * ((int32_t)c->dig_T3)) >> 14;

    d->temperature = v1 + v2;

	return (d->temperature * 5 + 128) >> 8;
}

/**
 * @brief pressure compensation 
 *        Returns pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
 */
static uint32_t compensate_pressure(me_bme280_calib_t *c, me_bme280_readings_t *d)
{
	int32_t  v1, v2;
	uint32_t p;
	
    v1 = (((int32_t)d->temperature)>>1) - (int32_t)64000;
    v2 = (((v1>>2) * (v1>>2)) >> 11 ) * ((int32_t)c->dig_P6);
    v2 = v2 + ((v1*((int32_t)c->dig_P5))<<1);
    v2 = (v2>>2)+(((int32_t)c->dig_P4)<<16);
    v1 = (((c->dig_P3 * (((v1>>2) * (v1>>2)) >> 13 )) >> 3)
          + ((((int32_t)c->dig_P2) * v1)>>1))>>18;

    v1 =((((32768+v1))*((int32_t)c->dig_P1))>>15);
	if (v1 == 0)
		return 0; // avoid exception caused by division by zero

	p = (((uint32_t)(((int32_t)1048576)-d->pressure)-(v2>>12)))*3125;
	if (p < 0x80000000)
		p = (p << 1) / ((uint32_t)v1);
	else
		p = (p / (uint32_t)v1) * 2;

	v1 = (((int32_t)c->dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
	v2 = (((int32_t)(p>>2)) * ((int32_t)c->dig_P8))>>13;

	p = (uint32_t)((int32_t)p + ((v1 + v2 + c->dig_P7) >> 4));
	return p;
}

/**
 * @brief   humidity compensation 
 *          Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
 *          Output value of “47445” represents 47445/1024 = 46.333 %RH
 */
static uint32_t compensate_humidity(me_bme280_calib_t *c, me_bme280_readings_t *d)
{
	int32_t v;

	v = (d->temperature - ((int32_t)76800));
	v = (((((d->humidity << 14) - (((int32_t)c->dig_H4) << 20) 
			- (((int32_t)c->dig_H5) * v)) + ((int32_t)16384)) >> 15)
		 * (((((((v * ((int32_t)c->dig_H6)) >> 10) 
		        * (((v * ((int32_t)c->dig_H3)) >> 11)
		           + ((int32_t)32768))
		       ) >> 10) + ((int32_t)2097152)
		     ) * ((int32_t)c->dig_H2) + 8192
		    ) >> 14));
	v = (v - (((((v >> 15) * (v >> 15)) >> 7)
	           * ((int32_t)c->dig_H1)) >> 4));
    v = (v < 0 ? 0 : v);
    v = (v > 419430400 ? 419430400 : v);
    
    return (uint32_t)(v>>12);
}

/**
 * @brief read BME280 info register
 *        data[0] - CHIP ID
 *        data[1] - CHIP VERSION
 *
 * @param conf    bme280_conf_t pointer
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_bme280_info(me_bme280_conf_t *conf, uint8_t *data)
{
    uint8_t req[] = { BME280_REG_ID };

    return me_i2c_req_resp(conf->port, conf->addr, req, 1, data, 2);
}

esp_err_t me_bme280_init(me_bme280_conf_t *conf)
{
	esp_err_t	err;
    uint8_t    	req[6];
	uint8_t    	res[26];

    err = me_bme280_info(conf, res);
    if( err == ESP_OK )
	{
		if( res[0] != BME280_CHIP_ID )
			return ESP_FAIL;
	} else
        return err;

	
    // Do the soft reset
    req[0] = BME280_REG_RESET;
    req[1] = BME280_CMD_RESET;
	err    = me_i2c_write(conf->port, conf->addr, req, 2);
    if( err != ESP_OK )
		return err;

	vTaskDelay(200/portTICK_PERIOD_MS);

	// BME280 setup. Oversampling, filtering
    req[0]  = BME280_REG_CTRL_HUM;
    req[1]  = ME_BME280_OVERSAMPLE_HUM;
    req[2]  = BME280_REG_CTRL_MEAS;
    req[3]  = ME_BME280_OVERSAMPLE_TEMP | 
              ME_BME280_OVERSAMPLE_PRESS | 
			  BME280_NORMAL_MODE;
	req[4]  = BME280_REG_CONFIG;
	req[5]  = ME_BME280_FILTER_COEFF | 
			  BME280_T_SB_500;
	
	
    err = me_i2c_write(conf->port, conf->addr, req, 6);
	if( err != ESP_OK )
		return err;

	/* read calibration data */
    req[0] = BME280_REG_CALIB_00_25;
	err  = me_i2c_req_resp(conf->port, conf->addr, req, 1
                            , (uint8_t *)&conf->calib, 26);
    if( err != ESP_OK )
        return err;

    req[0] = BME280_REG_CALIB_26_41;
	err  = me_i2c_req_resp(conf->port, conf->addr, req, 1        
                            , res, 41-26+1);
	if( err != ESP_OK )
        return err;

	conf->calib.dig_H2 = (int16_t)((int16_t)res[1] << 8 | res[0]);
	conf->calib.dig_H3 = (uint8_t)res[2];
	conf->calib.dig_H4 = (int16_t)((int16_t)res[3] << 4 | (res[4] & 0x0F));
	conf->calib.dig_H5 = (int16_t)((int16_t)res[5] << 4 | ((res[4] >> 4) & 0x0F));
	conf->calib.dig_H6 = (int8_t)res[6];

	return ESP_OK;
}

esp_err_t me_bme280_read(me_bme280_conf_t *conf, me_bme280_readings_t *readings)
{
	esp_err_t			   err;
	me_bme280_readings_t   uncomp;
	uint8_t                req[1];
	uint8_t                res[8];

#ifdef CONFIG_BME280_FORCED_MODE
	uint8_t                cnt = 0;

    req[0] = BME280_REG_STATUS;
	do {
		if( cnt++ > 5 )
			return ESP_FAIL;

		vTaskDelay(( 1.25 + (2.3 * ME_BME280_OVERSAMPLE_TEMP)
			     + ((2.3 * ME_BME280_OVERSAMPLE_PRESS) + 0.575) 
			     + ((2.3 * ME_BME280_OVERSAMPLE_HUM) + 0.575)
			   ) / portTICK_PERIOD_MS
			  );

		err = me_i2c_req_resp(conf->port, conf->addr, req, 1, res, 1);
	} while( (err == ESP_OK) && (res[0] & BME280_STATUS_UPDATING) );
#endif

    req[0] = BME280_REG_PRESS_MSB;

	err = me_i2c_req_resp(conf->port, conf->addr, req, 1, res, 8);
	if( err != ESP_OK )
        return err;

	uncomp.pressure    = ( ((int32_t)res[0] << 12)
                           | ((int32_t)res[1] << 4)
                           | (res[2] >> 4)
                         );
	uncomp.temperature = ( ((int32_t)res[3] << 12)
                           | ((int32_t)res[4] << 4)
                           | (res[5] >> 4)
                         );
	uncomp.humidity    = (((int32_t)res[6] <<  8) | (res[7]));

	readings->temperature = compensate_temperature(&conf->calib, &uncomp);
	readings->pressure    = compensate_pressure(&conf->calib, &uncomp);
	readings->humidity    = compensate_humidity(&conf->calib, &uncomp);

	return ESP_OK;
}
