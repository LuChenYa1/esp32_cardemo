idf_component_register(
    SRCS "485servo.c"
    INCLUDE_DIRS "include"
    REQUIRES uart board_config
)
