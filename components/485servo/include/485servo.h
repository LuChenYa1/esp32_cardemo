#ifndef SERVO485_H
#define SERVO485_H

#include <stdint.h>
#include "driver/gpio.h"
#include "pin_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

// RS485方向控制引脚定义在 pin_definitions.h 中
// #define RS485_DIR_GPIO         GPIO_NUM_19
// #define RS485_DIR_TX_LEVEL     1
// #define RS485_DIR_RX_LEVEL     0

/**
 * @brief 初始化485伺服模块
 * 
 * 配置485通信所需的GPIO引脚、UART参数等，
 * 并设置初始状态为接收模式。
 */
void servo485_init(void);

/**
 * @brief 反初始化485伺服模块
 * 
 * 释放485通信相关的资源，包括GPIO和UART，
 * 确保模块安全关闭。
 */
void servo485_deinit(void);

/**
 * @brief 设置伺服电机的位置
 * 
 * @param id 伺服电机的ID（用于区分多个伺服设备）
 * @param position 目标位置（单位：角度或脉冲数，具体取决于伺服协议）
 * @param speed 运动速度（单位：rpm或百分比，具体取决于伺服协议）
 * 
 * 向指定ID的伺服电机发送位置控制指令。
 */
void Set_Servo_position(uint8_t id, uint16_t position, uint8_t speed);

/**
 * @brief 读取伺服电机的当前位置
 * 
 * @param id 伺服电机的ID（用于区分多个伺服设备）
 * 
 * 向指定ID的伺服电机发送位置查询指令，并等待返回当前实际位置。
 */
void Read_Servo_position(uint8_t id);
#ifdef __cplusplus
}
#endif

#endif // SERVO485_H