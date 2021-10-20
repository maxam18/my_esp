/* SH1106 OLED
 * File: me_sh1106.h
 * Started: Sun 17 Nov 2019 01:17:51 PM MSK
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2019 ..."
 *
 */

#ifndef _ME_SH1106_H
#define _ME_SH1106_H

#include <me_i2c.h>

#define ME_SH1106_I2C_ADDRESS_PRIMARY  0x3C

typedef enum {
      ME_SH1106_SCROLL_OFF
    , ME_SH1106_SCROLL_LEFT
    , ME_SH1106_SCROLL_RIGHT
    , ME_SH1106_SCROLL_UP
    , ME_SH1106_SCROLL_DOWN
} me_sh1106_scroll_t;

/**
 * @brief SH1106 configuration 
 */
typedef struct {
    i2c_port_t          port;     /*!< I2C bus number */
    uint8_t             addr;     /*!< I2C sensor's slave address */
    enum { ME_SH1106_FLIP_NO
         , ME_SH1106_FLIP }
                        flip;     /*!< OLED flip */
} me_sh1106_conf_t;

/**
 * @brief SH1106 OLED initialization
 *        me_i2c MUST be initialized before
 *
 * @param conf    me_sh1106_conf_t pointer
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_sh1106_init(me_sh1106_conf_t *conf);

/**
 * @brief Clear SH1106 OLED
 *
 * @param conf    me_sh1106_conf_t pointer
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_sh1106_display_clear(me_sh1106_conf_t *conf);

/**
 * @brief Set SH1106 OLED contrast
 *
 * @param conf        me_sh1106_conf_t pointer
 * @param contrast    display contrast (0 - 255)
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_sh1106_contrast(me_sh1106_conf_t *conf, int contrast);

/**
 * @brief Scroll SH1106 OLED
 *        Uses hardware scroll functions
 *
 * @param conf    me_sh1106_conf_t pointer
 * @param dir     direction
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_sh1106_scroll(me_sh1106_conf_t *conf, me_sh1106_scroll_t dir);

/**
 * @brief Print one line of characters of 8x8 pixels size on SH1106 OLED
 *
 * @param conf    me_sh1106_conf_t pointer
 * @param str     characters to display (NULL terminated)
 * @param row     row number to print characters
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_INVALID_ARG row or column is out of range
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_sh1106_8x8_string(me_sh1106_conf_t *conf, char *str, int row);

#endif
