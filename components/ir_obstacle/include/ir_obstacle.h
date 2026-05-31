#ifndef IR_OBSTACLE_H
#define IR_OBSTACLE_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "pin_definitions.h"

// 红外避障传感器引脚定义在 pin_definitions.h 中
// #define IR_OBSTACLE_GPIO GPIO_NUM_38

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 红外避障传感器状态
 */
typedef enum {
    IR_OBSTACLE_CLEAR = 0,      // 无障碍物（传感器输出0）
    IR_OBSTACLE_DETECTED = 1    // 检测到障碍物（传感器输出1）
} ir_obstacle_state_t;

/**
 * @brief 初始化红外避障传感器
 * 
 * 配置GPIO38（SSA1）作为输入引脚，启用上拉电阻
 * 传感器输出：1=需要避障，0=无障碍
 */
void ir_obstacle_init(void);

/**
 * @brief 读取红外避障传感器状态
 * 
 * @return ir_obstacle_state_t 传感器状态
 *         - IR_OBSTACLE_CLEAR (0): 无障碍物
 *         - IR_OBSTACLE_DETECTED (1): 检测到障碍物
 */
ir_obstacle_state_t ir_obstacle_read(void);

/**
 * @brief 检测是否有障碍物
 * 
 * @return true 检测到障碍物（需要避障）
 *         false 无障碍物（可以通行）
 */
bool ir_obstacle_is_detected(void);

/**
 * @brief 打印传感器状态（调试用）
 */
void ir_obstacle_print_status(void);

#ifdef __cplusplus
}
#endif

#endif // IR_OBSTACLE_H
