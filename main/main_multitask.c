/*
 * ESP32巡线小车 - 多任务版本
 *
 * 功能说明：
 * 1. 初始化灰度传感器、电机、编码器
 * 2. 传感器校准
 * 3. 启动转弯检测任务（独立FreeRTOS任务）
 * 4. 启动巡线任务（独立FreeRTOS任务）
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"

// 巡线相关头文件
#include "line_following.h"
#include "turn_detection.h"
#include "gray_sensor.h"
#include "pwm.h"

static const char *TAG = "巡线小车";

/**
 * @brief 巡线任务
 */
void line_following_task(void *pvParameters)
{
    ESP_LOGI(TAG, "巡线任务启动");

    // 巡线参数（可根据实际情况调整）
    uint16_t speed = 717; // 基础速度 (0-1023，约70%占空比)
    float kp = 5.0f;      // 比例系数
    float kd = 12.0f;     // 微分系数

    while (1)
    {
        // 带转弯检测的巡线
        line_following_with_turns(speed, kp, kd);
        // 延时已在 line_following_with_turns 内部处理
    }
}

/**
 * @brief 应用主函数
 */
void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "ESP32巡线小车启动 - 多任务版本");
    ESP_LOGI(TAG, "========================================");

    // 1. 初始化硬件模块
    ESP_LOGI(TAG, "初始化硬件模块...");

    // 初始化巡线控制（包含灰度传感器和PWM）
    line_following_init();

    // 初始化转弯检测
    turn_detection_init();

    ESP_LOGI(TAG, "硬件初始化完成");

    // 2. 设置传感器校准参数（根据实际测试结果调整）
    ESP_LOGI(TAG, "设置传感器校准参数...");
    gray_sensor_set_calibration(GRAY_SENSOR_LEFT, 4095, 1150);  // 左传感器
    gray_sensor_set_calibration(GRAY_SENSOR_RIGHT, 4095, 1150); // 右传感器

    // 3. 启动转弯检测任务
    turn_detection_start();

    // 4. 等待系统稳定
    ESP_LOGI(TAG, "系统准备中...");
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 5. 创建巡线任务
    ESP_LOGI(TAG, "启动巡线任务...");
    xTaskCreate(line_following_task, "line_following",
                configMINIMAL_STACK_SIZE * 4, NULL, 3, NULL);

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "巡线小车运行中...");
    ESP_LOGI(TAG, "========================================");

    // 主循环 - 定期打印传感器状态（调试用）
    while (1)
    {
        gray_sensor_print_status();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
