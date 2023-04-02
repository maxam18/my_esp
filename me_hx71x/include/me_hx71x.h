/* My ESP HX710B adc
 * File: me_hx71x.h
 * Started: Sat Mar 25 22:25:14 MSK 2023
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2023 ..."
 */

#ifndef _ME_HX71X_H
#define _ME_HX71X_H

#include <driver/gpio.h>

typedef struct {
    int      clk;
    int      dta;
} me_hx71x_t;


esp_err_t me_hx71x_init(me_hx71x_t **hx, int pin_clk, int pin_dta);
esp_err_t me_hx71x_read(me_hx71x_t *hx, int32_t *retval);

#endif

