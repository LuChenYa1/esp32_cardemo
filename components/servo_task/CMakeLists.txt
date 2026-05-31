idf_component_register(
    SRCS "servo_task.c"
    INCLUDE_DIRS "include"
    REQUIRES freertos 485servo
)
