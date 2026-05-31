/**
 * @file dht11.h
 * @brief DHT11 温湿度传感器驱动（基于 ESP-IDF）
 * 
 * 简化版 DHT11 驱动，参考 esp-idf-lib 的 dht 组件实现
 */

#ifndef DHT11_H
#define DHT11_H

#include <stdint.h>
#include <driver/gpio.h>
#include <esp_err.h>
#include "pin_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------- 引脚定义 ---------------- */
// DHT11引脚定义在 pin_definitions.h 中
// #define DHT11_DATA_GPIO  GPIO_NUM_38  // DHT11 默认连接到 SSA1 (GPIO38)

/**
 * @brief 读取 DHT11 传感器数据（整数格式）
 * 
 * 湿度和温度以整数形式返回，单位为 0.1
 * 例如：humidity=625 表示 62.5%，temperature=244 表示 24.4°C
 * 
 * @param pin GPIO 引脚编号
 * @param[out] humidity 湿度值（单位：0.1%），可为 NULL
 * @param[out] temperature 温度值（单位：0.1°C），可为 NULL
 * @return 
 *     - ESP_OK 成功
 *     - ESP_ERR_INVALID_ARG 参数错误
 *     - ESP_ERR_TIMEOUT 通信超时
 *     - ESP_ERR_INVALID_CRC 校验和错误
 */
esp_err_t dht11_read_data(gpio_num_t pin, int16_t *humidity, int16_t *temperature);

/**
 * @brief 读取 DHT11 传感器数据（浮点格式）
 * 
 * 湿度和温度以浮点数形式返回
 * 
 * @param pin GPIO 引脚编号
 * @param[out] humidity 湿度值（单位：%），可为 NULL
 * @param[out] temperature 温度值（单位：°C），可为 NULL
 * @return 
 *     - ESP_OK 成功
 *     - ESP_ERR_INVALID_ARG 参数错误
 *     - ESP_ERR_TIMEOUT 通信超时
 *     - ESP_ERR_INVALID_CRC 校验和错误
 */
esp_err_t dht11_read_float_data(gpio_num_t pin, float *humidity, float *temperature);

#ifdef __cplusplus
}
#endif

#endif /* DHT11_H */
