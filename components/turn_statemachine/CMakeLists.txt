idf_component_register(
    SRCS "turn_statemachine.c"
    INCLUDE_DIRS "include"
    REQUIRES driver pwm gray_sensor turn_detector
)
