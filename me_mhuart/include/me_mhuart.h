/* MH-Z19 CO2 sensor UART mode
 * File: mh_z19.h
 * Started: Wed 27 Nov 2019 02:26:07 PM MSK
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2019 ..."
 */

#ifndef _MH_MHUART_H
#define _MH_MHUART_H

#include "driver/uart.h"

#include "sdkconfig.h"

#define MH_UART_BUF_SIZE        9*10
/**
 * @brief MH series sensor short int concentration read command
 *        this is for the sensors of 0~32000 range
 */
#define MH_CMD_READ_SHORT       0x86
/**
 * @brief MH series sensor long int concentration read command 
 *        this is for the sensors of 32000+ range
 */
#define MH_CMD_READ_LONG        0x9C
/**
 * @brief MH series sensor Q&A mode
 *        request/response mode
 */
#define MH_HCL_CMODE_QA         0x04
#define MH_CH2O_CMODE_QA        0x41
/**
 * @brief MH series sensor active send mode
 *        sensor sends readings repeatedly
 */
#define MH_HCL_CMODE_ACTIVE     0x03
#define MH_CH2O_CMODE_ACTIVE    0x40

/**
 * @brief Read Winsen-Sensor MH series (UART) sensor concentration 
 *
 * @param uart_num UART number
 * @param cmd      Sensor specific command to read concentration
 *
 * @return
 *     - concentration on success
 *     - -1 on failure
 */
int me_mhuart_read_concentration(int uart_num, uint8_t cmd);

/**
 * @brief Winsen-Sensor MH series (UART) sensor auto calibration 
 *        MH series sensors has auto calibration feature. It assumes
 *        the sensor will be exposed to a gas of zero concentration 
 *        during a certain period of time. 
 *        The micro logic of the sensor keeps track of minimum read value
 *        and provides self-calibration some times.
 *        Turn this off if not sure.
 *
 * @param uart_num UART number
 * @param is_on    should the self-calibration be set on (1) or off (0)
 *
 * @return
 *     esp_err_t
 */
esp_err_t me_mhuart_auto_calibration(int uart_num, int is_on);

/**
 * @brief Calibrate Winsen-Sensor MH series (UART) sensor at certain level
 *        Expose the sensor at cetain concentration for at about 20 minutes
 *        before the command.
 *        zero point calibration if ppm is set to 0
 *        span point calibration if pmm is set to non zero
 *
 * @param uart_num UART number
 * @param ppm      concentration in PPM
 *
 * @return
 *     esp_err_t
 */
esp_err_t me_mhuart_calibrate(int uart_num, int ppm);

/**
 * @brief Set Winsen-Sensor MH series (UART) sensor to Q&A mode
 *        sensor is always sends data by default (active mode)
 *
 * @param uart_num UART number
 * @param mode UART number
 *
 * @return
 *     esp_err_t
 */
esp_err_t me_mhuart_set_mode(int uart_num, uint8_t mode);

#endif

