idf_component_register(SRCS "gray_sensor.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_adc freertos)