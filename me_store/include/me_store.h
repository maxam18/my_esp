/* My ESP generalized NVS functions
 * File: me_store.h
 * Started: Wed Mar 23 12:55:20 MSK 2022
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2022 ..."
 */

#ifndef _ME_STORE_H
#define _ME_STORE_H

#include <nvs_flash.h>

esp_err_t me_store_read(const char *name, void *data, size_t size);
esp_err_t me_store_write(const char *name, void *data, size_t size);

#endif


