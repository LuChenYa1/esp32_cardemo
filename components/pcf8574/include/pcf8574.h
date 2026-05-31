/**
 * @file pcf8574.h
 * @brief PCF8574 I2C GPIO 扩展芯片驱动
 * 
 * 使用 ESP-IDF 5.3+ 的新 I2C master 驱动
 * 兼容 managed_components 中的 API 接口
 */

#ifndef PCF8574_H
#define PCF8574_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief I2C 设备描述符（兼容 i2cdev 接口）
 */
typedef struct {
    i2c_master_bus_handle_t bus_handle;   // I2C 总线句柄
    i2c_master_dev_handle_t dev_handle;   // I2C 设备句柄
    uint8_t addr;                         // I2C 地址
    gpio_num_t sda_io_num;                // SDA 引脚
    gpio_num_t scl_io_num;                // SCL 引脚
    i2c_port_t port;                      // I2C 端口号
} i2c_dev_t;

/**
 * @brief 初始化 PCF8574 设备描述符
 * 
 * @param dev 指向 I2C 设备描述符的指针
 * @param addr I2C 地址 (PCF8574: 0x20-0x27, PCF8574A: 0x38-0x3F)
 * @param port I2C 端口号
 * @param sda_gpio SDA 引脚
 * @param scl_gpio SCL 引脚
 * @return 
 *     - ESP_OK 成功
 *     - ESP_ERR_INVALID_ARG 参数无效
 *     - ESP_ERR_NO_MEM 内存不足
 */
esp_err_t pcf8574_init_desc(i2c_dev_t *dev, uint8_t addr, i2c_port_t port, 
                            gpio_num_t sda_gpio, gpio_num_t scl_gpio);

/**
 * @brief 释放 PCF8574 设备描述符
 * 
 * @param dev 指向 I2C 设备描述符的指针
 * @return 
 *     - ESP_OK 成功
 *     - ESP_ERR_INVALID_ARG 参数无效
 */
esp_err_t pcf8574_free_desc(i2c_dev_t *dev);

/**
 * @brief 读取 GPIO 端口值（8位）
 * 
 * @param dev 指向 I2C 设备描述符的指针
 * @param val 读取的 8 位 GPIO 端口值
 * @return 
 *     - ESP_OK 成功
 *     - ESP_ERR_INVALID_ARG 参数无效
 */
esp_err_t pcf8574_port_read(i2c_dev_t *dev, uint8_t *val);

/**
 * @brief 写入 GPIO 端口值（8位）
 * 
 * @param dev 指向 I2C 设备描述符的指针
 * @param value GPIO 端口值
 * @return 
 *     - ESP_OK 成功
 *     - ESP_ERR_INVALID_ARG 参数无效
 */
esp_err_t pcf8574_port_write(i2c_dev_t *dev, uint8_t value);

/**
 * @brief 读取单个引脚电平
 * 
 * @param dev 指向设备描述符的指针
 * @param pin 引脚编号 (0-7)
 * @param val 引脚电平 (1=高, 0=低)
 * @return 
 *     - ESP_OK 成功
 *     - ESP_ERR_INVALID_ARG 参数无效
 */
esp_err_t pcf8574_get_level(i2c_dev_t *dev, uint8_t pin, uint32_t *val);

/**
 * @brief 设置单个引脚电平
 * 
 * 引脚必须配置为输出模式
 * 
 * @param dev 指向设备描述符的指针
 * @param pin 引脚编号 (0-7)
 * @param val 引脚电平 (1=高, 0=低)
 * @return 
 *     - ESP_OK 成功
 *     - ESP_ERR_INVALID_ARG 参数无效
 */
esp_err_t pcf8574_set_level(i2c_dev_t *dev, uint8_t pin, uint32_t val);

#ifdef __cplusplus
}
#endif

#endif // PCF8574_H
