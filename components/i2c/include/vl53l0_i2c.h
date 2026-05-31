/**
 * @file vl53l0_i2c.h
 * @brief VL53L0X激光测距传感器的软件I2C驱动头文件
 */

#ifndef VL53L0_I2C_H
#define VL53L0_I2C_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化VL53L0X的GPIO引脚
 * 
 * 配置SCL和SDA引脚为开漏输出模式
 */
void vl53l0_i2c_init(void);

/**
 * @brief 将SDA引脚设置为输入模式
 */
void vl53l0_sda_set_input(void);

/**
 * @brief 将SDA引脚设置为输出模式
 */
void vl53l0_sda_set_output(void);

/**
 * @brief 设置SCL引脚为高电平
 */
void vl53l0_scl_high(void);

/**
 * @brief 设置SCL引脚为低电平
 */
void vl53l0_scl_low(void);

/**
 * @brief 设置SDA引脚为高电平
 */
void vl53l0_sda_high(void);

/**
 * @brief 设置SDA引脚为低电平
 */
void vl53l0_sda_low(void);

/**
 * @brief 读取SDA引脚电平
 * 
 * @return 1=高电平，0=低电平
 */
uint8_t vl53l0_sda_read(void);

#ifdef __cplusplus
}
#endif

#endif // VL53L0_I2C_H
