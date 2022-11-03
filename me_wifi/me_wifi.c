/* My ESP wifi functions
 * File: me_wifi.c
 * Started: Fri Jun 18 23:16:57 MSK 2021
 * Author: Max Amzarakov (maxam18 _at_ gmail _._ com)
 * Copyright (c) 2021 ..."
 */

#include "me_wifi.h"

#include <string.h>

#include "sdkconfig.h"
#include "esp_event.h"
#include "esp_wifi.h"

#ifndef CONFIG_ESP_NETIF_TCPIP_LWIP
#include "tcpip_adapter.h"
#else
#include "esp_netif.h"
#endif

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#define GOT_IPv4_BIT    BIT(0)
#define CONNECTED_BITS  GOT_IPv4_BIT

static const char           *TAG = "WIFI";

static EventGroupHandle_t    wifi_comm_event_group;

static void (*got_ip_callback)(uint8_t) = NULL;

static void on_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

    ESP_LOGI(TAG, "Connected to " CONFIG_ME_WIFI_SSID ". Got IP " IPSTR, IP2STR(&event->ip_info.ip));

    if( got_ip_callback )
        got_ip_callback(1);

    xEventGroupSetBits(wifi_comm_event_group, GOT_IPv4_BIT);
}

static void on_connection(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if( event_id == WIFI_EVENT_STA_START )
    {
        ESP_LOGI(TAG, "Wi-Fi connected, getting IP...");
    } else if( event_id == WIFI_EVENT_STA_DISCONNECTED )
    {
        ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");

        if( got_ip_callback )
            got_ip_callback(0);
    }
    ESP_ERROR_CHECK(esp_wifi_connect());
}

static void start()
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

#ifdef CONFIG_ESP_NETIF_TCPIP_LWIP
    esp_netif_config_t     netif_config = ESP_NETIF_DEFAULT_WIFI_STA();
    esp_netif_t           *netif        = esp_netif_new(&netif_config);

    assert(netif);

    esp_netif_attach_wifi_station(netif);
    esp_wifi_set_default_wifi_sta_handlers();
#else
    tcpip_adapter_init();
#endif


    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START, &on_connection, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_connection, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid     = CONFIG_ME_WIFI_SSID,
            .password = CONFIG_ME_WIFI_PASSWORD,
        },
    };
    ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void me_wifi_init_cb(void (*func)(uint8_t))
{
    if (wifi_comm_event_group != NULL) {
        ESP_LOGE(TAG, "invalid state @ %s", __FUNCTION__);
        return;
    }

    wifi_comm_event_group = xEventGroupCreate();

    got_ip_callback = func;

    start();
    ESP_LOGI(TAG, "Waiting for IP...");

    if( func == NULL )
    {
        xEventGroupWaitBits(wifi_comm_event_group, CONNECTED_BITS, true, true, portMAX_DELAY);
    }

}

void me_wifi_init(void)
{
    me_wifi_init_cb(NULL);   
}
