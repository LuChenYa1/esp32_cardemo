# 主程序CMakeLists.txt配置
# 注意: 只能包含一个包含 app_main() 的文件
# 使用方法: 修改 TEST_ACTIVE 变量来选择要编译的测试程序

# ========================================
# 测试程序选择（修改此处切换测试）
# ========================================
# 可选值:
#   main_spiflash_test        - SPI Flash读写测试
#   main_wireless_test        - 无线通信模块测试
#   main_pcf8574_test         - PCF8574 I2C扩展芯片测试
#   gray_calibration_test     - 灰度传感器校准测试
#   main_integrated_test      - 综合测试（巡线+显示+舵机）
#   main_cartest              - 巡线小车单任务版本
#   main_adc_sampling_test    - ADC采样任务测试
#   main_display_test         - 数码管轮播显示测试
#   main_servo_test           - 舵机自动往复控制测试
#   main_pd_test              - PD控制器测试
#   main_statemachine_test    - 六状态转弯状态机测试
#   main_timer_test           - 定时器系统测试
#   main_voice_test           - 语音控制模块测试
#   main_turn_detector_test   - 转弯检测模块测试
#   gpio_drive_test           - GPIO驱动能力测试
#   main                      - 原始主程序（需配合line_following.c等）
# ========================================

set(TEST_ACTIVE "main_spiflash_test")

# 根据选择的测试程序设置源文件
if(TEST_ACTIVE STREQUAL "main_spiflash_test")
    set(MAIN_SRCS "main_spiflash_test.c")
elseif(TEST_ACTIVE STREQUAL "main_wireless_test")
    set(MAIN_SRCS "main_wireless_test.c")
elseif(TEST_ACTIVE STREQUAL "main_pcf8574_test")
    set(MAIN_SRCS "main_pcf8574_test.c")
elseif(TEST_ACTIVE STREQUAL "gray_calibration_test")
    set(MAIN_SRCS "gray_calibration_test.c")
elseif(TEST_ACTIVE STREQUAL "main_integrated_test")
    set(MAIN_SRCS "main_integrated_test.c")
elseif(TEST_ACTIVE STREQUAL "main_cartest")
    set(MAIN_SRCS "main_cartest.c")
elseif(TEST_ACTIVE STREQUAL "main_adc_sampling_test")
    set(MAIN_SRCS "main_adc_sampling_test.c")
elseif(TEST_ACTIVE STREQUAL "main_display_test")
    set(MAIN_SRCS "main_display_test.c")
elseif(TEST_ACTIVE STREQUAL "main_servo_test")
    set(MAIN_SRCS "main_servo_test.c")
elseif(TEST_ACTIVE STREQUAL "main_pd_test")
    set(MAIN_SRCS "main_pd_test.c")
elseif(TEST_ACTIVE STREQUAL "main_statemachine_test")
    set(MAIN_SRCS "main_statemachine_test.c")
elseif(TEST_ACTIVE STREQUAL "main_timer_test")
    set(MAIN_SRCS "main_timer_test.c")
elseif(TEST_ACTIVE STREQUAL "main_voice_test")
    set(MAIN_SRCS "main_voice_test.c")
elseif(TEST_ACTIVE STREQUAL "main_turn_detector_test")
    set(MAIN_SRCS "main_turn_detector_test.c")
elseif(TEST_ACTIVE STREQUAL "gpio_drive_test")
    set(MAIN_SRCS "gpio_drive_test.c")
elseif(TEST_ACTIVE STREQUAL "main")
    set(MAIN_SRCS "main.c" "line_following.c" "turn_detection.c")
else()
    message(FATAL_ERROR "未知的测试程序: ${TEST_ACTIVE}")
endif()

idf_component_register(SRCS ${MAIN_SRCS}
                    INCLUDE_DIRS "."
                    REQUIRES 
                    dht11 
                    wireless 
                    esp_timer 
                    tm1637 
                    ssax1 
                    ssdx 
                    pwm 
                    pcf8574 
                    encoder 
                    spiflash 
                    485servo 
                    uart 
                    gray_sensor
                    pcf_buzzer 
                    timer_system 
                    turn_detector 
                    pd_controller 
                    turn_statemachine 
                    display_task 
                    servo_task 
                    voice_module 
                    camera_protocol 
                    ir_obstacle 
                    traffic_light 
                    five_way_gray_i2c 
                    hc-sr04 
                    board_config)
