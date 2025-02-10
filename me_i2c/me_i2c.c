/* General I2C functions
 * File: me_i2c.c
 * Started: Sun Jun 13 22:22:54 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */

#include "esp_system.h"
//#include "esp32/rom/ets_sys.h" /* ets_delay_us */
#include "me_i2c.h"

#include "hal/i2c_types.h"
#include "me_debug.h"
#include <stdatomic.h>
#include <stdint.h>
#include <sys/_intsup.h>

#ifdef CONFIG_ME_I2C_DEBUG
#include <esp_log.h>
static const char   *TAG = "ME-I2C";
#define dd(...)     ESP_LOGI( TAG, __VA_ARGS__)
#define de(...)     ESP_LOGE( TAG, __VA_ARGS__)
#else
#define dd(...)
#define de(...)
#endif

#ifdef CONFIG_ME_I2C_USE_I2C_MASTER
#ifndef CONFIG_ME_I2C_GLITCH_IGNORE_CNT
#define CONFIG_ME_I2C_GLITCH_IGNORE_CNT 7
#endif
#ifndef CONFIG_ME_I2C_DEF_OP_TIMEOUT_MS
#define CONFIG_ME_I2C_DEF_OP_TIMEOUT_MS 100
#endif

typedef struct dev_vec_s dev_vec_t;

struct dev_vec_s {
    i2c_master_dev_handle_t     i2c_dev;
    dev_vec_t                  *next;
    int                         i2c_dev_addr;
};

i2c_master_bus_handle_t     i2c_bus_handle[I2C_NUM_MAX] = { NULL };
static int                  def_speed = 0;
static int                  def_ack_wait_us[2] = {0};
dev_vec_t                   devices = {0};

esp_err_t me_i2c_master_init(i2c_port_t port, int sda_pin, int scl_pin, int speed, me_i2c_pullup_t pullup)
{
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port   = port,
        .scl_io_num = scl_pin,
        .sda_io_num = sda_pin,
        .glitch_ignore_cnt = CONFIG_ME_I2C_GLITCH_IGNORE_CNT,
        .flags = { .enable_internal_pullup = pullup}
    };

    if( (uint8_t)port >= I2C_NUM_MAX )
        return ESP_ERR_INVALID_ARG;

    dd("I2C init requested. Port: %d", port);

    if( i2c_bus_handle[port] )
        return ESP_OK;

    def_speed = speed;

    dd("I2C init. Initializing. Port: %d", port);

    return i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle[port]);
}


static esp_err_t check_port(i2c_port_t port)
{
    if( (uint8_t)port >= I2C_NUM_MAX )
        return ESP_ERR_INVALID_ARG;

    if( !i2c_bus_handle[port] )
        return ESP_ERR_INVALID_STATE;

    dd("Check port OK. Port %d, Addr: 0x%X", port, (uintptr_t)i2c_bus_handle[port]);

    return ESP_OK;
}


static esp_err_t get_i2c_dev(i2c_port_t port, int addr, i2c_master_dev_handle_t *devp)
{
    esp_err_t                    err;
    dev_vec_t                   *vecp, *vecn;
    i2c_device_config_t          dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = def_speed,
    };

    err = check_port(port);
    if( err != ESP_OK )
        return err;

    if( unlikely(!devp) )
        return ESP_ERR_INVALID_ARG;

    dd("I2C dev lookup. Addr: %d", addr);

    vecp = &devices;
    while( vecp->next )
    {
        if( vecp->i2c_dev_addr == addr )
        {
            *devp = vecp->i2c_dev;
        }
        vecp = vecp->next;
    }

/* NOTE: non thread safe segment! */
    vecn = malloc(sizeof(dev_vec_t));
    if( !vecn )
        return ESP_ERR_NO_MEM;

    dd("New I2C dev");

    vecn->i2c_dev_addr  = addr;
    vecn->next          = NULL;
    dev_cfg.scl_wait_us = def_ack_wait_us[port];

    err = i2c_master_bus_add_device(i2c_bus_handle[port], &dev_cfg, &vecn->i2c_dev);
    if( err != ESP_OK )
    {
        free(vecn);
        return err;
    }

    vecp->next  = vecn;
    *devp       = vecn->i2c_dev;

    dd("I2C dev added");

    return ESP_OK;
}


esp_err_t me_i2c_reset_timeout(i2c_port_t port, int us)
{
    def_ack_wait_us[port] = us;

    return ESP_OK;
}


esp_err_t me_i2c_ping(i2c_port_t port, int addr)
{
    esp_err_t err;

    err = check_port(port);

    if( err == ESP_OK )
        err = i2c_master_probe(i2c_bus_handle[port], addr, CONFIG_ME_I2C_DEF_OP_TIMEOUT_MS);

    return err;
}


esp_err_t me_i2c_bus_reset(i2c_port_t port)
{
    esp_err_t err;

    err = check_port(port);

    if( err == ESP_OK )
        err = i2c_master_bus_reset(i2c_bus_handle[port]);

    return err;
}


esp_err_t me_i2c_write(i2c_port_t port, uint8_t addr, uint8_t *data, uint8_t size)
{
    esp_err_t                   err;
    i2c_master_dev_handle_t     dev;

    err = get_i2c_dev(port, addr, &dev);
    if( err != ESP_OK )
        return err;

    dd("I2C Write");

    err = i2c_master_transmit(dev, data, size, CONFIG_ME_I2C_DEF_OP_TIMEOUT_MS);

    return err;
}


esp_err_t me_i2c_read(i2c_port_t port, uint8_t addr, void *data, uint8_t size)
{
    esp_err_t                   err;
    i2c_master_dev_handle_t     dev;

    err = get_i2c_dev(port, addr, &dev);
    if( err != ESP_OK )
        return err;

    dd("I2C Read");

    err = i2c_master_receive(dev, data, size, CONFIG_ME_I2C_DEF_OP_TIMEOUT_MS);

    return err;
}


esp_err_t me_i2c_req_resp(i2c_port_t port, uint8_t addr, uint8_t *req, uint8_t req_size, void *resp, uint8_t resp_size)
{
    esp_err_t                   err;
    i2c_master_dev_handle_t     dev;

    err = get_i2c_dev(port, addr, &dev);
    if( err != ESP_OK )
        return err;

    dd("I2C Req Resp");

    err = i2c_master_transmit_receive(dev,  req, req_size, resp, resp_size, CONFIG_ME_I2C_DEF_OP_TIMEOUT_MS);

    return err;
}

#else
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

#endif

esp_err_t me_i2c_write_reg(i2c_port_t port, uint8_t addr, uint8_t reg, uint8_t val)
{
    uint8_t     req[2] = {reg, val};

    return me_i2c_write(port, addr, req, 2);
}