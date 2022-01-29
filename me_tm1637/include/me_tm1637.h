/* My ESP tm1637 component
 * File: me_tm1637.h
 * Started: Thu Nov 25 22:22:48 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * 
 * 4digits, 6digits support
 */

#ifndef _ME_TM1637_H
#define _ME_TM1637_H

#include <inttypes.h>
#include <stdbool.h>
#include <driver/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	TM1637_4SEGS,
	TM1637_6SEGS_DM
} me_tm1637_led_model_t;

typedef struct {
	gpio_num_t 		 m_pin_clk;
	gpio_num_t 		 m_pin_dta;
	const uint8_t   *seq; /*!< sequence of segments */
} me_tm1637_led_t;

#define TM1637_SEG_MAX 9

/**
 * @brief Constructs new LED TM1637 object
 *
 * @param pin_clk GPIO pin for CLK input of LED module
 * @param pin_data GPIO pin for DIO input of LED module
 * @param model led model
 * @return LED object
 */
me_tm1637_led_t *me_tm1637_init(gpio_num_t pin_clk, gpio_num_t pin_data, me_tm1637_led_model_t model);

/**
 * @brief Reset LED TM1637
 *
 * @param led LED object
 */
void me_tm1637_reset(me_tm1637_led_t *led);

/**
 * @brief Set brightness level. Note - will be set after next display render
 * @param led LED object
 * @param level Brightness level 0..7 value
 */
void me_tm1637_set_brightness(me_tm1637_led_t *led, uint8_t level);
//#define me_tm1637_set_brightness(led,level) 	(led)->m_brightness = (level) & 0x07

/**
 * @brief Set one-segment number, also controls dot of this segment
 * @param led LED object
 * @param segment_idx Segment index (0..3)
 * @param ch char to display. Available chars from ASCII '0' to ASCII 'Z'
 * @param dot Display dot of this segment
 */
void me_tm1637_set_segment(me_tm1637_led_t *led, const uint8_t segment_idx, const uint8_t ch, const bool dot);

/**
 * @brief Set full display number, in decimal encoding
 * @param led LED object
 * @param number Display number (0...9999)
 */
#define me_tm1637_set_number(led, number) me_tm1637_set_number_dot(led, number, false, TM1637_SEG_MAX)

/**
 * @brief Set full display number, in decimal encoding + control leading zero
 * @param led LED object
 * @param number Display number (0...9999)
 * @param lead_zero Display leading zero(s)
 */
#define me_tm1637_set_number_lead(led, number, lead_zero) me_tm1637_set_number_dot(led, number, lead_zero, TM1637_SEG_MAX)

/**
 * @brief Set full display number, in decimal encoding + control leading zero + control dot display
 * @param led LED object
 * @param number Display number (0...9999)
 * @param lead_zero Display leading zero(s)
 * @param dot_mask Dot mask, bits left-to-right
 */
void me_tm1637_set_number_dot(me_tm1637_led_t * led, int32_t number, const bool lead_zero, int8_t dot_pos);

/**
 * @brief Set floating point number, correctly handling negative numbers
 * @param led LED object
 * @param n Floating point number
 * @param digs number of digits after a dot (i.e. dot position)
 * @param lead_zero Display leading zero(s)
 */
#define me_tm1637_set_float(led, n, digs, lead_zero) me_tm1637_set_number_dot(led, (int)(n * (10 * digs)), lead_zero, digs)

/**
 * @brief Set text (available chars from ASCII '0' to ASCII 'Z')
 * @param led LED object
 * @param text text to display
 * @param chars number of bytes (chars) @text buffer
 */
void me_tm1637_set_text(me_tm1637_led_t * led, uint8_t *text, int8_t chars);

#ifdef __cplusplus
}
#endif

#endif

