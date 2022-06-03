/* ADS111x ADC
 * File: me_ads111x.h
 * Started: Tue Jun 15 12:28:30 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */

#ifndef _ME_ADS111X_H
#define _ME_ADS111X_H

#include <sys/types.h>
#include <me_i2c.h>

#include "me_ads111x_defs.h"

/**
 * @brief Slave addresses
 */
#define ME_ADS111x_ADDR_GND   0x48 /* ADDR to GND 1001000b */
#define ME_ADS111x_ADDR_VDD   0x49 /* ADDR to VDD 1001001b */
#define ME_ADS111x_ADDR_SDA   0x4A /* ADDR to SDA 1001010b */
#define ME_ADS111x_ADDR_SCL   0x4B /* ADDR to SCL 1001011b */
#define ME_ADS111x_ADDR       ME_ADS111x_ADDR_GND /* Default */

/**
 * @brief Registers 
 */
#define ME_ADS111x_REG_CONV   0x00 /* 0b0000 0000 */
#define ME_ADS111x_REG_CONF   0x01 /* 0b0000 0001 */
#define ME_ADS111x_REG_LOW    0x02 /* 0b0000 0010 */
#define ME_ADS111x_REG_HIGH   0x03 /* 0b0000 0011 */

#define ME_ADS111x_CMD_RESET  0x06

/**
 * @brief ADS111x chip models 
 */
typedef enum {  ME_ADS111x_CHIP_1115
              , ME_ADS111x_CHIP_1114
              , ME_ADS111x_CHIP_1113 } me_ads111x_chip_t;

/**
 * @brief ADS111x configuration 
 */
typedef struct 
{
    me_ads111x_chip_t   chip; /*!< ADS chip. Only 1115 has mux and pge */
    i2c_port_t          port; /*!< I2C port ADS connected to */
    uint8_t             addr; /*!< i2c slave address of ADS. see `ME_ADS111x_ADDR_...` */
    uint8_t             reg;  /*!< latest accessed register */
    uint16_t            cur;  /*!< current configuration set */
    uint16_t            set;  /*!< requrested configuration set */
} me_ads111x_conf_t;

/**
 * @brief Sets various ADS configuration paramters. Macro handy tool
 *
 * @param conf    ads111x_conf_t pointer
 * @param FIELD   configuration field one of 
 *      [ OS, MUX, PGA, MODE, DR, COMP_MODE, COMP_POL, COMP_LAT, COMP_QUE ]
 * @param VAL     configuration field value 
 *      [ ...
 *        MUX - multplexor:
 *          A0_1 -  AIN0 to AIN1 differential input,
 *          A[0-2]-3 - AIN0 | AIN1 | AIN2 to AIN3 differential input,
 *          A[0-3]-G - single ended input AINx to GND ], 
 *        PGA - measurement range:
 *          6114, 4096, etc...
 *        look into `me_ads111x_defs.h` for more details
 *      ]
 */
