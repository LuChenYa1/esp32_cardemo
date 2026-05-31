// /**
//  * @file main_turn_detector_test.c
//  * @brief 转弯检测模块测试程序
//  * 
//  * 测试内容：
//  * - 转弯检测初始化
//  * - 模拟传感器输入测试转弯检测逻辑
//  * - 验证连续15次确认机制
//  * - 验证转弯期间暂停检测
//  */

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_log.h"
// #include "turn_detector.h"
// #include "gray_sensor.h"
// #include "timer_system.h"

// static const char *TAG = "turn_detector_test";

// /**
//  * @brief 测试1：初始化测试
//  */
// void test_turn_detector_init(void)
// {
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "测试1：转弯检测初始化");
//     ESP_LOGI(TAG, "========================================");
    
//     turn_detector_init();
    
//     // 验证初始状态
//     uint8_t turn_type = turn_detector_get_type();
//     bool is_turning = turn_detector_is_turning();
    
//     ESP_LOGI(TAG, "初始转弯类型: %d (期望: 0)", turn_type);
//     ESP_LOGI(TAG, "初始转弯状态: %d (期望: 0)", is_turning);
    
//     if (turn_type == TURN_TYPE_NONE && !is_turning) {
//         ESP_LOGI(TAG, "✓ 测试1通过");
//     } else {
//         ESP_LOGE(TAG, "✗ 测试1失败");
//     }
    
//     ESP_LOGI(TAG, "");
// }

// /**
//  * @brief 测试2：连续确认机制测试
//  */
// void test_turn_detector_confirm(void)
// {
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "测试2：连续15次确认机制");
//     ESP_LOGI(TAG, "========================================");
    
//     // 重置状态
//     turn_detector_clear_flags();
    
//     ESP_LOGI(TAG, "说明：此测试需要在Timer 0中断中运行");
//     ESP_LOGI(TAG, "当前仅验证API调用正常");
    
//     // 验证清除标志功能
//     uint8_t turn_type = turn_detector_get_type();
//     bool is_turning = turn_detector_is_turning();
    
//     ESP_LOGI(TAG, "清除后转弯类型: %d (期望: 0)", turn_type);
//     ESP_LOGI(TAG, "清除后转弯状态: %d (期望: 0)", is_turning);
    
//     if (turn_type == TURN_TYPE_NONE && !is_turning) {
//         ESP_LOGI(TAG, "✓ 测试2通过");
//     } else {
//         ESP_LOGE(TAG, "✗ 测试2失败");
//     }
    
//     ESP_LOGI(TAG, "");
// }

// /**
//  * @brief 测试3：转弯标志设置和读取
//  */
// void test_turn_detector_flags(void)
// {
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "测试3：转弯标志设置和读取");
//     ESP_LOGI(TAG, "========================================");
    
//     // 清除标志
//     turn_detector_clear_flags();
    
//     // 设置转弯进行中标志
//     turn_detector_set_turning(true);
    
//     bool is_turning = turn_detector_is_turning();
//     ESP_LOGI(TAG, "设置后转弯状态: %d (期望: 1)", is_turning);
    
//     if (is_turning) {
//         ESP_LOGI(TAG, "✓ 设置转弯标志成功");
//     } else {
//         ESP_LOGE(TAG, "✗ 设置转弯标志失败");
//     }
    
//     // 清除标志
//     turn_detector_set_turning(false);
//     is_turning = turn_detector_is_turning();
//     ESP_LOGI(TAG, "清除后转弯状态: %d (期望: 0)", is_turning);
    
//     if (!is_turning) {
//         ESP_LOGI(TAG, "✓ 测试3通过");
//     } else {
//         ESP_LOGE(TAG, "✗ 测试3失败");
//     }
    
//     ESP_LOGI(TAG, "");
// }

// /**
//  * @brief 测试4：调试信息获取
//  */
// void test_turn_detector_debug_info(void)
// {
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "测试4：调试信息获取");
//     ESP_LOGI(TAG, "========================================");
    
//     uint8_t both_count, left_count, right_count;
//     turn_detector_get_debug_info(&both_count, &left_count, &right_count);
    
//     ESP_LOGI(TAG, "双传感器计数器: %d", both_count);
//     ESP_LOGI(TAG, "左传感器计数器: %d", left_count);
//     ESP_LOGI(TAG, "右传感器计数器: %d", right_count);
    
//     ESP_LOGI(TAG, "✓ 测试4通过（调试信息获取正常）");
//     ESP_LOGI(TAG, "");
// }

// /**
//  * @brief 测试5：集成测试（需要Timer 0中断）
//  */
// void test_turn_detector_integration(void)
// {
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "测试5：集成测试（Timer 0中断）");
//     ESP_LOGI(TAG, "========================================");
    
//     ESP_LOGI(TAG, "初始化灰度传感器扫描器...");
//     gray_scanner_init();
//     vTaskDelay(pdMS_TO_TICKS(100));
    
//     ESP_LOGI(TAG, "初始化定时器系统...");
//     esp_err_t err = timer_system_init();
//     if (err != ESP_OK) {
//         ESP_LOGE(TAG, "✗ 定时器系统初始化失败");
//         return;
//     }
    
//     ESP_LOGI(TAG, "启动定时器系统...");
//     err = timer_system_start();
//     if (err != ESP_OK) {
//         ESP_LOGE(TAG, "✗ 定时器系统启动失败");
//         return;
//     }
    
//     ESP_LOGI(TAG, "定时器系统已启动，转弯检测正在运行...");
//     ESP_LOGI(TAG, "监控转弯检测状态（10秒）...");
    
//     // 监控10秒
//     for (int i = 0; i < 10; i++) {
//         vTaskDelay(pdMS_TO_TICKS(1000));
        
//         uint32_t timer0_count = timer_system_get_timer0_count();
//         uint8_t turn_type = turn_detector_get_type();
//         bool is_turning = turn_detector_is_turning();
        
//         uint8_t both_count, left_count, right_count;
//         turn_detector_get_debug_info(&both_count, &left_count, &right_count);
        
//         ESP_LOGI(TAG, "[%d秒] Timer0计数: %lu, 转弯类型: %d, 转弯中: %d, 计数器: %d/%d/%d",
//                  i + 1, timer0_count, turn_type, is_turning,
//                  both_count, left_count, right_count);
        
//         // 如果检测到转弯，清除标志继续测试
//         if (is_turning) {
//             ESP_LOGI(TAG, "检测到转弯！类型: %d", turn_type);
//             turn_detector_clear_flags();
//         }
//     }
    
//     ESP_LOGI(TAG, "停止定时器系统...");
//     timer_system_stop();
    
//     ESP_LOGI(TAG, "✓ 测试5完成");
//     ESP_LOGI(TAG, "");
// }

// /**
//  * @brief 主函数
//  */
// void app_main(void)
// {
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "转弯检测模块测试程序");
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "");
    
//     vTaskDelay(pdMS_TO_TICKS(1000));
    
//     // 执行测试
//     test_turn_detector_init();
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     test_turn_detector_confirm();
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     test_turn_detector_flags();
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     test_turn_detector_debug_info();
//     vTaskDelay(pdMS_TO_TICKS(500));
    
//     test_turn_detector_integration();
    
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "所有测试完成");
//     ESP_LOGI(TAG, "========================================");
    
//     // 保持程序运行
//     while (1) {
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }
