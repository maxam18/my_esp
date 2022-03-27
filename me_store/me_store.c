/* My ESP generalized NVS functions
 * File: me_store.c
 * Started: Wed Mar 23 12:55:13 MSK 2022
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2022 ..."
 */
#include <esp_system.h>

#include <esp_err.h>
#include <me_store.h>

#ifdef CONFIG_ME_NVS_STORE_NAME
static const char *me_nvs_name = CONFIG_ME_NVS_STORE_NAME;
#else
static const char *me_nvs_name = "my_esp";
#endif

esp_err_t me_store_read(const char *name, void *data, size_t size)
{
    nvs_handle  store;
    esp_err_t   err;
    size_t      store_sz = 0;

    err = nvs_open(me_nvs_name, NVS_READONLY, &store);
    if( err != ESP_OK )
        return err;

    err = nvs_get_blob(store, name, NULL, &store_sz);

    if( (err == ESP_OK) 
        && (store_sz == size) )
    {
        err = nvs_get_blob(store, name, data, &store_sz);
    }

    nvs_close(store);

    return err;
}

esp_err_t me_store_write(const char *name, void *data, size_t size)
{
    nvs_handle  store;
    esp_err_t   err;

    err = nvs_open(me_nvs_name, NVS_READWRITE, &store);
    if( err != ESP_OK )
        return err;

    err = nvs_set_blob(store, name, data, size);
    if (err ==  ESP_OK ) {
        nvs_commit(store);
    }

    nvs_close(store);

    return err;
}