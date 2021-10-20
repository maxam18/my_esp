/* SSD1306 SH1106 OLED
 * File: me_ssd1306.h
 * Started: Sun 17 Nov 2019 01:17:51 PM MSK
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2019 ..."
 *
 * Based on https://github.com/yanbe/ssd1306-esp-idf-i2c.git
 */

#ifndef _ME_SSD1306_H
#define _ME_SSD1306_H

#include <me_i2c.h>

#define ME_SSD1306_I2C_ADDRESS_PRIMARY  0x3C

typedef enum {
      ME_SSD1306_SCROLL_OFF
    , ME_SSD1306_SCROLL_LEFT
    , ME_SSD1306_SCROLL_RIGHT
    , ME_SSD1306_SCROLL_UP
    , ME_SSD1306_SCROLL_DOWN
} me_ssd1306_scroll_t;

/**
 * @brief SSD1306 configuration 
 */
typedef struct {
    i2c_port_t          port;     /*!< I2C bus number */
    uint8_t             addr;     /*!< I2C sensor's slave address */
    enum { ME_SSD1306_H64
         , ME_SSD1306_H32 }
                        height;   /*!< OLED size (height) */
    uint8_t             _rows;    /*!< max rows - set by ssd1306_init */
    enum { ME_SSD1306_FLIP_NO
         , ME_SSD1306_FLIP }
                        flip;     /*!< OLED flip */
} me_ssd1306_conf_t;

/**
 * @brief SSD1306 OLED initialization
 *        me_i2c MUST be initialized before
 *
 * @param conf    me_ssd1306_conf_t pointer
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_ssd1306_init(me_ssd1306_conf_t *conf);

/**
 * @brief Clear SSD1306 OLED
 *
 * @param conf    me_ssd1306_conf_t pointer
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_ssd1306_display_clear(me_ssd1306_conf_t *conf);

/**
 * @brief Set SSD1306 OLED contrast
 *
 * @param conf        me_ssd1306_conf_t pointer
 * @param contrast    display contrast (0 - 255)
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_ssd1306_contrast(me_ssd1306_conf_t *conf, int contrast);

/**
 * @brief Scroll SSD1306 OLED
 *        Uses hardware scroll functions
 *
 * @param conf    me_ssd1306_conf_t pointer
 * @param dir     direction
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_ssd1306_scroll(me_ssd1306_conf_t *conf, me_ssd1306_scroll_t dir);

/**
 * @brief Print one line of characters of 8x8 pixels size on SSD1306 OLED
 *
 * @param conf    me_ssd1306_conf_t pointer
 * @param str     characters to display (NULL terminated)
 * @param row     row number to print characters
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_INVALID_ARG row or column is out of range
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_ssd1306_8x8_string(me_ssd1306_conf_t *conf, char *str, int row);

/**
 * @brief Print lines of characters of 8x8 pixels size on SSD1306 OLED
 *
 * @param conf    me_ssd1306_conf_t pointer
 * @param text    lines of characters to display 
 *                separated by '\n'
 *                NULL terminated buffer must be supplied
 *
 * @return
 *     - ESP_OK     Success
 *     - ESP_ERR_INVALID_ARG row or column is out of range
 *     - ESP_ERR_*  returned by me_i2c_read
 */
esp_err_t me_ssd1306_8x8_text(me_ssd1306_conf_t *conf, const char *text);

#endif