#define me_ads111x_set_conf(conf, FIELD, VAL) \
        (conf)->set = \
            ((conf)->set & ME_ADS111x_CONF_ ## FIELD ## _MASK ) \
             | ME_ADS111x_CONF_ ## FIELD ## _ ## VAL

/**
 * @brief ADS111x default configuration
 */
#define me_ads111x_set_conf_default(c)                  \
            (c)->chip = ME_ADS111x_CHIP_1115;           \
            (c)->port = I2C_NUM_0;                      \
            (c)->addr = ME_ADS111x_ADDR_GND;            \
            (c)->reg  = 0;                              \
            (c)->cur  = 0;                              \
            (c)->set  = ME_ADS111x_CONF_OS_SNGL |       \
                        ME_ADS111x_CONF_MUX_A0_G |      \
                        ME_ADS111x_CONF_PGA_4096 |      \
                        ME_ADS111x_CONF_RATE_64 |       \
                        ME_ADS111x_CONF_MODE_CONT |     \
                        ME_ADS111x_CONF_COMP_MODE_DEF | \
                        ME_ADS111x_CONF_COMP_POL_LOW |  \
                        ME_ADS111x_CONF_COMP_LAT_NON |  \
                        ME_ADS111x_CONF_COMP_QUE_DIS

#define ME_ADS111x_INIT_CONF_CONT                       \
            .chip = ME_ADS111x_CHIP_1115,               \
            .port = I2C_NUM_0,                          \
            .addr = ME_ADS111x_ADDR,                    \
            .reg  = 0,                                  \
            .cur  = 0,                                  \
            .set  = ME_ADS111x_CONF_OS_CONT |           \
                    ME_ADS111x_CONF_MUX_A0_G |          \
                    ME_ADS111x_CONF_PGA_4096 |          \
                    ME_ADS111x_CONF_RATE_64 |           \
                    ME_ADS111x_CONF_MODE_CONT |         \
                    ME_ADS111x_CONF_COMP_MODE_DEF |     \
                    ME_ADS111x_CONF_COMP_POL_LOW |      \
                    ME_ADS111x_CONF_COMP_LAT_NON |      \
                    ME_ADS111x_CONF_COMP_QUE_DIS        

#define ME_ADS111x_INIT_CONF_SIGNLE                    \
            .chip = ME_ADS111x_CHIP_1115,               \
            .port = I2C_NUM_0,                          \
            .addr = ME_ADS111x_ADDR,                    \
            .reg  = 0,                                  \
            .cur  = 0,                                  \
            .set  = ME_ADS111x_CONF_OS_SNGL |           \
                    ME_ADS111x_CONF_MUX_A0_G |          \
                    ME_ADS111x_CONF_PGA_4096 |          \
                    ME_ADS111x_CONF_RATE_16 |           \
                    ME_ADS111x_CONF_MODE_SNGL |         \
                    ME_ADS111x_CONF_COMP_MODE_DEF |     \
                    ME_ADS111x_CONF_COMP_POL_LOW |      \
                    ME_ADS111x_CONF_COMP_LAT_NON |      \
                    ME_ADS111x_CONF_COMP_QUE_DIS        
/**
 * @brief ADS111x initialization
 *        Resets device. 
 *        Initializes device into continous readings
 *
 * @param conf    ads111x_conf_t pointer
 *              conf MUST be initialized with chip, addr and port
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_ads111x_init(me_ads111x_conf_t *conf);

/**
 * @brief ADS111x read raw
 *        Read raw value for specified port . 
 *        Initializes device into continous readings
 *
 * @param conf    ads111x_conf_t pointer
 * @param raw     raw readings buffer
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_ads111x_get_raw(me_ads111x_conf_t *conf, int16_t *raw);

/**
 * @brief ADS1115 read raw
 *        Read raw value for specified port . 
 *        Initializes device into continous readings
 *
 * @param conf    ads111x_conf_t pointer
 * @param pga     gain for measurements (one of ME_ADS111x_CONF_PGA)
 * @param mux     multiplexor configuration (one of ME_ADS111x_CONF_MUX)
 * @param raw     raw readings buffer
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_ads111x_get_raw_ads1115(me_ads111x_conf_t *conf, int16_t *raw, uint16_t pga, uint16_t mux);

/**
 * @brief ADS111x read value
 *        Read raw value for specified port . 
 *        Initializes device into continous readings
 *
 * @param conf    ads111x_conf_t pointer
 * @param pga     gain for measurements (one of ME_ADS111x_CONF_PGA)
 * @param mux     multiplexor configuration (one of ME_ADS111x_CONF_MUX)
 * @param value   readings buffer (milliVolts)
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_ads111x_get_value(me_ads111x_conf_t *conf, double *value);

/**
 * @brief ADS1115 read value
 *        Read raw value for specified port . 
 *        Initializes device into continous readings
 *
 * @param conf    ads111x_conf_t pointer
 * @param pga     gain for measurements (one of ME_ADS111x_CONF_PGA)
 * @param mux     multiplexor configuration (one of ME_ADS111x_CONF_MUX)
 * @param value   readings buffer (milliVolts)
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_ads111x_get_value_ads1115(me_ads111x_conf_t *conf, double *value, uint16_t pga, uint16_t mux);

#endif

