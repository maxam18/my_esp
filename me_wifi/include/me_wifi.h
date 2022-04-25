/* My ESP wifi functions
 * File: me_wifi.h
 * Started: Fri Jun 18 23:17:05 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */

#ifndef _ME_WIFI_H
#define _ME_WIFI_H
/**
 * @brief initialized wifi client
 *        storage MUST be initialized before the call wifi_init
 *        CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD must be set
 */
void me_wifi_init(void);

/**
 * @brief initialized wifi client with callback function
 *        storage MUST be initialized before the call wifi_init
 *        CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD must be set
 */
void me_wifi_init_cb(void (*func)(unsigned char));


#endif
