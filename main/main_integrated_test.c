/*
 * ESP32 综合测试程序
 *
 * 功能：
 * 1. 巡线小车（主任务，高优先级）- PD控制 + 十字路口右转
 * 2. 数码管轮播显示：5s距离 / 5s温湿度 / 5s触摸状态
 * 3. 485舵机往复运动（独立任务）
 *
 * 注意：485使用UART0，需关闭日志输出
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

// 巡线
#include "line_following.h"
#include "gray_sensor.h"
#include "pwm.h"

// 传感器 & 显示
#include "i2c.h"
#include "vl53l0.h"
#include "tm1637.h"
#include "dht11.h"
#include "ssax1.h"

// 485舵机
#include "485servo.h"

static const char *TAG = "综合测试";

// ==================== 巡线参数 ====================
#define LINE_SPEED          700     // 基础速度 (0-1023)
#define LINE_KP             5.0f    // 比例系数
#define LINE_KD             12.0f   // 微分系数
#define LEFT_WHITE          4095
#define LEFT_BLACK          1476
#define RIGHT_WHITE         4095
#define RIGHT_BLACK         1546
#define LEFT_THRESHOLD      ((LEFT_WHITE + LEFT_BLACK) / 2)
#define RIGHT_THRESHOLD     ((RIGHT_WHITE + RIGHT_BLACK) / 2)
#define CROSSROAD_CONFIRM   3       // 连续N次检测到双黑才触发十字路口

// ==================== 485舵机参数 ====================
#define SERVO_ID            0xFE    // 广播地址，控制所有舵机
#define SERVO_POS_A         400
#define SERVO_POS_B         600
#define SERVO_SPEED         50
#define SERVO_INTERVAL_MS   2000

// ==================== 数码管轮播显示任务 ====================
// 轮播模式：0=距离, 1=温湿度, 2=触摸状态
#define DISPLAY_INTERVAL_MS  5000   // 每个模式显示5秒
#define DISPLAY_REFRESH_MS   200    // 刷新间隔200ms

/**
 * @brief 数码管轮播任务
 * 每5秒切换显示：
 *   模式0 - 距离，单位cm，范围 0~9999
 *   模式1 - 温湿度，格式 TTHH（前两位=温度℃，后两位=湿度%），范围 0099~5090
 *   模式2 - 触摸状态，显示 0 或 1
 */
void display_task(void *pvParameters)
{
    // 初始化 VL53L0X
    Gir_distance_sensor_init();
    vTaskDelay(pdMS_TO_TICKS(1000));
    Gir_setMode(0); // 高精度模式
    vTaskDelay(pdMS_TO_TICKS(500));

    // 初始化触摸传感器 GPIO37
    ssax1_gpio_init();

    uint8_t  mode           = 0;
    uint32_t mode_ticks     = 0;
    // 每个模式持续 5000ms / 200ms = 25 次刷新
    const uint32_t ticks_per_mode = DISPLAY_INTERVAL_MS / DISPLAY_REFRESH_MS;

    while (1) {
        uint16_t disp_val = 0;

        switch (mode) {
            case 0: {
                // 距离（cm），VL53L0X 量程约 0~200cm，限制在 9999 以内防溢出
                float dist = getDistance();
                if (dist < 0.0f)    dist = 0.0f;
                if (dist > 9999.0f) dist = 9999.0f;
                disp_val = (uint16_t)dist;
                break;
            }
            case 1: {
                // 温湿度合并显示：TTHH
                // 温度 0~50℃（2位），湿度 20~90%（2位），合并后最大 5090，不超过9999
                int16_t temp_raw, humi_raw;
                if (dht11_read_data(DHT11_PIN_1, &humi_raw, &temp_raw) == ESP_OK) {
                    uint8_t temp = temp_raw / 10;  // 转换为整数部分
                    uint8_t humi = humi_raw / 10;  // 转换为整数部分
                    // 限制范围防止异常值溢出
                    if (temp > 99) temp = 99;
                    if (humi > 99) humi = 99;
                    disp_val = (uint16_t)(temp * 100 + humi); // 最大 9999
                }
                break;
            }
            case 2: {
                // 触摸传感器：0 或 1
                disp_val = (uint16_t)gpio_get_level(GPIO_NUM_37);
                break;
            }
        }

        tm1637_disp_num_process(disp_val);

        mode_ticks++;
        if (mode_ticks >= ticks_per_mode) {
            mode_ticks = 0;
            mode = (mode + 1) % 3; // 循环切换 0->1->2->0
        }

        vTaskDelay(pdMS_TO_TICKS(DISPLAY_REFRESH_MS));
    }
}

// ==================== 485舵机任务 ====================

/**
 * @brief 485舵机往复运动任务
 */
void servo485_task(void *pvParameters)
{
    servo485_init();
    vTaskDelay(pdMS_TO_TICKS(500));

    while (1) {
        Set_Servo_position(SERVO_ID, SERVO_POS_A, SERVO_SPEED);
        vTaskDelay(pdMS_TO_TICKS(SERVO_INTERVAL_MS));

        Set_Servo_position(SERVO_ID, SERVO_POS_B, SERVO_SPEED);
        vTaskDelay(pdMS_TO_TICKS(SERVO_INTERVAL_MS));
    }
}

// ==================== app_main ====================

void app_main(void)
{
    // 初始化前先打印启动信息（日志关闭前）
    ESP_LOGI(TAG, "ESP32 综合测试启动");

    // ---------- 初始化 ----------
    gray_sensor_init_simple();
    gray_sensor_set_calibration(GRAY_SENSOR_LEFT,  LEFT_WHITE,  LEFT_BLACK);
    gray_sensor_set_calibration(GRAY_SENSOR_RIGHT, RIGHT_WHITE, RIGHT_BLACK);

    ledc_init();
    i2c_init();

    vTaskDelay(pdMS_TO_TICKS(500));

    // 关闭所有日志，因为UART0是默认日志输出口，会影响485通信
    esp_log_level_set("*", ESP_LOG_NONE);

    // ---------- 启动辅助任务 ----------
    xTaskCreate(display_task, "display",  4096, NULL, 3, NULL);
    xTaskCreate(servo485_task, "servo485", 3072, NULL, 3, NULL);

    // ---------- 巡线主循环 ----------
    uint8_t both_on_count = 0;
    bool    turning       = false;

    while (1) {
        uint16_t left_raw, right_raw;

        if (!gray_sensor_read_both_raw_direct(&left_raw, &right_raw)) 
        {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        //打印adc
        ESP_LOGI(TAG, "adc: %d, %d", left_raw, right_raw);

        bool left_black  = (left_raw  < 2400);
        bool right_black = (right_raw < 2400);

        if (!turning) {
            if (left_black && right_black) {
                both_on_count++;
                if (both_on_count >= CROSSROAD_CONFIRM) {
                    turning = true;

                    car_stop();
                    vTaskDelay(pdMS_TO_TICKS(100));
                    car_move_backward(600);
                    vTaskDelay(pdMS_TO_TICKS(200));
                    car_stop();
                    vTaskDelay(pdMS_TO_TICKS(100));

                    execute_right_turn_simple(LINE_SPEED);

                    both_on_count = 0;
                    turning       = false;
                }
            } else {
                both_on_count = 0;
            }
        }

        if (!turning) {
            line_following_pd_control_with_values(
                left_raw, right_raw, LINE_SPEED, LINE_KP, LINE_KD);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
