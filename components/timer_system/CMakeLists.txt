idf_component_register(
    SRCS "timer_system.c"
    INCLUDE_DIRS "include"
    REQUIRES driver turn_detector pd_controller turn_statemachine
)
