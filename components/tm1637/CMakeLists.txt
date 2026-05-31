idf_component_register(
    SRCS "tm1637.c"
    INCLUDE_DIRS "include"
    REQUIRES us_delay driver board_config
)
