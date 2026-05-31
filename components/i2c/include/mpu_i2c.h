/**
 * @file mpu_i2c.h
 * @brief MPU6050陀螺仪的硬件I2C驱动头文件
 * 
 * 所有引脚和配置定义来自 pin_definitions.h
 */

#ifndef MPU_I2C_H
#define MPU_I2C_H

#include <stdint.h>
#include "driver/i2c_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MPU6050的I2C总线初始化函数
 * 
 * @return ESP_OK表示初始化成功，其他值表示失败
 */
esp_err_t mpu_i2c_init(void);

/**
 * @brief 向MPU6050的指定寄存器写入单个字节数据
 * 
 * @param reg 目标寄存器地址
 * @param byte 要写入的数据字节
 * @return esp_err_t 写入操作的状态码
 */
esp_err_t mpu_write_byte(uint8_t reg, uint8_t byte);

/**
 * @brief 向MPU6050的指定寄存器写入多个字节数据
 * 
 * @param reg 起始寄存器地址
 * @param write_len 要写入的数据长度
 * @param buf 包含待写入数据的缓冲区指针
 * @return esp_err_t 写入操作的状态码
 */
esp_err_t mpu_write_buf(uint8_t reg, uint16_t write_len, uint8_t *buf);

/**
 * @brief 从MPU6050的指定寄存器读取单个字节数据
 * 
 * @param reg 要读取的寄存器地址
 * @return 读取到的数据字节，如果读取失败则返回0
 */
uint8_t mpu_read_byte(uint8_t reg);

/**
 * @brief 从MPU6050的指定寄存器开始读取多个字节数据
 * 
 * @param reg 起始寄存器地址
 * @param read_len 要读取的数据长度
 * @param read_buf 用于存储读取数据的缓冲区指针
 * @return esp_err_t 读取操作的状态码
 */
esp_err_t mpu_read_buf(uint8_t reg, uint16_t read_len, uint8_t *read_buf);

#ifdef __cplusplus
}
#endif

#endif // MPU_I2C_H
