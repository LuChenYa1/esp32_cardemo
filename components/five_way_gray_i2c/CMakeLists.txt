idf_component_register(SRCS "five_way_gray_i2c.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver board_config freertos timer_system)
