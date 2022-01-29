/* @brief My ESP MCP23x17 IO expander
 * @file me_mcp23x17.h
 * @since Sun Jan 23 17:30:43 MSK 2022
 * @author Max Amzarakov (maxam18 _at_ gmail _._ com)
 * @details MCP23x17 extender GPIO operation functions. 
 *          No interrupts functions realised.
 *          All communications are 16bits. (Both BANKs in one go)
 * Copyright (c) 2022 ..."
 */

#ifndef _ME_MCP23X17_H
#define _ME_MCP23X17_H

#include <driver/i2c.h>

#define ME_MCP23x17_DEF_ADDR        0x20

#define ME_MCP23x17_REG_IODIRA      0x00
#define ME_MCP23x17_REG_IODIRB      0x01
#define ME_MCP23x17_REG_IPOLA       0x02
#define ME_MCP23x17_REG_IPOLB       0x03
#define ME_MCP23x17_REG_GPINTENA    0x04
#define ME_MCP23x17_REG_GPINTENB    0x05
#define ME_MCP23x17_REG_DEFVALA     0x06
#define ME_MCP23x17_REG_DEFVALB     0x07
#define ME_MCP23x17_REG_INTCONA     0x08
#define ME_MCP23x17_REG_INTCONB     0x09
#define ME_MCP23x17_REG_IOCON       0x0A
#define ME_MCP23x17_REG_GPPUA       0x0C
#define ME_MCP23x17_REG_GPPUB       0x0D
#define ME_MCP23x17_REG_INTFA       0x0E
#define ME_MCP23x17_REG_INTFB       0x0F
#define ME_MCP23x17_REG_INTCAPA     0x10
#define ME_MCP23x17_REG_INTCAPB     0x11
#define ME_MCP23x17_REG_GPIOA       0x12
#define ME_MCP23x17_REG_GPIOB       0x13
#define ME_MCP23x17_REG_OLATA       0x14
#define ME_MCP23x17_REG_OLATB       0x15

/**
 * @brief PIN mode
 * 
 */
typedef enum { 
    ME_MCP23x17_MODE_OUTPUT, /*!< OUTPUT pin */
    ME_MCP23x17_MODE_INPUT   /*!< INPUT pin */
} me_mcp23x17_mode_t;

/**
 * @brief MCP23x17 device
 */
typedef struct {
    i2c_port_t          port;     /*!< I2C bus number */
    uint8_t             addr;     /*!< I2C sensor's slave address */
} me_mcp23x17_t;

esp_err_t me_mcp23x17_set_with_mask(me_mcp23x17_t *dev, uint8_t reg, uint16_t mask, uint8_t val);
esp_err_t me_mcp23x17_get(me_mcp23x17_t *dev, uint8_t reg, uint16_t *data);

/**
 * @brief Set pins direction
 *
 * @param dev       MCP23x17 dev
 * @param mask      IO pins mask
 * @param dir       direction (0 - output, 1 - input)
 *
 * @return  esp_err_t
 */
#define me_mcp23x17_set_dir(dev,mask,dir) me_mcp23x17_set_with_mask((dev), ME_MCP23x17_REG_IODIRA, mask, dir)

/**
 * @brief Set pin(s) level
 *
 * @param dev       MCP23x17 dev
 * @param mask      IO pins mask
 * @param level     state: 0 - low, 1 - high
 *
 * @return  esp_err_t
 */
#define me_mcp23x17_set_level(dev, mask, level) me_mcp23x17_set_with_mask((dev), ME_MCP23x17_REG_GPIOA, mask, level)

/**
 * @brief Set pin level
 *
 * @param dev       MCP23x17 dev
 * @param pin       IO pin
 * @param level     state: 0 - low, 1 - high
 *
 * @return  esp_err_t
 */
#define me_mcp23x17_set_pin_level(dev, pin, level) me_mcp23x17_set_with_mask((dev), ME_MCP23x17_REG_GPIOA, (1<<pin), level)

/**
 * @brief Get all pins level
 *
 * @param dev       MCP23x17 dev
 * @param val       IO pins state by postion. 0 - low, 1 - high
 *
 * @return  esp_err_t
 */
#define me_mcp23x17_get_level(dev, val) me_mcp23x17_get((dev), ME_MCP23x17_REG_GPIOA, (val))

/**
 * @brief Get pin level
 *
 * @param dev       MCP23x17 dev
 * @param val       IO pins state by postion. 0 - low, 1 - high
 * @param pin       pin number
 *
 * @return  esp_err_t
 */
esp_err_t me_mcp23x17_get_pin_level(me_mcp23x17_t *dev, uint8_t pin, uint16_t *val);


#endif
