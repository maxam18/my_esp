/* General I2C functions
 * File: me_i2c.c
 * Started: Sun Jun 13 22:22:54 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */

#include "esp_system.h"
#include "esp32/rom/ets_sys.h" /* ets_delay_us */
#include "me_i2c.h"

#include "me_debug.h"

#define APB_CYCLES          80000000
#define APB_US_CYCLES       125 / 10000 /*!< 1 000 000 us / 80 000 000 cycles */
#define APB_CYCLES_US       80      /*!< 80 000 000 cycles / 1 000 000 us */
#define ACK_CHECK_EN        0x1     /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS       0x0     /*!< I2C master will not check ack from slave */
#define ACK_VAL             0x0     /*!< I2C ack value */
#define NACK_VAL            0x1     /*!< I2C nack value */

#define I2C_RESET_ADDR      0x00
#define I2C_RESET_CMD       0x06

/* start, stop, ack + 8 bits + RTOS cycles. 
 * Timeout for RTOS cannot be less than 1.
 * It doesn't realy slows down. Once the reply is read ISR will fire
 * Set 10ms at least for a case */
#define _TICK_TIMEOUT(C)    ((C * 125 / 10000000) + 10) \
                            / portTICK_PERIOD_MS

int _ack_timeout_ticks[2];

static uint8_t  initialized[2] = {0,0};

esp_err_t me_i2c_master_init(i2c_port_t port, int sda_pin, int scl_pin, int speed, me_i2c_pullup_t pullup)
{
    int             timeout;
    esp_err_t       ret;
    i2c_config_t    c = {
		.mode 			  = I2C_MODE_MASTER,
		.master.clk_speed = speed
	};

    if( initialized[port] )
        return ESP_ERR_INVALID_STATE;

    c.sda_pullup_en = c.scl_pullup_en = pullup;

	c.sda_io_num = sda_pin;
	c.scl_io_num = scl_pin;
    
	if( (ret = i2c_param_config(port, &c)) == ESP_OK )
        ret = i2c_driver_install(port, c.mode, 0, 0, 0);

    i2c_get_timeout(port, &timeout);
    _ack_timeout_ticks[port] = _TICK_TIMEOUT(timeout);

    me_debug( "i2c", "Port: %d, PIN scl: %d, PIN sda: %d, APB timeout: %d, Timeout: %d", port, scl_pin, sda_pin, timeout,  _ack_timeout_ticks[port]);

    if( ret == ESP_OK )    
        initialized[port] = 1;

    return ret;
}

esp_err_t me_i2c_reset_timeout(i2c_port_t port, int us)
{
    int timeout;

    timeout = APB_CYCLES_US * us;
    _ack_timeout_ticks[port] = _TICK_TIMEOUT(timeout);

    me_debug( "i2c", "Port: %d, Timeout: %d", port,  _ack_timeout_ticks[port]);

    return i2c_set_timeout(port, timeout);
}

esp_err_t me_i2c_write(i2c_port_t port, uint8_t addr, uint8_t *data, uint8_t size)
{
    int                 timeout;
    esp_err_t           err;
    i2c_cmd_handle_t    cmd;
    
    cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    if( size )
        i2c_master_write(cmd, data, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);

    timeout = 2 * (size+2) * _ack_timeout_ticks[port];

    err = i2c_master_cmd_begin(port, cmd, timeout);
    i2c_cmd_link_delete(cmd);

    return err;
}

esp_err_t me_i2c_read(i2c_port_t port, uint8_t addr, void *data, uint8_t size)
{
    int                 timeout;
    esp_err_t           err;
    i2c_cmd_handle_t    cmd;

    if( !size )
        return ESP_ERR_INVALID_ARG;

    cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, ACK_VAL);
    i2c_master_read(cmd, data, size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    timeout = 2 * (size+2) * _ack_timeout_ticks[port];

    err = i2c_master_cmd_begin(port, cmd, timeout);
    i2c_cmd_link_delete(cmd);
    
    return err;
}

esp_err_t me_i2c_req_resp(i2c_port_t port, uint8_t addr, uint8_t *req, uint8_t req_size, void *resp, uint8_t resp_size)
{
    int                 timeout;
    esp_err_t           err;
    i2c_cmd_handle_t    cmd;

    cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write(cmd, req, req_size, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
    i2c_master_read(cmd, resp, resp_size, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    timeout = 2 * (req_size+resp_size+4) * _ack_timeout_ticks[port];

    err = i2c_master_cmd_begin(port, cmd, timeout);
    i2c_cmd_link_delete(cmd);
    
    return err;
}

esp_err_t me_i2c_bus_reset(i2c_port_t port)
{
    esp_err_t           err;
    uint8_t             req[1];

    err = i2c_reset_tx_fifo(port);
    if( err != ESP_OK )
        return err;
    
    err = i2c_reset_rx_fifo(port);
    if( err != ESP_OK )
        return err;
    
    req[0] = I2C_RESET_CMD;

    return me_i2c_write(port, I2C_RESET_ADDR, req, 1);
}
