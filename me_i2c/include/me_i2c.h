/**
 * @brief My ESP (me) General I2C master functions
 * @file me_i2c.h
 * @since Sun Jun 13 22:23:02 MSK 2021
 * @author Max Amzarakov (maxam18 _at_ gmail _._ com)
 * @details general purpose i2c read write functions
 *          the library has internal configuration for both of ESP32 
 *          i2c ports
 *          Call initialization function @see me_i2c_master_init
 *          ` ESP_ERROR_CHECK(me_i2c_master_init());
 *            me_i2c_ping();
 *          `
 * 
 * Copyright (c) 2021 ... by Max Amzarakov
 */

/**
 * @file buffer.h
 *
 * Interfaces to Data Buffer.
 *
 * This header define the interfaces of general purpose dynamic data buffer that
 * implemented by Equinox.
 */

#ifndef _ME_I2C_H
#define _ME_I2C_H

#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>

#ifdef CONFIG_ME_I2C_USE_I2C_MASTER
#include <driver/i2c_master.h>
#define me_i2c_bus_handle_t     i2c_master_bus_handle_t
#else
#include "driver/i2c.h"
#include "sdkconfig.h"
#endif 

typedef enum {
    ME_I2C_PULLUP_ENABLE  = GPIO_PULLUP_ENABLE,
    ME_I2C_PULLUP_DISABLE = GPIO_PULLUP_DISABLE
} me_i2c_pullup_t;

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
 *     - ESP_ERR_INVALID_STATE  Duplicate initialization
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
 #ifdef CONFIG_ME_I2C_USE_I2C_MASTER
esp_err_t me_i2c_ping(i2c_port_t port, int addr);
#else
#define me_i2c_ping(port, addr) me_i2c_write(port, addr, NULL, 0)
#endif

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

/**
 * @brief I2C write to register
  * 
 * @param port I2C port number
 * @param addr I2C slave address
 * @param reg register to write to
 * @param val value to write to the register
 * 
 * @return
 *     - ESP_OK Success
 *     - ESP_ERR_INVALID_ARG Parameter error
 *     - ESP_FAIL Sending command error, slave doesn't ACK the transfer.
 *     - ESP_ERR_INVALID_STATE I2C driver not installed or not in master mode.
 *     - ESP_ERR_TIMEOUT Operation timeout because the bus is busy.    
 */
esp_err_t me_i2c_write_reg(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t val);

#endif

