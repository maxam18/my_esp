/* My ESP real time functions
 * File: me_time.c
 * Started: Sat Jun 19 21:05:23 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */

#include <me_time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

int32_t me_time_boot = 0;