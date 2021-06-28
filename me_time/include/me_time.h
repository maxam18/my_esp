/* My ESP real time functions
 * File: me_time.h
 * Started: Sat Jun 19 21:05:29 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */
#include <stdint.h>

#ifndef _ME_TIME_H
#define _ME_TIME_H

/**
 * @brief Time in seconds of controller boot
 */
extern int32_t me_time_boot;

/**
 * @brief Set time of controller boot (seconds)
 *        Execute this macro just after the controller boot
 *        The realtime (sec) could come from MQTT or HTTP request
 *        or a realtime clock.
 * 
 * @param sec  UNIX epoch timestamp of conroller boot  
 */
#define me_time_set_seconds(sec)    me_time_boot = sec

/**
 * @brief get UNIX epoch timestamp
 * 
 * @return
 *      UNIX epoch current timestamp
 */
#define me_time_get_seconds()       ((xTaskGetTickCount() / configTICK_RATE_HZ) + me_time_boot)

#endif
