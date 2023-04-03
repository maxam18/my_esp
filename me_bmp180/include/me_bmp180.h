/* My ESP MBP180 driver
 * File: me_bmp180.h
 * Started: Sun Apr  2 12:57:15 MSK 2023
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2023 ..."
 */

#include "esp_err.h"
#include <me_i2c.h>

#ifndef _ME_BMP180_H
#define _ME_BMP180_H

/**
 * @brief Oversampling 
 * */
#define ME_BMP180_OVER_NO		0x0
#define ME_BMP180_OVER_2X		0x1
#define ME_BMP180_OVER_4X		0x2
#define ME_BMP180_OVER_8X		0x3

/**
 * @brief oversampling setup
 */
#define ME_BMP180_OVERSAMPLING		    ME_BMP180_OVER_4X

/**\name I2C addresses */
#define ME_BMP180_I2C_ADDR_PRIMARY		0x77

/**
 * @brief Temp and pressure calibration data (Table 16)
 */
typedef struct {
    int16_t         ac1; /*!< reg 0xAA 0xAB signed */
    int16_t         ac2; /*!< reg 0xAC 0xAD signed */
    int16_t         ac3; /*!< reg 0xAE 0xAF signed */
    uint16_t        ac4; /*!< reg 0xB0 0xB1 unsigned */
    uint16_t        ac5; /*!< reg 0xB2 0xB3 unsigned */
    uint16_t        ac6; /*!< reg 0xB4 0xB5 unsigned */

    int16_t         b1;  /*!< reg 0xB6 0xB7 signed */
    int16_t         b2;  /*!< reg 0xB8 0xB9 signed */
    int16_t         mb;  /*!< reg 0xBA 0xBB signed */
    int16_t         mc;  /*!< reg 0xBC 0xBD signed */
    int16_t         md;  /*!< reg 0xBE 0xBF signed */
} me_bmp180_calib_t;

/**
 * @brief BMP180 readings 
 */
typedef struct {
	int32_t  		temperature; /*!< temperature * 10 oC*/
	int32_t  		pressure;    /*!< pressure */
} me_bmp180_readings_t;

/**
 * @brief BMP180 configuration 
 */
typedef struct {
    me_bmp180_calib_t  calib;    /*!< calibration data */
    i2c_port_t         port;     /*!< I2C bus number */
    uint8_t            addr;     /*!< I2C sensor's slave address */
} me_bmp180_conf_t;

/**
 * @brief BMP180 initialization
 *        Check CHIP_ID
 *        Does not check CHIP_VERSION.
 *        Resets device.
 *        Reads compensation data from BMP180
 *
 * @param conf    bmp180_conf_t pointer
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_bmp180_init(me_bmp180_conf_t *conf);

/**
 * @brief read BMP180 info register
 *        
 *
 * @param conf    bmp180_conf_t pointer
 * @param data    2 bytes buffer to place info into
 *                   * data[0] - CHIP ID
 *                   * data[1] - CHIP VERSION
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_bmp180_info(me_bmp180_conf_t *conf, uint8_t *data);

/**
 * @brief read BMP180 measurements
 *        
 *
 * @param conf      bmp180_conf_t pointer
 * @param readings  pointer to BMP180 readings structure
 *                  the actual values will be filled in
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_bmp180_read(me_bmp180_conf_t *conf, me_bmp180_readings_t *readings);

#endif
