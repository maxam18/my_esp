idf_component_register(SRCS "me_wifi_ap.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash me_debug esp_event esp_netif esp_wifi)
