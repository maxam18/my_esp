/* @brief My ESP MCP23x17 IO expander
 * @file me_mcp23x17.c
 * @since Sun Jan 23 17:30:37 MSK 2022
 * @author Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2022 ..."
 */

#include <me_i2c.h>
#include <esp_err.h>

#include "me_mcp23x17.h"

esp_err_t me_mcp23x17_set_with_mask(me_mcp23x17_t *dev, uint8_t reg, uint16_t mask, uint8_t val)
{
    esp_err_t   err;
    uint16_t    u16;
    uint8_t     data[3];

    if( mask != 0xFFFF )
    {
        err = me_i2c_req_resp(dev->port, dev->addr, &reg, 1, (uint8_t *)&u16, 2);
        if( err != ESP_OK )
            return err;

        u16 = (u16 & ~mask) | (mask & (val * 0xFFFF));
    } else
        u16 = mask & (0xFFFF * val);

    data[0] = reg;
    data[1] = u16;
    data[2] = u16 >> 8;

    return me_i2c_write(dev->port, dev->addr, data, 3);
}


esp_err_t me_mcp23x17_get(me_mcp23x17_t *dev, uint8_t reg, uint16_t *data)
{
    return me_i2c_req_resp(dev->port, dev->addr, &reg, 1, (uint8_t *)data, 2);
}


esp_err_t me_mcp23x17_get_pin_level(me_mcp23x17_t *dev, uint8_t pin, uint16_t *val)
{
    esp_err_t   err;
    uint16_t    u16;

    err = me_mcp23x17_get(dev, ME_MCP23x17_REG_GPIOA, &u16);
    if( err != ESP_OK )
        return err;

    *val = u16 & (1U<<pin);

    return err;
}

/*
esp_err_t me_mcp23x17_set_dir(me_mcp23x17_t *dev, uint16_t mask, me_mcp23x17_mode_t dir)
{
    uint8_t data[3] = { ME_MCP23x17_REG_IODIRA, mask & 0xFF, mask >> 8 };

    if( dir == 0 )
        data[1] = data[2] = 0;

    return me_i2c_write(dev->port, dev->addr, data, 3);
}


esp_err_t me_mcp23x17_set_level(me_mcp23x17_t *dev, uint16_t mask, uint8_t level)
{
    uint8_t data[3] = { ME_MCP23x17_REG_GPIOA, mask & 0xFF, mask >> 8 };

    if( level == 0 )
        data[1] = data[2] = 0;

    return me_i2c_write(dev->port, dev->addr, data, 3);
}
*/


