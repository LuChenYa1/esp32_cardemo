idf_component_register(SRCS "vl53l0.c"
                       INCLUDE_DIRS "include"
                       REQUIRES driver esp_timer i2c board_config)