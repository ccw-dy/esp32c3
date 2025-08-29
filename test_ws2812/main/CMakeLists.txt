idf_component_register(SRCS "main.c" "ws2812b_driver.c" "wifi_manager.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver esp_common nvs_flash esp_netif esp_event esp_wifi)