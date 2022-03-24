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

#ifdef CONFIG_ME_NVS_STORE_NAME
static const char *nvs_name = CONFIG_ME_NVS_STORE_NAME;
#else
static const char *nvs_name = "my_esp";
#endif

static void read_calibration(me_asensor_t *sensor)
{
    nvs_handle  store;
    esp_err_t   err;
    size_t      store_sz = 0;

    err = nvs_open(nvs_name, NVS_READONLY, &store);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "Warning (%s) opening NVS handle for %s. Using defaults."
                    , esp_err_to_name(err)
                    , sensor->name);
        return;
    }

    err = nvs_get_blob(store, sensor->name, NULL, &store_sz);

    if( (err == ESP_OK) 
        && (store_sz == sizeof(me_asensor_span_t) * sensor->length) )
    {
        err = nvs_get_blob(store, sensor->name, sensor->points, &store_sz);
        switch (err) {
            case ESP_OK:
                ESP_LOGI(TAG, "Calibration data of %s read from storage"
                            , sensor->name);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGW(TAG, "No calibration data of %s found in store. Using defaults."
                            , sensor->name);
                break;
            default :
                ESP_LOGE(TAG, "Error (%s) reading store for %s!"
                            , esp_err_to_name(err)
                            , sensor->name);
                break;
        }
    } else
        ESP_LOGE(TAG, "Cannot open calibration store for %s or different size: %s"
                    , sensor->name
                    , esp_err_to_name(err));

    nvs_close(store);
}

static void write_calibration(me_asensor_t *sensor)
{
    nvs_handle  store;
    esp_err_t   err;
    size_t      store_sz = sizeof(me_asensor_span_t) * sensor->length;

    err = nvs_open(nvs_name, NVS_READWRITE, &store);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle for sensor %s!"
                    , esp_err_to_name(err)
                    , sensor->name);
        return;
    }

    err = nvs_set_blob(store, sensor->name, sensor->points, store_sz);
    if (err ==  ESP_OK ) {
        ESP_LOGI(TAG, "Calibration data of %s written to the storage"
                    , sensor->name);
        nvs_commit(store);
    }
    else
        ESP_LOGE(TAG, "Error writting %s calibration to storage (%s)!"
                    , sensor->name
                    , esp_err_to_name(err));

    nvs_close(store);
}

void me_asensor_clear(me_asensor_t *sensor)
{
    me_asensor_span_t   *span;
    uint8_t              len;

    bzero(sensor->points, sizeof(me_asensor_span_t) * sensor->length);

    span = sensor->points;
    len  = sensor->length;
    while( len-- )
        (span++)->x = ME_ASENSOR_X_MIN;

    write_calibration(sensor);
}

double me_asensor_value(me_asensor_t *sensor, int16_t x)
{
    me_asensor_span_t   *span = sensor->points;
    uint8_t              len  = sensor->length;

    while( len-- ) {
        if( x > span->x )
            return x * span->k + span->c;
        span++;
    }

    return 0;
}

static void reset_kc(me_asensor_span_t *from, me_asensor_span_t *to)
{
    double           div;

    div = (from->x - to->x);
    if( !div )
        return;

    to->k = (from->y - to->y) / div;
    to->c = to->y - to->k * to->x;
}

esp_err_t me_asensor_add(me_asensor_t *sensor, me_asensor_span_t *span)
{
    int8_t              len;
    me_asensor_span_t  *lp, *rp, *swap;

    len    = sensor->length - 1;
    
    lp = sensor->points;
    for(; len; lp++, len-- )
        if( span->y >= lp->y )
            break;

    if( len == 0 )
        return ESP_FAIL;

    if( span->y == lp->y )
    {
        memcpy(lp, span, sizeof(me_asensor_span_t));
    } else {
        rp = sensor->points + sensor->length - 1;
        while( rp != lp )
        {
            swap = rp--;
            memcpy(swap, rp, sizeof(me_asensor_span_t));
        }

        memcpy(rp, span, sizeof(me_asensor_span_t));
    }

    /* reset bottom and all */
    len = sensor->length - 1;
    lp  = sensor->points;
    rp  = lp + 1;

    while( len-- )
        if( rp->x == ME_ASENSOR_X_MIN )
        {
            rp->k = lp->k;
            rp->c = lp->c;
            lp++;
            break;
        }
        else
            reset_kc(lp++, rp++);
    lp->x = ME_ASENSOR_X_MIN;

    /* reset top */
    rp = sensor->points + 1;
    sensor->points->k = rp->k;
    sensor->points->c = rp->c;

    write_calibration(sensor);

    return ESP_OK;
}

me_asensor_t *me_asensor_init(const char *name, uint8_t length, me_asensor_t *sb)
{
    me_asensor_t       *sensor;
    me_asensor_span_t  *span;
    void               *nb, *pb;

    if( name == NULL || length < 2 )
        return NULL;

    if( !sb ) {
        sb = malloc(sizeof(me_asensor_t));
        nb = malloc(strlen(name) + 1);
        pb = malloc(sizeof(me_asensor_span_t) * length);
        if( sb == NULL || nb == NULL || pb == NULL )
        {
            ESP_LOGE(TAG, "Memory allocation for asensor");
            return NULL;
        }
        sensor         = sb;
        sensor->name   = nb;
        sensor->points = pb;
        sensor->length = length;
        strcpy(sensor->name, name);
    } else
        sensor = sb;

    bzero(sensor->points, sizeof(me_asensor_span_t) * sensor->length);
    
    span = sensor->points;
    while( length-- )
        (span++)->x = ME_ASENSOR_X_MIN;

    read_calibration(sensor);

    return sensor;
}
