idf_component_register(
    SRCS "pcf_buzzer.c"
    INCLUDE_DIRS "include"
    REQUIRES freertos esp_timer pcf8574
)
