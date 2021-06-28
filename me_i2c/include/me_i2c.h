/* General I2C functions
 * File: me_i2c.h
 * Started: Sun Jun 13 22:23:02 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */

#ifndef _ME_I2C_H
#define _ME_I2C_H

#include "driver/i2c.h"
#include "sdkconfig.h"

typedef enum {
    ME_I2C_PULLUP_ENABLE  = GPIO_PULLUP_ENABLE,
    ME_I2C_PULLUP_DISABLE = GPIO_PULLUP_DISABLE
} me_i2c_pullup_t;


/**
 * @brief microseconds delay. 
 *        RTOS delay (`vTaskDelay`) is 1ms long. 
 *        Asking 1ms could result either in 1ms or 0 ms delay
 *        This macro selects `vTaskDelay` in the case 2+ millis delay  
 *
 * @param us      delay in microseconds
 */

#define us_delay( us )    \
    { \
        if( us_delay > 2000 ) \
            vTaskDelay( us / 1000 / portTICK_PERIOD_MS ); \
        else \
            ets_delay_us( us ); \
    }

/**
 * @brief Initialize i2c
 *
 * @param port    I2C port number
 * @param sda_pin PIN for SDA
 * @param scl_pin PIN for SCL
 * @param speed   SCL frequency (1000 to 400000 Hz).
 * @param pullup  Should internal pullup be set for SDA and SCL ping
 *
 * @return
 *     - ESP_OK               Success
 *     - ESP_ERR_INVALID_ARG  Parameter error
 *     - ESP_FAIL             Driver install error
 */
esp_err_t me_i2c_master_init(i2c_port_t port, int sda_pin, int scl_pin, int speed, me_i2c_pullup_t pullup);


/**
 * @brief Set internal I2C timeout and reset wait timeout
 *
 * @param port    I2C port number
 * @param us      timeout in microseconds 
 *
 * @return
 *     - ESP_OK               Success
 *     - ESP_ERR_INVALID_ARG  Parameter error
 *     - ESP_FAIL             Driver install error
 */
esp_err_t me_i2c_reset_timeout(i2c_port_t port, int us);


/**
 * @brief I2C ping slave. 
 *        Ping sends write command and stop bit right after
 *
 * @param port I2C port number
 * @param addr I2C slave address
 *
 * @return
 *     - ESP_OK Success
 *      - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 */
#define me_i2c_ping(port, addr) me_i2c_write(port, addr, NULL, 0)

/**
 * @brief Deinitialize I2C (delete driver)
 *
 * @param port I2C port number
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
#define me_i2c_deinit(port) i2c_driver_delete(port)

/**
 * @brief Reset I2C bus and fifo buffers
 *          Has no means for master. i2c_master_cmd_begin does it 
 * @param port I2C port number
 *
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 *      
 */
esp_err_t me_i2c_bus_reset(i2c_port_t port);


/**
 * @brief I2C write bytes to Slave
 *        Tha latest byte is set to NACK if bytes count more than 1
 * 
 * @param port I2C port number
 * @param addr I2C slave address
 * @param data bytes to write to device
 * @param size number of bytes in `data`
 * 
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 *      
 */
esp_err_t me_i2c_write(i2c_port_t port, uint8_t addr, uint8_t *data, uint8_t size);

/**
 * @brief I2C read bytes from Slave
 *        Tha latest byte is set to NACK if bytes count more than 1
 * 
 * @param port I2C port number
 * @param addr I2C slave address
 * @param data buffer to place read bytes
 * @param size number of bytes expected from the device
 * 
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 *      
 */
esp_err_t me_i2c_read(i2c_port_t port, uint8_t addr, void *data, uint8_t size);

/**
 * @brief I2C write command to and read bytes from Slave
 *        Tha latest byte to read is set to NACK if bytes count more than 1
 * 
 * @param port I2C port number
 * @param addr I2C slave address
 * @param req bytes to write to device
 * @param req_size number of bytes in `req`
 * @param resp buffer to place read bytes
 * @param resp_size number of bytes expected from the device
 * 
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.
 *      
 */
esp_err_t me_i2c_req_resp(i2c_port_t port, uint8_t addr, uint8_t *req, uint8_t req_size, void *resp, uint8_t resp_size);

#endif

