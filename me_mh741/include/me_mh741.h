/* MH741A Winsen gas concentration sensor
 * File: me_mh741.h
 * Started: Mon Jun 14 20:44:18 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */

#ifndef _ME_MH741_H
#define _ME_MH741_H

#include <me_i2c.h>

/**
 * @brief Setup timer for MH741
 *        Sets delay in microseconds for how long MH741 can hold the bus
 *        Call this function after i2c initialization `me_i2c_master_init`
 *
 * @param port    I2C port number   
 *
 * @return
 *     - ESP_OK               Success
 *     - ESP_ERR_INVALID_ARG  Parameter error
 */
#define me_mh741_init(port) me_i2c_reset_timeout(port, 1000)

/**
 * @brief Read concentration
 *        0-100%, 0-30% sensors will reply with %VOL * 100
 *
 * @param port    I2C port number
 * @param value   pointer to a value. Sets to -1 on error. Reply checksum incorrect if ESP_OK in return and value is set to -1
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t me_mh741_read(i2c_port_t port, int16_t *value);

/**
 * @brief Calibrate sensor
 *        ppm = 0 - zero point calibration
 *        ppm > 0 - span calibration
 *
 * @param port    I2C port number
 * @param value   PPM value. Set value to 0 for Zero point calibration 
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
esp_err_t me_mh741_calibrate(i2c_port_t port, int ppm);


#endif

