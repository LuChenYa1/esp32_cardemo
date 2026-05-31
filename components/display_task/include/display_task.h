/**
 * @file display_task.h
 * @brief 数码管轮播显示任务头文件
 * 
 * 功能：
 * - 每5秒自动切换显示模式
 * - 每200ms刷新一次数码管显示
 * - 支持多种传感器数据轮播显示
 */

#ifndef DISPLAY_TASK_H
#define DISPLAY_TASK_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* ==================== 配置参数 ==================== */

// 任务优先级（低于Timer中断）
#define DISPLAY_TASK_PRIORITY       3

// 任务栈大小
#define DISPLAY_TASK_STACK_SIZE     4096

// 显示刷新间隔（毫秒）
#define DISPLAY_REFRESH_MS          200

// 模式切换间隔（毫秒）
#define DISPLAY_MODE_INTERVAL_MS    5000

/* ==================== 显示模式枚举 ==================== */

/**
 * @brief 显示模式枚举
 */
typedef enum {
    DISPLAY_MODE_TEMP_HUMI = 0,     // 温湿度模式（格式：TTHH）
    DISPLAY_MODE_ULTRASONIC,        // 超声波测距模式（单位：cm）
    DISPLAY_MODE_IR_OBSTACLE,       // 红外避障状态模式（0=无障碍，1=有障碍）
    DISPLAY_MODE_COUNT              // 模式总数
} DisplayMode_t;

/* ==================== 对外接口 ==================== */

/**
 * @brief 数码管轮播显示任务入口函数
 * @param pvParameters 任务参数（未使用）
 * 
 * 功能：
 * - 初始化所有传感器（TM1637、DHT11、HC-SR04、红外避障）
 * - 每200ms刷新数码管显示
 * - 每5秒切换显示模式
 * - DHT11读取失败时保持上次有效值
 */
void display_task(void *pvParameters);

/**
 * @brief 创建数码管显示任务
 * @return ESP_OK 成功，ESP_FAIL 失败
 */
esp_err_t display_task_create(void);

#endif /* DISPLAY_TASK_H */
