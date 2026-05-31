idf_component_register(
    SRCS "gpio_manager.c" "board_config.c"
    INCLUDE_DIRS "include"
    REQUIRES driver
)