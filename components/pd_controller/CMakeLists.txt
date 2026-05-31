idf_component_register(
    SRCS "pd_controller.c"
    INCLUDE_DIRS "include"
    REQUIRES gray_sensor turn_detector pwm
)
