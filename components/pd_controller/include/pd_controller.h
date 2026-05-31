/**
 * @file pd_controller.h
 * @brief PD控制器模块 - 实现巡线PD控制算法
 * 
 * 功能：
 * - 在Timer 1中断（10ms周期）中执行PD控制
 * - 读取缓存的ADC值并归一化
 * - 计算误差和PD输出
 * - 控制四个电机速度
 * - 转弯期间暂停PD控制
 */

#ifndef PD_CONTROLLER_H
#define PD_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_attr.h"  // 包含IRAM_ATTR宏定义

#ifdef __cplusplus
extern "C" {
#endif

// ==================== 配置参数 ====================

// 校准参数（来自设计文档）
#define LEFT_WHITE_VALUE    4095
#define LEFT_BLACK_VALUE    1269
#define RIGHT_WHITE_VALUE   4095
#define RIGHT_BLACK_VALUE   1354

// 默认PD参数
#define DEFAULT_SPEED       700
#define DEFAULT_KP          6.0f
#define DEFAULT_KD          30.0f

// 误差死区阈值
#define ERROR_DEADZONE      50.0f

// PWM速度范围
#define PWM_MIN_SPEED       0
#define PWM_MAX_SPEED       1023

// ==================== 公共接口 ====================

/**
 * @brief 初始化PD控制器
 * 
 * 初始化PD参数和状态变量
 */
void pd_controller_init(void);

/**
 * @brief PD控制器tick函数（在Timer 1中断中调用）
 * 
 * 执行PD控制算法：
 * 1. 检查转弯标志，转弯期间暂停控制
 * 2. 读取缓存的ADC值
 * 3. 归一化处理
 * 4. 计算误差（error = 510 * (right_norm - left_norm)）
 * 5. 应用误差死区（|error| < 30时，error = 0）
 * 6. 计算PD输出（output = Kp * error + Kd * (error - last_error)）
 * 7. 限制输出范围在[-speed, +speed]
 * 8. 计算左右轮速度并限制在[0, 1023]
 * 9. 调用PWM接口设置四个电机速度
 * 
 * 注意：
 * - 必须在Timer 1中断中调用（10ms周期）
 * - 使用IRAM_ATTR属性
 * - 执行时间不超过2毫秒
 */
void IRAM_ATTR pd_controller_tick(void);

/**
 * @brief 设置PD控制参数
 * 
 * @param speed 基础速度（0-1023）
 * @param kp 比例系数
 * @param kd 微分系数
 */
void pd_controller_set_params(uint16_t speed, float kp, float kd);

/**
 * @brief 获取当前PD参数
 * 
 * @param speed 速度输出
 * @param kp Kp输出
 * @param kd Kd输出
 */
void pd_controller_get_params(uint16_t *speed, float *kp, float *kd);

/**
 * @brief 获取上次误差值（用于调试）
 * 
 * @return 上次误差值
 */
float pd_controller_get_last_error(void);

/**
 * @brief 重置PD控制器状态
 * 
 * 重置last_error为0
 */
void pd_controller_reset(void);

/**
 * @brief 启用PD控制器
 * 
 * 启用后，pd_controller_tick()会正常执行PD控制
 */
void pd_controller_enable(void);

/**
 * @brief 禁用PD控制器
 * 
 * 禁用后，pd_controller_tick()会立即返回，不执行任何控制
 */
void pd_controller_disable(void);

/**
 * @brief 获取PD控制器使能状态
 * 
 * @return true 已启用，false 已禁用
 */
bool pd_controller_is_enabled(void);

#ifdef __cplusplus
}
#endif

#endif // PD_CONTROLLER_H
