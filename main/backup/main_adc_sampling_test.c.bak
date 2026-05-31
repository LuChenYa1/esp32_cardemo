// /*
//  * ADC采样任务测试程序
//  *
//  * 功能：
//  * 1. 测试ADC采样任务的创建和运行
//  * 2. 验证原子变量的读写
//  * 3. 验证错误处理逻辑
//  * 4. 打印ADC采样数据和错误计数
//  */

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_log.h"
// #include "gray_sensor.h"

// static const char *TAG = "ADC_SAMPLING_TEST";

// /**
//  * @brief 监控任务 - 定期打印ADC缓存值和错误计数
//  */
// void monitor_task(void *pvParameters)
// {
//     ESP_LOGI(TAG, "监控任务已启动");
    
//     while (1) {
//         uint16_t left, right;
//         uint32_t error_count;
        
//         // 获取缓存的ADC值（中断安全）
//         gray_scanner_get_cached_values(&left, &right);
        
//         // 获取错误计数
//         error_count = gray_scanner_get_error_count();
        
//         // 打印数据
//         ESP_LOGI(TAG, "ADC缓存值 - 左: %d, 右: %d | 错误计数: %lu", 
//                  left, right, error_count);
        
//         // 每500ms打印一次
//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
// }

// void app_main(void)
// {
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "ADC采样任务测试程序");
//     ESP_LOGI(TAG, "========================================");
    
//     // 初始化灰度传感器扫描器（创建ADC采样任务）
//     gray_scanner_init();
    
//     ESP_LOGI(TAG, "ADC采样任务已创建，优先级: 5");
//     ESP_LOGI(TAG, "采样周期: 1ms");
//     ESP_LOGI(TAG, "使用原子变量保护共享数据");
    
//     // 等待ADC采样任务启动
//     vTaskDelay(pdMS_TO_TICKS(100));
    
//     // 创建监控任务
//     xTaskCreate(monitor_task, "monitor", 3072, NULL, 3, NULL);
    
//     ESP_LOGI(TAG, "监控任务已创建，优先级: 3");
//     ESP_LOGI(TAG, "开始监控ADC采样数据...");
//     ESP_LOGI(TAG, "========================================");
    
//     // 主循环空闲
//     while (1) {
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }
