/* MQ4 methane sensor functions
 * File: me_mq4.h
 * Started: Fri 20 Nov 2020 11:55:15 AM MSK
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2020 ..."
 */

#ifndef _ME_MQ4_H
#define _ME_MQ4_H

/**
 * @brief MQ series sensor calibration parameters
 */

typedef struct {
    double      mV_ref;
    double      divider; /* (Vref/V@Ro - 1) */
} me_mq4_t;

/**
 * @brief Read MQ4 sensor concentration 
 *
 * @param milliV  sensor output
 * @param calib   pointer to sensor calibration data
 *
 * @return
 *     - concentration in PPM
 */
double me_mq4_ppm(double milliV, me_mq4_t *calib);

/**
 * @brief Read MQ4 sensor concentration compensation
 *
 * @param milliV    sensor output
 * @param calib     pointer to sensor calibration data
 * @param temp      measured temperature in Celsium
 * @param humidity  measured relative humidity in % (two decimals value)
 *
 * @return
 *     - concentration in PPM
 */
double me_mq4_ppm_compensated(double milliV, me_mq4_t *calib, double temp, double rh);

/**
 * @brief Calibrate MQ4 sensor
 *
 * @param refV    reference voltage to the sensor measurement pins
 * @param measV   sensor output
 * @param calib   pointer to sensor calibration data
 *
 * @return
 *     - no return
 */
void me_mq4_calibrate(double refV, double measV, me_mq4_t *calib);

/**
 * @brief Calibrate MQ4 sensor with temperature and humidity compensation
 *
 * @param refV      reference voltage to the sensor measurement pins
 * @param measV     sensor output
 * @param calib     pointer to sensor calibration data
 * @param temp      measured temperature in Celsium
 * @param humidity  measured relative humidity in % (two decimals value)
 *
 * @return
 *     - no return
 */
void me_mq4_calibrate_compensated(double refV, double measV, me_mq4_t *calib, double temp, double rh);


#endif

