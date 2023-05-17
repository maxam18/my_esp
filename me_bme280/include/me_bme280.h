/* BME280 reader
 * File: app_bme.h
 * Started: Fri Nov 16 10:41:11 MSK 2018
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2018 ..."
 */

#include <me_i2c.h>


#ifndef _ME_BME280_H
#define _ME_BME280_H

/**
 * @brief Oversampling 
 * */
#define ME_BME280_OVERSAMPLING_NO		0x00
#define ME_BME280_OVERSAMPLING_1X		0x01
#define ME_BME280_OVERSAMPLING_2X		0x02
#define ME_BME280_OVERSAMPLING_4X		0x03
#define ME_BME280_OVERSAMPLING_8X		0x04
#define ME_BME280_OVERSAMPLING_16X		0x05

/**\name Filter coefficient selection */
#define ME_BME280_FILTER_COEFF_OFF     	0x00
#define ME_BME280_FILTER_COEFF_2       	0x01
#define ME_BME280_FILTER_COEFF_4       	0x02
#define ME_BME280_FILTER_COEFF_8       	0x03
#define ME_BME280_FILTER_COEFF_16      	0x04

/**
 * @brief oversampling setup
 */
#define ME_BME280_OVERSAMPLE_HUM		ME_BME280_OVERSAMPLING_1X
#define ME_BME280_OVERSAMPLE_TEMP		(ME_BME280_OVERSAMPLING_2X << 5)
#define ME_BME280_OVERSAMPLE_PRESS	    (ME_BME280_OVERSAMPLING_16X << 2)

/**
 * #@brief filter setup
 */
#define ME_BME280_FILTER_COEFF	      	ME_BME280_FILTER_COEFF_16

/**\name I2C addresses */
#define ME_BME280_I2C_ADDR_PRIMARY		0x76
#define ME_BME280_I2C_ADDR_SECONDARY	0x77

/**
 * @brief Temp and pressure calibration data (Table 16)
 */
typedef struct {
	uint16_t		dig_T1;  /*!< reg 0x88 - 0x89 0x9F*/
	int16_t 		dig_T2;  /*!< reg 0x8A - 0x8B  */      
	int16_t 		dig_T3;  /*!< reg 0x8C - 0x8D  */      
	uint16_t		dig_P1;  /*!< reg 0x8E - 0x8F  */      
	int16_t 		dig_P2;  /*!< reg 0x90 - 0x91  */      
	int16_t 		dig_P3;  /*!< reg 0x92 - 0x93  */      
	int16_t 		dig_P4;  /*!< reg 0x94 - 0x95  */      
	int16_t 		dig_P5;  /*!< reg 0x96 - 0x97  */      
	int16_t 		dig_P6;  /*!< reg 0x98 - 0x99  */      
	int16_t 		dig_P7;  /*!< reg 0x9A - 0x9B  */      
	int16_t 		dig_P8;  /*!< reg 0x9C - 0x9D  */      
	int16_t 		dig_P9;  /*!< reg 0x9E - 0x9F  */      
	char    		e;       /*!< A0 - alignment   */
	uint8_t 		dig_H1;  /*!< 0xA1             */
	int16_t 		dig_H2;  /*!< 0xE1 - 0xE2      */
	uint8_t 		dig_H3;  /*!< 0xE3             */
	int16_t 		dig_H4;  /*!< 0xE4 - 0xE5[3-0] */
	int16_t 		dig_H5;  /*!< 0xE5[7:4] - 0xE6 */
	int8_t  		dig_H6;  /*!< 0xE7             */
} me_bme280_calib_t;

/**
 * @brief BME280 readings 
 */
typedef struct {
	int32_t  		temperature; /*!< temperature * 100 oC*/
	int32_t  		humidity;    /*!< humidity * 1024 %RH */
	int32_t  		pressure;    /*!< pressure * 100 hPa */
} me_bme280_readings_t;

/**
 * @brief BME280 configuration 
 */
typedef struct {
    me_bme280_calib_t  calib;    /*!< calibration data */
    i2c_port_t         port;     /*!< I2C bus number */
    uint8_t            addr;     /*!< I2C sensor's slave address */
} me_bme280_conf_t;

/**
 * @brief ME_BME280 default config
 */
#define ME_BME280_CONFIG_DEF	{ \
									{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, \
									I2C_NUM_0, \
									ME_BME280_I2C_ADDR_PRIMARY \
								}

/**
 * @brief BM280 initialization
 *        Does not check CHIP_VERSION.
 *        Resets device. Uploads oversampling info. 
 *          (see `OVERSAMPLE_HUM`, `OVERSAMPLE_TEMP`, `OVERSAMPLE_PRESS`)
 *        Initializes BME280 for `BME280_NORMAL_MODE`
 *        Reads compensation data from BME280
 *
 * @param conf    bme280_conf_t pointer
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_bme280_init(me_bme280_conf_t *conf);

/**
 * @brief read BME280 info register
 *        
 *
 * @param conf    bme280_conf_t pointer
 * @param data    2 bytes buffer to place info into
 *                   * data[0] - CHIP ID
 *                   * data[1] - CHIP VERSION
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_bme280_info(me_bme280_conf_t *conf, uint8_t *data);

/**
 * @brief read BME280 measurements
 *        
 *
 * @param conf      bme280_conf_t pointer
 * @param readings  pointer to BME280 readings structure
 *                  the actual values will be filled in
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_bme280_read(me_bme280_conf_t *conf, me_bme280_readings_t *readings);

#endif

