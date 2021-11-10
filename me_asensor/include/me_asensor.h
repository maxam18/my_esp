/* My ESP adc sensor with calibration
 * File: me_adcs.h
 * Started: Tue Oct 19 11:05:32 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */
#include <stdint.h>
#include <esp_err.h>

#ifndef _ME_ANALOG_SENSOR_H
#define _ME_ANALOG_SENSOR_H

#define ME_ASENSOR_X_MIN    INT16_MIN

typedef struct {
    int16_t     x;
    double      y;
    double      k;
    double      c;
    double      humidity;
    double      temperature;
    int32_t     ts;
} me_asensor_span_t;

typedef struct {
    uint8_t               length;
    char                 *name;
    me_asensor_span_t    *points;
} me_asensor_t;

me_asensor_t *me_asensor_init(const char *name, uint8_t length, me_asensor_t *sensor);
#define me_asensor_free(S) { free((S)->points); free((S)->name); free(S); }
double me_asensor_value(me_asensor_t *sensor, int16_t x);
void me_asensor_clear(me_asensor_t *sensor);
esp_err_t me_asensor_add(me_asensor_t *sensor, me_asensor_span_t *span);

#endif

