idf_component_register(
    SRCS "mpu_i2c.c" "vl53l0_i2c.c"
    INCLUDE_DIRS "include"
    REQUIRES driver board_config
)
