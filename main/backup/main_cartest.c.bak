// /*
//  * ESP32巡线小车 - 单任务顺序执行版本
//  *
//  * 功能说明：
//  * 1. 初始化灰度传感器、电机
//  * 2. 在主循环中顺序执行：读传感器 -> 判断转弯 -> 巡线控制 -> 延时
//  */

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_log.h"
// #include "esp_system.h"
// #include "esp_task_wdt.h"
// #include "esp_timer.h"

// // 巡线相关头文件
// #include "line_following.h"
// #include "gray_sensor.h"
// #include "pwm.h"

// static const char *TAG = "巡线小车";

// /**
//  * @brief 应用主函数
//  */
// void app_main(void)
// {
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "ESP32巡线小车启动 - 单任务版本");
//     ESP_LOGI(TAG, "========================================");

//     // 1. 初始化硬件模块
//     ESP_LOGI(TAG, "初始化硬件模块...");

//     // 初始化灰度传感器（不启动采样任务）
//     gray_sensor_init_simple();

//     // 初始化PWM电机控制
//     ledc_init();

//     ESP_LOGI(TAG, "硬件初始化完成");

//     // 2. 传感器校准
//     ESP_LOGI(TAG, "设置传感器校准参数...");
//     gray_sensor_set_calibration(GRAY_SENSOR_LEFT, 4095, 1150);  // 左传感器
//     gray_sensor_set_calibration(GRAY_SENSOR_RIGHT, 4095, 1150); // 右传感器

//     // 3. 等待系统稳定
//     ESP_LOGI(TAG, "系统准备中...");
//     vTaskDelay(pdMS_TO_TICKS(1000));

//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "巡线小车运行中...");
//     ESP_LOGI(TAG, "========================================");

//     // 巡线参数
//     uint16_t speed = 717; // 基础速度 (0-1023，约70%占空比)
//     float kp = 5.0f;      // 比例系数
//     float kd = 12.0f;     // 微分系数

//     // 转弯检测变量
//     uint16_t left_threshold = (4095 + 1476) / 2;
//     uint16_t right_threshold = (4095 + 1546) / 2;
//     uint8_t both_on_count = 0;
//     const uint8_t CONFIRM_THRESHOLD = 3;
//     bool turning_in_progress = false;

//     uint32_t loop_count = 0;

//     // 主循环：顺序执行
//     while (1)
//     {
//         loop_count++;

//         // ========== 1. 读取传感器数据 ==========
//         uint16_t left_raw, right_raw;
//         if (!gray_sensor_read_both_raw_direct(&left_raw, &right_raw))
//         {
//             // 读取失败，跳过本次循环
//             vTaskDelay(pdMS_TO_TICKS(10));
//             continue;
//         }

//         // ========== 2. 判断是否需要转弯 ==========
//         bool left_on_black = (left_raw < left_threshold);
//         bool right_on_black = (right_raw < right_threshold);

//         if (!turning_in_progress)
//         {
//             if (left_on_black && right_on_black)
//             {
//                 // 两个都在黑线上 - 可能是十字路口
//                 both_on_count++;
//                 if (both_on_count >= CONFIRM_THRESHOLD)
//                 {
//                     turning_in_progress = true;
//                     ESP_LOGI(TAG, "检测到十字路口，执行右转");

//                     // 执行十字路口处理
//                     car_stop();
//                     vTaskDelay(pdMS_TO_TICKS(100));
//                     car_move_backward(614);
//                     vTaskDelay(pdMS_TO_TICKS(200));
//                     car_stop();
//                     vTaskDelay(pdMS_TO_TICKS(100));

//                     // 执行右转
//                     execute_right_turn_simple(speed);

//                     // 重置状态
//                     both_on_count = 0;
//                     turning_in_progress = false;
//                 }
//             }
//             else
//             {
//                 both_on_count = 0;
//             }
//         }

//         // ========== 3. 执行巡线控制 ==========
//         if (!turning_in_progress)
//         {
//             line_following_pd_control_with_values(left_raw, right_raw, speed, kp, kd);
//         }
//         vTaskDelay(pdMS_TO_TICKS(5));

//         // //    esp_rom_delay_us(10000);
//     }
// }