/**
 * @file traffic_light.h
 * @brief 红绿灯控制模块
 * 
 * 使用两根信号线控制三个灯（红、黄、绿）
 * 控制逻辑：
 * - 10 (信号线1=高, 信号线2=低) → 红灯
 * - 01 (信号线1=低, 信号线2=高) → 黄灯
 * - 11 (信号线1=高, 信号线2=高) → 绿灯
 * 
 * 固定引脚：GPIO39（信号线1）、GPIO40（信号线2）
 */

#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include "driver/gpio.h"
#include "esp_err.h"
#include "pin_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

// ========================================
// 红绿灯状态枚举
// ========================================
typedef enum {
    TRAFFIC_LIGHT_RED,      // 红灯 (10)
    TRAFFIC_LIGHT_YELLOW,   // 黄灯 (01)
    TRAFFIC_LIGHT_GREEN     // 绿灯 (11)
} traffic_light_state_t;

// ========================================
// 函数声明
// ========================================

/**
 * @brief 初始化红绿灯GPIO引脚
 * 
 * 使用固定引脚：GPIO39（信号线1）、GPIO40（信号线2）
 * 
 * @return 
 *     - ESP_OK: 初始化成功
 *     - ESP_FAIL: GPIO配置失败
 */
esp_err_t traffic_light_init(void);

/**
 * @brief 设置红绿灯状态
 * 
 * @param state 目标状态（红/黄/绿）
 * 
 * @return 
 *     - ESP_OK: 设置成功
 *     - ESP_ERR_INVALID_STATE: 红绿灯未初始化
 */
esp_err_t traffic_light_set_state(traffic_light_state_t state);

/**
 * @brief 获取当前红绿灯状态
 * 
 * @return 当前状态
 */
traffic_light_state_t traffic_light_get_state(void);

#ifdef __cplusplus
}
#endif

#endif // TRAFFIC_LIGHT_H
