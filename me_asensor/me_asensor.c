/* My ESP adc sensor with calibration
 * File: me_adcs.c
 * Started: Tue Oct 19 11:05:36 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */
#include <stdlib.h>
#include <string.h>
#include <me_asensor.h>

#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char *TAG = "me_asensor";

static void read_calibration(me_asensor_t *sensor)
{
    nvs_handle  store;
    esp_err_t   err;
    size_t      store_sz;

    err = nvs_open("my_esp", NVS_READONLY, &store);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "Warning (%s) opening NVS handle. Using defaults.", esp_err_to_name(err));
        return;
    }

    err = nvs_get_blob(store, sensor->name, NULL, &store_sz);

    if( (err == ESP_OK) 
        && (store_sz == sizeof(me_asensor_span_t) * sensor->length) )
    {
        err = nvs_get_blob(store, sensor->name, sensor->points, &store_sz);
        switch (err) {
            case ESP_OK:
                ESP_LOGI(TAG, "Calibration data read from storage");
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGW(TAG, "No calibration data found in store. Using defaults.");
                break;
            default :
                ESP_LOGE(TAG, "Error (%s) reading store!", esp_err_to_name(err));
                break;
        }
    } else
        ESP_LOGE(TAG, "Size is not equal if Ok: %s", esp_err_to_name(err));

    nvs_close(store);
}

static void write_calibration(me_asensor_t *sensor)
{
    nvs_handle  store;
    esp_err_t   err;
    size_t      store_sz = sizeof(me_asensor_span_t) * sensor->length;

    err = nvs_open("my_esp", NVS_READWRITE, &store);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return;
    }

    err = nvs_set_blob(store, sensor->name, sensor->points, store_sz);
    if (err ==  ESP_OK ) {
        ESP_LOGI(TAG, "Calibration data written to the storage");
        nvs_commit(store);
    }
    else
        ESP_LOGE(TAG, "Error writting to storage (%s)!", esp_err_to_name(err));

    nvs_close(store);
}

me_asensor_t *me_asensor_init(const char *name, uint8_t length, me_asensor_t *buf)
{
    me_asensor_t   *sensor;

    if( !buf ) {
        sensor = malloc(sizeof(me_asensor_t));
        sensor->name = malloc(strlen(name)+1);
        sensor->points = malloc(sizeof(me_asensor_span_t)*length);
        sensor->length = length;
    } else
        sensor = buf;

    bzero(sensor->points, sizeof(me_asensor_span_t)*sensor->length);
    
    read_calibration(sensor);

    return sensor;
}

void me_asensor_clear(me_asensor_t *sensor)
{
    bzero(sensor->points, sizeof(me_asensor_span_t)*sensor->length);

    write_calibration(sensor);
}

double me_asensor_value(me_asensor_t *sensor, int16_t x)
{
    int               i = sensor->length;
    me_asensor_span_t   *point = sensor->points;

    for(; i > 0; i--, point++ )
        if( x > point->x )
            return x * point->k + point->c;

    return 0;
}

esp_err_t me_asensor_add(me_asensor_t *sensor, me_asensor_span_t *span)
{
    size_t           len;
    uint8_t          i;
    double           div;
    me_asensor_span_t  *spanp, *spanr, *points;

    len = sizeof(me_asensor_span_t) * (sensor->length + 1);
    points = malloc(len);
    memcpy( points + 1, sensor->points, len);

    for(    i = sensor->length - 1
            , spanp = points + 1
            , spanr = points
        ;   i > 0
        ;   i--
            , spanp++
            , spanr++)
    {
        if( span->y > spanp->y )
            break;
        memcpy(spanr, spanp, sizeof(me_asensor_span_t));
    }

    memcpy(spanr, span, sizeof(me_asensor_span_t));
    
    if( span->y == spanp->y )
        for( span = spanp; i-- > 0; span++ )
            memcpy(span, span+1, sizeof(me_asensor_span_t));

    span = spanr - 1;
    if( span >= points )
    {
        div = (span->x - spanr->x);
        if( !div )
        {
            free(points);
            return ESP_FAIL;
        }
        spanr->k = (span->y - spanr->y) / div;
        spanr->c = spanr->y - spanr->k * spanr->x;
    } else
    {
        spanr->k = 0;
        spanr->c = spanr->y;
    }

    span = spanr + 1;
    if( spanr < points + sensor->length - 1 )
    {
        div = (spanp->x - spanr->x);
        if( !div ) {
            free(points);
            return ESP_FAIL;
        }
        spanp->k = (spanr->y - spanp->y) / div;
        spanp->c = spanp->y - spanp->k * spanp->x;
    } else
    {
        spanp->k = 0;
        spanp->c = spanp->y;
    }

    memcpy(sensor->points, points, len);
    free(points);

    write_calibration(sensor);

    return ESP_OK;
}


