/**
 * @file turn_detector.h
 * @brief 转弯检测模块 - 基于灰度传感器检测转弯标记
 * 
 * 功能：
 * - 在Timer 0中断（1ms周期）中执行转弯检测
 * - 使用阈值判断灰度传感器是否在黑线上
 * - 实现连续15次确认机制
 * - 使用原子变量保护共享数据
 */

#ifndef TURN_DETECTOR_H
#define TURN_DETECTOR_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_attr.h"  // 包含IRAM_ATTR宏定义

#ifdef __cplusplus
extern "C" {
#endif

// ==================== 配置参数 ====================

// 阈值定义（基于设计文档）
#define LEFT_THRESHOLD   ((4095 + 1476) / 2)  // 2785
#define RIGHT_THRESHOLD  ((4095 + 1546) / 2)  // 2820
#define CONFIRM_COUNT    3  // 连续确认次数

// 转弯类型定义
#define TURN_TYPE_NONE       0  // 无转弯
#define TURN_TYPE_LEFT       1  // 左转（可选功能）
#define TURN_TYPE_RIGHT      2  // 右转（可选功能）
#define TURN_TYPE_CROSS      3  // 十字路口

// ==================== 公共接口 ====================

/**
 * @brief 初始化转弯检测模块
 * 
 * 初始化共享变量和计数器
 */
void turn_detector_init(void);

/**
 * @brief 转弯检测tick函数（在Timer 0中断中调用）
 * 
 * 执行转弯检测逻辑：
 * - 读取缓存的ADC值
 * - 使用阈值判断左右传感器是否在黑线上
 * - 实现连续15次确认机制
 * - 更新转弯类型和转弯进行中标志
 * 
 * 注意：
 * - 必须在Timer 0中断中调用（1ms周期）
 * - 使用IRAM_ATTR属性
 * - 执行时间不超过200微秒
 */
void IRAM_ATTR turn_detector_tick(void);

/**
 * @brief 获取转弯类型（线程安全）
 * 
 * @return 转弯类型：0=无，1=左转，2=右转，3=十字路口
 */
uint8_t turn_detector_get_type(void);

/**
 * @brief 检查是否正在转弯（线程安全）
 * 
 * @return true=正在转弯，false=未转弯
 */
bool turn_detector_is_turning(void);

/**
 * @brief 清除转弯标志（由状态机调用）
 * 
 * 在转弯完成后，状态机调用此函数清除转弯标志
 */
void turn_detector_clear_flags(void);

/**
 * @brief 设置转弯进行中标志（由状态机调用）
 * 
 * 在转弯开始时，状态机调用此函数设置标志
 * 
 * @param in_progress true=转弯进行中，false=转弯结束
 */
void turn_detector_set_turning(bool in_progress);

/**
 * @brief 获取调试信息（用于测试）
 * 
 * @param both_count 双传感器计数器输出
 * @param left_count 左传感器计数器输出
 * @param right_count 右传感器计数器输出
 */
void turn_detector_get_debug_info(uint8_t *both_count, uint8_t *left_count, uint8_t *right_count);

#ifdef __cplusplus
}
#endif

#endif // TURN_DETECTOR_H
