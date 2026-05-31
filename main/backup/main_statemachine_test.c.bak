// /**
//  * @file main_statemachine_test.c
//  * @brief 六状态转弯状态机测试程序
//  * 
//  * 测试内容：
//  * 1. 状态机初始化
//  * 2. 模拟转弯请求，验证状态转换
//  * 3. 验证转弯完成后标志清除
//  */

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_log.h"
// #include "nvs_flash.h"

// // 组件头文件
// #include "timer_system.h"
// #include "gray_sensor.h"
// #include "turn_detector.h"
// #include "pd_controller.h"
// #include "turn_statemachine.h"
// #include "pwm.h"

// static const char *TAG = "statemachine_test";

// /**
//  * @brief 状态名称字符串
//  */
// static const char* state_names[] = {
//     "TURN_IDLE",
//     "TURN_STOP",
//     "TURN_BACK",
//     "TURN_PHASE1",
//     "TURN_PHASE2",
//     "TURN_ADJUST"
// };

// /**
//  * @brief 获取状态名称
//  */
// static const char* get_state_name(TurnState_t state)
// {
//     if (state >= 0 && state <= TURN_ADJUST) {
//         return state_names[state];
//     }
//     return "UNKNOWN";
// }

// /**
//  * @brief 监控任务 - 显示状态机状态
//  */
// void monitor_task(void *pvParameters)
// {
//     TurnState_t last_state = TURN_IDLE;
//     uint32_t state_change_count = 0;
    
//     ESP_LOGI(TAG, "=== 状态机监控任务启动 ===");
    
//     while (1) {
//         // 获取当前状态
//         TurnState_t current_state = turn_statemachine_get_state();
        
//         // 检测状态变化
//         if (current_state != last_state) {
//             state_change_count++;
            
//             // 获取调试信息
//             uint16_t tick_count;
//             uint8_t turn_dir;
//             turn_statemachine_get_debug_info(&tick_count, &turn_dir);
            
//             ESP_LOGI(TAG, "[状态变化 #%lu] %s -> %s (tick=%u, dir=%u)",
//                      state_change_count,
//                      get_state_name(last_state),
//                      get_state_name(current_state),
//                      tick_count,
//                      turn_dir);
            
//             last_state = current_state;
//         }
        
//         // 显示转弯标志
//         bool is_turning = turn_detector_is_turning();
//         uint8_t turn_type = turn_detector_get_type();
        
//         if (is_turning || turn_type != TURN_TYPE_NONE) {
//             ESP_LOGI(TAG, "  转弯标志: type=%u, in_progress=%d", turn_type, is_turning);
//         }
        
//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
// }

// /**
//  * @brief 模拟转弯请求任务
//  */
// void simulate_turn_task(void *pvParameters)
// {
//     ESP_LOGI(TAG, "=== 模拟转弯请求任务启动 ===");
    
//     // 等待5秒，让系统稳定
//     vTaskDelay(pdMS_TO_TICKS(5000));
    
//     ESP_LOGI(TAG, ">>> 模拟十字路口检测（设置转弯标志）");
    
//     // 模拟转弯检测：手动设置转弯标志
//     // 注意：正常情况下这些标志由turn_detector_tick自动设置
//     turn_detector_set_turning(true);
    
//     // 等待状态机完成转弯
//     ESP_LOGI(TAG, "等待状态机完成转弯...");
    
//     while (1) {
//         TurnState_t state = turn_statemachine_get_state();
//         bool is_turning = turn_detector_is_turning();
        
//         if (state == TURN_IDLE && !is_turning) {
//             ESP_LOGI(TAG, ">>> 转弯完成！状态机已回到IDLE，转弯标志已清除");
//             break;
//         }
        
//         vTaskDelay(pdMS_TO_TICKS(100));
//     }
    
//     // 等待5秒后再次测试
//     vTaskDelay(pdMS_TO_TICKS(5000));
    
//     ESP_LOGI(TAG, ">>> 再次模拟十字路口检测");
//     turn_detector_set_turning(true);
    
//     ESP_LOGI(TAG, "测试任务完成");
//     vTaskDelete(NULL);
// }

// void app_main(void)
// {
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "  六状态转弯状态机测试程序");
//     ESP_LOGI(TAG, "========================================");
    
//     // 初始化NVS
//     esp_err_t ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         ret = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(ret);
    
//     // 1. 初始化PWM（电机控制）
//     ESP_LOGI(TAG, "1. 初始化PWM...");
//     ledc_init();
    
//     // 2. 初始化灰度传感器
//     ESP_LOGI(TAG, "2. 初始化灰度传感器...");
//     gray_sensor_init();
    
//     // 设置校准参数（白底黑线）
//     gray_sensor_set_calibration(GRAY_SENSOR_LEFT, 4095, 1476);
//     gray_sensor_set_calibration(GRAY_SENSOR_RIGHT, 4095, 1546);
    
//     // 3. 初始化转弯检测模块
//     ESP_LOGI(TAG, "3. 初始化转弯检测模块...");
//     turn_detector_init();
    
//     // 4. 初始化PD控制器
//     ESP_LOGI(TAG, "4. 初始化PD控制器...");
//     pd_controller_init();
//     pd_controller_set_params(700, 5.0f, 12.0f);
    
//     // 5. 初始化转弯状态机
//     ESP_LOGI(TAG, "5. 初始化转弯状态机...");
//     turn_statemachine_init();
    
//     // 6. 初始化定时器系统
//     ESP_LOGI(TAG, "6. 初始化定时器系统...");
//     ret = timer_system_init();
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "定时器系统初始化失败！");
//         return;
//     }
    
//     // 7. 创建ADC采样任务
//     ESP_LOGI(TAG, "7. 创建ADC采样任务...");
//     xTaskCreate(adc_sampling_task, "adc_sampling", 4096, NULL, 5, NULL);
    
//     // 8. 创建监控任务
//     ESP_LOGI(TAG, "8. 创建监控任务...");
//     xTaskCreate(monitor_task, "monitor", 4096, NULL, 3, NULL);
    
//     // 9. 创建模拟转弯请求任务
//     ESP_LOGI(TAG, "9. 创建模拟转弯请求任务...");
//     xTaskCreate(simulate_turn_task, "simulate_turn", 4096, NULL, 3, NULL);
    
//     // 10. 启动定时器系统
//     ESP_LOGI(TAG, "10. 启动定时器系统...");
//     ret = timer_system_start();
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "定时器系统启动失败！");
//         return;
//     }
    
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "  系统启动完成，开始测试");
//     ESP_LOGI(TAG, "========================================");
    
//     // 主循环
//     while (1) {
//         // 显示定时器计数
//         uint32_t timer0_count = timer_system_get_timer0_count();
//         uint32_t timer1_count = timer_system_get_timer1_count();
        
//         ESP_LOGI(TAG, "Timer0: %lu, Timer1: %lu", timer0_count, timer1_count);
        
//         vTaskDelay(pdMS_TO_TICKS(5000));
//     }
// }
