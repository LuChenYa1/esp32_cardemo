idf_component_register(
    SRCS "camera_protocol.c"
    INCLUDE_DIRS "include"
    REQUIRES driver uart 485servo board_config
)
