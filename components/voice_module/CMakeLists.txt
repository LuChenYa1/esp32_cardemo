idf_component_register(
    SRCS "voice_module.c"
    INCLUDE_DIRS "include"
    REQUIRES 
        driver              # UART驱动
        freertos            # FreeRTOS任务
        esp_timer           # esp_timer_get_time函数
        camera_protocol     # 摄像头协议
        pd_controller       # PD控制器
        turn_statemachine   # 转弯状态机
        board_config        # 引脚定义
)
