idf_component_register(
    SRCS "turn_detector.c"
    INCLUDE_DIRS "include"
    REQUIRES gray_sensor voice_module
)
