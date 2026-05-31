// /**
//  * @file main_pd_test.c
//  * @brief PD控制器测试程序
//  * 
//  * 测试内容：
//  * 1. PD控制器初始化
//  * 2. 参数设置和读取
//  * 3. 与定时器系统集成测试
//  * 4. 转弯期间暂停控制测试
//  */

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_log.h"
// #include "gray_sensor.h"
// #include "turn_detector.h"
// #include "pd_controller.h"
// #include "timer_system.h"
// #include "pwm.h"

// static const char *TAG = "pd_test";

// /**
//  * @brief 测试PD控制器参数设置和读取
//  */
// void test_pd_params(void)
// {
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "测试1: PD参数设置和读取");
//     ESP_LOGI(TAG, "========================================");
    
//     // 设置新参数
//     pd_controller_set_params(800, 6.0f, 15.0f);
    
//     // 读取参数
//     uint16_t speed;
//     float kp, kd;
//     pd_controller_get_params(&speed, &kp, &kd);
    
//     ESP_LOGI(TAG, "读取到的参数: 速度=%d, Kp=%.2f, Kd=%.2f", speed, kp, kd);
    
//     // 验证
//     if (speed == 800 && kp == 6.0f && kd == 15.0f) {
//         ESP_LOGI(TAG, "✓ 参数设置和读取测试通过");
//     } else {
//         ESP_LOGE(TAG, "✗ 参数设置和读取测试失败");
//     }
    
//     // 恢复默认参数
//     pd_controller_set_params(DEFAULT_SPEED, DEFAULT_KP, DEFAULT_KD);
// }

// /**
//  * @brief 测试PD控制器重置
//  */
// void test_pd_reset(void)
// {
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "测试2: PD控制器重置");
//     ESP_LOGI(TAG, "========================================");
    
//     // 重置控制器
//     pd_controller_reset();
    
//     // 获取last_error
//     float last_error = pd_controller_get_last_error();
    
//     ESP_LOGI(TAG, "重置后的last_error: %.2f", last_error);
    
//     if (last_error == 0.0f) {
//         ESP_LOGI(TAG, "✓ PD重置测试通过");
//     } else {
//         ESP_LOGE(TAG, "✗ PD重置测试失败");
//     }
// }

// /**
//  * @brief 监控任务 - 定期打印PD控制器状态
//  */
// void monitor_task(void *pvParameters)
// {
//     ESP_LOGI(TAG, "监控任务已启动");
    
//     uint32_t last_timer0_count = 0;
//     uint32_t last_timer1_count = 0;
    
//     while (1) {
//         vTaskDelay(pdMS_TO_TICKS(1000));
        
//         // 获取定时器计数
//         uint32_t timer0_count = timer_system_get_timer0_count();
//         uint32_t timer1_count = timer_system_get_timer1_count();
        
//         // 计算频率
//         uint32_t timer0_freq = timer0_count - last_timer0_count;
//         uint32_t timer1_freq = timer1_count - last_timer1_count;
        
//         last_timer0_count = timer0_count;
//         last_timer1_count = timer1_count;
        
//         // 获取ADC值
//         uint16_t left_raw, right_raw;
//         gray_scanner_get_cached_values(&left_raw, &right_raw);
        
//         // 获取转弯状态
//         bool is_turning = turn_detector_is_turning();
//         uint8_t turn_type = turn_detector_get_type();
        
//         // 获取PD参数
//         uint16_t speed;
//         float kp, kd;
//         pd_controller_get_params(&speed, &kp, &kd);
        
//         // 获取误差
//         float last_error = pd_controller_get_last_error();
        
//         ESP_LOGI(TAG, "========================================");
//         ESP_LOGI(TAG, "定时器频率: Timer0=%lu Hz, Timer1=%lu Hz", timer0_freq, timer1_freq);
//         ESP_LOGI(TAG, "ADC值: 左=%d, 右=%d", left_raw, right_raw);
//         ESP_LOGI(TAG, "转弯状态: %s, 类型=%d", is_turning ? "进行中" : "空闲", turn_type);
//         ESP_LOGI(TAG, "PD参数: 速度=%d, Kp=%.2f, Kd=%.2f", speed, kp, kd);
//         ESP_LOGI(TAG, "PD误差: %.2f", last_error);
//         ESP_LOGI(TAG, "ADC错误计数: %lu", gray_scanner_get_error_count());
//     }
// }

// /**
//  * @brief 主函数
//  */
// void app_main(void)
// {
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "PD控制器测试程序");
//     ESP_LOGI(TAG, "========================================");
    
//     // 延时等待系统稳定
//     vTaskDelay(pdMS_TO_TICKS(1000));
    
//     // 1. 初始化PWM
//     ESP_LOGI(TAG, "初始化PWM...");
//     ledc_init();
    
//     // 2. 初始化灰度传感器（简化模式）
//     ESP_LOGI(TAG, "初始化灰度传感器...");
//     gray_sensor_init_simple();
    
//     // 设置校准参数（使用设计文档中的值）
//     gray_sensor_set_calibration(GRAY_SENSOR_LEFT, 4095, 1476);
//     gray_sensor_set_calibration(GRAY_SENSOR_RIGHT, 4095, 1546);
    
//     // 3. 初始化ADC采样任务
//     ESP_LOGI(TAG, "初始化ADC采样任务...");
//     gray_scanner_init();
    
//     // 等待ADC采样任务启动
//     vTaskDelay(pdMS_TO_TICKS(100));
    
//     // 4. 初始化转弯检测器
//     ESP_LOGI(TAG, "初始化转弯检测器...");
//     turn_detector_init();
    
//     // 5. 初始化PD控制器
//     ESP_LOGI(TAG, "初始化PD控制器...");
//     pd_controller_init();
    
//     // 6. 执行单元测试
//     test_pd_params();
//     test_pd_reset();
    
//     // 7. 初始化定时器系统
//     ESP_LOGI(TAG, "初始化定时器系统...");
//     esp_err_t ret = timer_system_init();
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "定时器系统初始化失败");
//         return;
//     }
    
//     // 8. 启动定时器
//     ESP_LOGI(TAG, "启动定时器系统...");
//     ret = timer_system_start();
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "定时器系统启动失败");
//         return;
//     }
    
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "PD控制器已启动，开始实时控制");
//     ESP_LOGI(TAG, "========================================");
    
//     // 9. 创建监控任务
//     xTaskCreate(monitor_task, "monitor", 4096, NULL, 3, NULL);
    
//     // 10. 主循环 - 可以在这里动态调整参数
//     while (1) {
//         vTaskDelay(pdMS_TO_TICKS(5000));
        
//         // 示例：每5秒调整一次速度
//         static uint16_t test_speed = 700;
//         test_speed = (test_speed == 700) ? 800 : 700;
        
//         ESP_LOGI(TAG, "调整速度为: %d", test_speed);
//         pd_controller_set_params(test_speed, DEFAULT_KP, DEFAULT_KD);
//     }
// }
