/* My ESP HMI
 * File: me_diface.h
 * Started: Wed Aug 30 20:24:36 MSK 2023
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2023 ..."
 */

#ifndef _ME_DIFACE_H
#define _ME_DIFACE_H

#include <stdint.h>
#include <me_str.h>

#include "sdkconfig.h"

#define ME_DIFACE_ITEM_FLAG_IDLE     0x01 /* show at idle state useless? */
#define ME_DIFACE_ITEM_FLAG_SP       0x02 /* set setpoint */
#define ME_DIFACE_ITEM_FLAG_SET      0x04
#define ME_DIFACE_ITEM_FLAG_CONFIRM  0x08 /* confirm set action */
//#define ME_DIFACE_ITEM_FLAG_VALUE    0x02 /* show value */

#define ME_DIFACE_REFRESH_DELAY pdMS_TO_TICKS(CONFIG_ME_DIFACE_REFRESH_DELAY)
#define ME_DIFACE_SET_DELAY pdMS_TO_TICKS(CONFIG_ME_DIFACE_SET_DELAY)



typedef struct me_diface_value_range_s
{
    double             min;
    double             max;
    double             step;
    double             stepk;
} me_diface_range_t;

typedef struct me_diface_item_s me_diface_item_t;

struct me_diface_item_s {
    me_str_t           *name;
    uint8_t             flags;
    uint8_t             vshift;
    uint8_t             nshift;

    double              value;
    me_diface_range_t  *range;

    esp_err_t           (*set)(me_diface_item_t *item);

    me_diface_item_t   *items;
    me_diface_item_t   *next;
    me_diface_item_t   *prev;
};

esp_err_t me_diface_init(me_diface_item_t *root, me_diface_item_t *second);

#endif

