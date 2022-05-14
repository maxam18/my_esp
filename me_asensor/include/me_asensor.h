/* My ESP analog sensor with calibration
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
#define ME_ASENSOR_UNDEF    INT32_MIN

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

/**
 * @brief Initialize asensor
 * 
 * @param name sensor name to keep in the nvs store
 * @param length maximum amount of calibration points
 * @param sensor asensor structure pointer.
 * @return me_asensor_t*  Memory allocated internally if the *sensor is NULL
 */
me_asensor_t *me_asensor_init(const char *name, uint8_t length, me_asensor_t *sensor);
/**
 * @brief Deallocate asensor structure
 */
#define me_asensor_free(S) { free((S)->points); free((S)->name); free(S); }

/**
 * @brief converts analog sensor readings according to a span points
 * 
 * @param sensor asensor pointer. Structure MUST be initialized
 * @param x analog readings
 * @return double converted value. 0 returned in the case of empty span points
 */
double me_asensor_value(me_asensor_t *sensor, int16_t x);

/**
 * @brief clears span points array
 * 
 * @param sensor asensor pointer. Structure MUST be initialized
 */
void me_asensor_clear(me_asensor_t *sensor);

/**
 * @brief Add span point
 * 
 * @param sensor asensor pointer. Structure MUST be initialized
 * @param span span pointer. Structure MUST be initialized. x and y MUST be set.
 * @return esp_err_t 
 */
esp_err_t me_asensor_add(me_asensor_t *sensor, me_asensor_span_t *span);

#ifdef CONFIG_ME_DEBUG
/**
 * @brief dump asensor data
 * 
 * @param sensor asensor pointer. Structure MUST be initialized
 */
void me_asensor_dump(me_asensor_t *sensor);
#else
#define me_asensor_dump(X)
#endif

#endif

