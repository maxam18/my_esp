/* My ESP GoodDisplay E-ink models
 * File: me_gd_models.h
 * Started: Sun Oct  1 22:04:39 MSK 2023
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2023 ..."
 */

#ifndef _ME_GD_MODELS_H
#define _ME_GD_MODELS_H

#include <stdint.h>

typedef enum {
    ME_GD_MODEL_0213F51,
    ME_GD_MODEL_029F51,
    ME_GD_MODEL_0213B74,
    ME_GD_MODEL_029T94
} me_gd_model_enum_t;

typedef struct {
    int                      width;
    int                      height;
    uint8_t                  bpp; /*!< bits per pixel */
    uint8_t                  busy_level;
} me_gd_model_t;

extern me_gd_model_t me_gd_models[];

#endif

