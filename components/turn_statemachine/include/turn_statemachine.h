/**
 * @file turn_statemachine.h
 * @brief 六状态转弯状态机 - 管理转弯过程的状态转换
 * 
 * 功能：
 * - 在Timer 1中断（10ms周期）中执行状态机逻辑
 * - 实现六个状态：IDLE、STOP、BACK、PHASE1、PHASE2、ADJUST
 * - 使用tick计数器替代延时函数（1 tick = 10ms）
 * - 使用原子操作保护状态变量
 */

#ifndef TURN_STATEMACHINE_H
#define TURN_STATEMACHINE_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_attr.h"  // 包含IRAM_ATTR宏定义

#ifdef __cplusplus
extern "C" {
#endif

// ==================== 状态定义 ====================

/**
 * @brief 转弯状态机状态枚举
 */
typedef enum {
    TURN_IDLE = 0,      // 空闲状态，等待转弯请求
    TURN_STOP,          // 停车状态，持续100ms（10 ticks）
    TURN_BACK,          // 后退状态，持续190ms（19 ticks）
    TURN_PHASE1,        // 转弯第一阶段，直到传感器离开黑线
    TURN_PHASE2,        // 转弯第二阶段，直到传感器找到新黑线
    TURN_ADJUST,        // 微调状态，停车100ms（10 ticks）后回到IDLE
} TurnState_t;

// ==================== 配置参数 ====================

// 状态持续时间（单位：tick，1 tick = 10ms）
#define TURN_STOP_TICKS      10   // 停车100ms
#define TURN_BACK_TICKS_DEFAULT  13   // 后退140ms（默认值）
// #define TURN_BACK_TICKS_DEFAULT  24   // 后退140ms（默认值）
#define TURN_ADJUST_TICKS    10   // 微调停车100ms

// 转弯第一阶段最小持续时间
#define TURN_PHASE1_MIN_TICKS_RIGHT  50   // 右转最少250ms
#define TURN_PHASE1_MIN_TICKS_LEFT   15   // 左转最少150ms

// 转弯速度
#define TURN_SPEED           500   // 转弯速度（0-1023）
#define TURN_BACK_SPEED      600   // 后退速度（0-1023）

// ==================== 公共接口 ====================

/**
 * @brief 初始化转弯状态机
 * 
 * 初始化状态变量和内部计数器
 */
void turn_statemachine_init(void);

/**
 * @brief 转弯状态机tick函数（在Timer 1中断中调用）
 * 
 * 执行状态机逻辑：
 * - 检查转弯请求
 * - 执行状态转换
 * - 控制电机运动
 * - 更新状态变量
 * 
 * 注意：
 * - 必须在Timer 1中断中调用（10ms周期）
 * - 使用IRAM_ATTR属性
 * - 执行时间不超过1毫秒
 */
void IRAM_ATTR turn_statemachine_tick(void);

/**
 * @brief 获取当前状态（线程安全）
 * 
 * @return 当前状态
 */
TurnState_t turn_statemachine_get_state(void);

/**
 * @brief 获取调试信息（用于测试）
 * 
 * @param tick_count 当前状态持续的tick数输出
 * @param turn_dir 转弯方向输出（1=左转，2=右转）
 */
void turn_statemachine_get_debug_info(uint16_t *tick_count, uint8_t *turn_dir);

/**
 * @brief 设置后退时间（单位：tick，1 tick = 10ms）
 * 
 * @param ticks 后退时间（建议：高速20，低速15）
 */
void turn_statemachine_set_back_ticks(uint8_t ticks);

/**
 * @brief 获取后退时间
 * 
 * @return 当前后退时间（tick数）
 */
uint8_t turn_statemachine_get_back_ticks(void);

#ifdef __cplusplus
}
#endif

#endif // TURN_STATEMACHINE_H
