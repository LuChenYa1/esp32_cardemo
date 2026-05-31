#ifndef LINE_FOLLOWING_H
#define LINE_FOLLOWING_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化巡线控制模块
 * 
 * 初始化所需的硬件模块（PWM、编码器、灰度传感器）
 */
void line_following_init(void);

/**
 * @brief PD控制巡线
 * 
 * 使用比例-微分控制算法进行巡线
 * 
 * @param speed 基础速度 (0-1023，直接对应硬件占空比)
 * @param kp 比例系数
 * @param kd 微分系数
 */
void line_following_pd_control(uint16_t speed, float kp, float kd);

/**
 * @brief PD控制巡线（使用传入的传感器值）
 * 
 * 使用比例-微分控制算法进行巡线，传感器值由调用者提供
 * 
 * @param left_raw 左传感器原始值
 * @param right_raw 右传感器原始值
 * @param speed 基础速度 (0-1023，直接对应硬件占空比)
 * @param kp 比例系数
 * @param kd 微分系数
 */
void line_following_pd_control_with_values(uint16_t left_raw, uint16_t right_raw, 
                                           uint16_t speed, float kp, float kd);

/**
 * @brief 简单巡线（无PID）
 * 
 * 基于传感器状态的简单巡线算法
 * 
 * @param speed 基础速度 (0-1023，直接对应硬件占空比)
 */
void line_following_simple(uint16_t speed);

/**
 * @brief 带转弯检测的巡线
 * 
 * 结合转弯检测模块，自动处理转弯
 * 
 * @param speed 基础速度 (0-1023，直接对应硬件占空比)
 * @param kp 比例系数
 * @param kd 微分系数
 */
void line_following_with_turns(uint16_t speed, float kp, float kd);

/**
 * @brief 执行左转90度
 * 
 * @param speed 转弯速度 (0-1023，直接对应硬件占空比)
 */
void execute_left_turn(uint16_t speed);

/**
 * @brief 执行右转90度
 * 
 * @param speed 转弯速度 (0-1023，直接对应硬件占空比)
 */
void execute_right_turn(uint16_t speed);

/**
 * @brief 执行右转90度（简化版本）
 * 
 * @param speed 转弯速度 (0-1023，直接对应硬件占空比)
 */
void execute_right_turn_simple(uint16_t speed);

/**
 * @brief 执行十字路口处理
 * 
 * 默认策略：停止 → 后退 → 右转
 * 
 * @param speed 速度 (0-1023，直接对应硬件占空比)
 */
void execute_crossroad(uint16_t speed);

/**
 * @brief 车辆前进
 * 
 * @param speed 速度 (0-1023，直接对应硬件占空比)，正值前进
 */
void car_move_forward(int16_t speed);

/**
 * @brief 车辆后退
 * 
 * @param speed 速度 (0-1023，直接对应硬件占空比)，正值后退
 */
void car_move_backward(int16_t speed);

/**
 * @brief 车辆左转
 * 
 * 原地左转：左轮反转，右轮正转
 * 
 * @param speed 速度 (0-1023，直接对应硬件占空比)
 */
void car_turn_left(int16_t speed);

/**
 * @brief 车辆右转
 * 
 * 原地右转：左轮正转，右轮反转
 * 
 * @param speed 速度 (0-1023，直接对应硬件占空比)
 */
void car_turn_right(int16_t speed);

/**
 * @brief 车辆停止
 */
void car_stop(void);

/**
 * @brief 设置所有车轮速度
 * 
 * @param front_left 左前轮速度 (-1023 到 +1023，直接对应硬件占空比)
 * @param front_right 右前轮速度 (-1023 到 +1023，直接对应硬件占空比)
 * @param rear_left 左后轮速度 (-1023 到 +1023，直接对应硬件占空比)
 * @param rear_right 右后轮速度 (-1023 到 +1023，直接对应硬件占空比)
 */
void set_all_wheels(int16_t front_left, int16_t front_right, 
                    int16_t rear_left, int16_t rear_right);

#ifdef __cplusplus
}
#endif

#endif // LINE_FOLLOWING_H
