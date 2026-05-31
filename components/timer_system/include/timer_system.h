/**
 * @file timer_system.h
 * @brief ESP32硬件定时器系统 - 用于实时任务调度
 * 
 * 功能：
 * - Timer 0: 1ms周期，用于灰度传感器扫描和转弯检测
 * - Timer 1: 10ms周期，用于PD控制和转弯状态机
 */

#ifndef TIMER_SYSTEM_H
#define TIMER_SYSTEM_H

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ==================== 配置参数 ====================

#define TIMER_GROUP_NUM         TIMER_GROUP_0
#define TIMER_0_NUM             TIMER_0
#define TIMER_1_NUM             TIMER_1
#define TIMER_0_INTERVAL_MS     1      // Timer 0周期：1ms
#define TIMER_1_INTERVAL_MS     10     // Timer 1周期：10ms
#define TIMER_0_PRIORITY        1      // Timer 0中断优先级（最高）
#define TIMER_1_PRIORITY        2      // Timer 1中断优先级（次高）

// ==================== 公共接口 ====================

/**
 * @brief 初始化定时器系统
 * 
 * 配置Timer 0和Timer 1，注册中断处理函数，但不启动定时器
 * 
 * @return 
 *   - ESP_OK: 初始化成功
 *   - ESP_FAIL: 初始化失败
 */
esp_err_t timer_system_init(void);

/**
 * @brief 启动定时器系统
 * 
 * 同时启动Timer 0和Timer 1
 * 
 * @return 
 *   - ESP_OK: 启动成功
 *   - ESP_FAIL: 启动失败
 */
esp_err_t timer_system_start(void);

/**
 * @brief 停止定时器系统
 * 
 * 同时停止Timer 0和Timer 1
 * 
 * @return 
 *   - ESP_OK: 停止成功
 *   - ESP_FAIL: 停止失败
 */
esp_err_t timer_system_stop(void);

/**
 * @brief 获取Timer 0中断计数器（用于调试）
 * 
 * @return Timer 0中断次数
 */
uint32_t timer_system_get_timer0_count(void);

/**
 * @brief 获取Timer 1中断计数器（用于调试）
 * 
 * @return Timer 1中断次数
 */
uint32_t timer_system_get_timer1_count(void);

/**
 * @brief 进入安全模式
 * 
 * 当系统检测到严重错误时调用此函数：
 * - 停止所有电机（设置速度为0）
 * - 记录错误日志
 * - 可选：触发蜂鸣器报警
 * - 进入死循环等待重启
 * 
 * 注意：此函数不会返回
 */
void enter_safe_mode(void);

/**
 * @brief 初始化看门狗定时器
 * 
 * 配置Task Watchdog Timer (TWDT)：
 * - 超时时间：5秒
 * - 监控IDLE任务
 * - 超时触发panic重启
 * 
 * @return 
 *   - ESP_OK: 初始化成功
 *   - ESP_FAIL: 初始化失败
 */
esp_err_t watchdog_init(void);

#ifdef __cplusplus
}
#endif

#endif // TIMER_SYSTEM_H
