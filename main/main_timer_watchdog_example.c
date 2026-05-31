// /**
//  * @file main_timer_watchdog_example.c
//  * @brief 定时器系统和看门狗配置示例
//  * 
//  * 本示例展示如何正确初始化和使用：
//  * 1. 看门狗定时器（在启动定时器之前配置）
//  * 2. 定时器系统（Timer 0和Timer 1）
//  * 3. 错误处理和安全模式
//  */

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_log.h"

// // 定时器系统
// #include "timer_system.h"

// // 灰度传感器和ADC采样
// #include "gray_sensor.h"

// // 电机控制
// #include "pwm.h"

// // 转弯检测和控制
// #include "turn_detector.h"
// #include "pd_controller.h"
// #include "turn_statemachine.h"

// static const char *TAG = "TIMER_WATCHDOG_EXAMPLE";

// // ==================== 校准参数 ====================
// #define LEFT_WHITE          4095
// #define LEFT_BLACK          1476
// #define RIGHT_WHITE         4095
// #define RIGHT_BLACK         1546

// // ==================== PD控制参数 ====================
// #define LINE_SPEED          700     // 基础速度 (0-1023)
// #define LINE_KP             5.0f    // 比例系数
// #define LINE_KD             12.0f   // 微分系数

// /**
//  * @brief 主函数
//  */
// void app_main(void)
// {
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "定时器系统和看门狗配置示例");
//     ESP_LOGI(TAG, "========================================");
    
//     // ========== 步骤1：初始化硬件外设 ==========
//     ESP_LOGI(TAG, "步骤1：初始化硬件外设...");
    
//     // 初始化灰度传感器（ADC2）
//     gray_sensor_init_simple();
    
//     // 设置校准参数
//     gray_sensor_set_calibration(GRAY_SENSOR_LEFT, LEFT_WHITE, LEFT_BLACK);
//     gray_sensor_set_calibration(GRAY_SENSOR_RIGHT, RIGHT_WHITE, RIGHT_BLACK);
    
//     // 初始化PWM（电机控制）
//     ledc_init();
    
//     ESP_LOGI(TAG, "硬件外设初始化完成");
    
//     // ========== 步骤2：初始化ADC采样任务 ==========
//     ESP_LOGI(TAG, "步骤2：初始化ADC采样任务...");
    
//     // 创建ADC采样任务（优先级5，高于定时器中断之外的任务）
//     gray_scanner_init();
    
//     ESP_LOGI(TAG, "ADC采样任务创建完成");
    
//     // ========== 步骤3：初始化转弯检测和控制模块 ==========
//     ESP_LOGI(TAG, "步骤3：初始化转弯检测和控制模块...");
    
//     // 初始化转弯检测器
//     turn_detector_init();
    
//     // 初始化PD控制器
//     pd_controller_init();
//     pd_controller_set_params(LINE_SPEED, LINE_KP, LINE_KD);
    
//     // 初始化转弯状态机
//     turn_statemachine_init();
    
//     ESP_LOGI(TAG, "转弯检测和控制模块初始化完成");
    
//     // ========== 步骤4：初始化定时器系统 ==========
//     ESP_LOGI(TAG, "步骤4：初始化定时器系统...");
    
//     esp_err_t ret = timer_system_init();
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "定时器系统初始化失败！");
//         ESP_LOGE(TAG, "系统停止启动");
//         return;
//     }
    
//     ESP_LOGI(TAG, "定时器系统初始化完成");
    
//     // ========== 步骤5：配置看门狗定时器（重要！） ==========
//     ESP_LOGI(TAG, "步骤5：配置看门狗定时器...");
    
//     ret = watchdog_init();
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "看门狗初始化失败！");
//         ESP_LOGW(TAG, "系统将继续运行，但没有看门狗保护");
//     } else {
//         ESP_LOGI(TAG, "看门狗定时器配置完成");
//     }
    
//     // ========== 步骤6：启动定时器系统 ==========
//     ESP_LOGI(TAG, "步骤6：启动定时器系统...");
    
//     ret = timer_system_start();
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "定时器系统启动失败！");
//         ESP_LOGE(TAG, "系统停止启动");
//         return;
//     }
    
//     ESP_LOGI(TAG, "定时器系统已启动");
    
//     // ========== 步骤7：系统运行 ==========
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "系统启动完成！");
//     ESP_LOGI(TAG, "Timer 0: 1ms周期 - 灰度扫描和转弯检测");
//     ESP_LOGI(TAG, "Timer 1: 10ms周期 - PD控制和转弯状态机");
//     ESP_LOGI(TAG, "看门狗: 5秒超时 - 监控IDLE任务");
//     ESP_LOGI(TAG, "========================================");
    
//     // 主循环：监控系统状态
//     uint32_t last_timer0_count = 0;
//     uint32_t last_timer1_count = 0;
    
//     while (1) {
//         // 每秒打印一次系统状态
//         vTaskDelay(pdMS_TO_TICKS(1000));
        
//         uint32_t timer0_count = timer_system_get_timer0_count();
//         uint32_t timer1_count = timer_system_get_timer1_count();
//         uint32_t adc_error_count = gray_scanner_get_error_count();
        
//         ESP_LOGI(TAG, "系统状态 - Timer0: %lu次/s, Timer1: %lu次/s, ADC错误: %lu",
//                  timer0_count - last_timer0_count,
//                  timer1_count - last_timer1_count,
//                  adc_error_count);
        
//         last_timer0_count = timer0_count;
//         last_timer1_count = timer1_count;
        
//         // 检查ADC错误计数
//         if (adc_error_count > 50) {
//             ESP_LOGW(TAG, "警告：ADC错误计数较高！");
//         }
        
//         // 打印传感器状态（可选）
//         // gray_sensor_print_status();
//     }
// }
