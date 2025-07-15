idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES nvs_flash esp_adc esp_wifi nvs_flash esp_http_server esp_timer Mini2
                    EMBED_FILES "index.html"
                    )
