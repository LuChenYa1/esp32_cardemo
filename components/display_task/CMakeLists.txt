idf_component_register(
    SRCS "display_task.c"
    INCLUDE_DIRS "include"
    REQUIRES 
        driver          # GPIO驱动
        tm1637          # TM1637数码管驱动
        dht11           # DHT11温湿度传感器驱动
        hc-sr04         # HC-SR04超声波传感器驱动
        ir_obstacle     # 红外避障传感器驱动
        board_config    # 引脚定义
)
