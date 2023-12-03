idf_component_register(SRCS "me_wifi.c"
                    INCLUDE_DIRS "include"
                    REQUIRES me_debug esp_event esp_netif esp_wifi)
