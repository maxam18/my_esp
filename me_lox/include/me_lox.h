/* My ESP Luminox sensor reader
 * File: me_lox.h
 * Started: Thu Apr  7 16:08:06 MSK 2022
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2022 ..."
 */

#ifndef _ME_LOX_H
#define _ME_LOX_H

#include <esp_err.h>

/*
O xxxx.x T yxx.x P xxxx % xxx.xx e xxxx\r\n
*/
typedef struct me_lox_s {
    uint16_t        ppressure;      /*!< partial pressure mBar x10 */
    uint16_t        temperature;    /*!< temperature oC x10 */
    uint16_t        pressure;       /*!< pressure mBar */
    uint16_t        value;          /*!< concentration %vol x10 */
    uint16_t        error;          /*!< sensor error code */
} me_lox_t;

/**
 * \brief  Read concentration in streaming mode
 * \param  uart_num UART number sensor connected to
 * \param  r result in me_lox_t structure
 * \return esp_err_t codes
 */
esp_err_t me_lox_read(int uart_num, me_lox_t *r);

/**
 * \brief  initializes uart and installs driver
 * \param  uart_num UART number sensor connected to
 * \param  tx TX pin number
 * \param  rx RX pin number
 * \return esp_err_t codes
 */
esp_err_t me_lox_init(int uart_num, int tx, int rx);

#endif