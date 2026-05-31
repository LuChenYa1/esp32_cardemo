idf_component_register(
    SRCS "wireless.c"
    INCLUDE_DIRS "include"
    REQUIRES driver uart board_config
)
