/**
 * @file board_config.h
 * @brief ESP32-S3 巡线小车板级配置模块
 * 
 * 本模块整合了GPIO管理、引脚定义和板级配置功能
 * 提供统一的硬件配置接口
 * 
 * 功能包括：
 * - GPIO分配表管理和冲突检测
 * - 统一的引脚定义
 * - 板级初始化接口
 * - 配置查询接口
 */

#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/gpio.h"
#include "pin_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

// ========================================
// GPIO功能类型枚举
// ========================================
typedef enum {
    GPIO_FUNC_UNUSED = 0,           // 未使用
    GPIO_FUNC_ADC,                  // ADC输入
    GPIO_FUNC_PWM,                  // PWM输出
    GPIO_FUNC_UART_TX,              // UART发送
    GPIO_FUNC_UART_RX,              // UART接收
    GPIO_FUNC_I2C_SDA,              // I2C数据线
    GPIO_FUNC_I2C_SCL,              // I2C时钟线
    GPIO_FUNC_GPIO_OUT,             // 通用GPIO输出
    GPIO_FUNC_GPIO_IN,              // 通用GPIO输入
    GPIO_FUNC_TOUCH,                // 触摸传感器
    GPIO_FUNC_DHT11,                // DHT11数据线
    GPIO_FUNC_TM1637_CLK,           // TM1637时钟
    GPIO_FUNC_TM1637_DIO,           // TM1637数据
    GPIO_FUNC_RS485_DIR,            // RS485方向控制
    GPIO_FUNC_ENCODER,              // 编码器输入
} gpio_function_t;

// ========================================
// GPIO分配信息结构体
// ========================================
typedef struct {
    gpio_num_t gpio_num;            // GPIO编号
    gpio_function_t function;       // 功能类型
    const char *module_name;        // 模块名称
    const char *description;        // 描述信息
} gpio_allocation_t;

// ========================================
// 板级配置信息结构体
// ========================================
typedef struct {
    const char *board_name;         // 板子名称
    const char *board_version;      // 硬件版本
    uint32_t total_gpio_count;      // GPIO总数
    uint32_t allocated_gpio_count;  // 已分配GPIO数量
} board_info_t;

// ========================================
// GPIO管理接口（保持向后兼容）
// ========================================

/**
 * @brief 初始化GPIO管理器
 * 
 * @return 
 *     - ESP_OK: 成功
 *     - ESP_FAIL: 失败
 */
esp_err_t gpio_manager_init(void);

/**
 * @brief 注册GPIO使用
 * 
 * @param gpio_num GPIO编号
 * @param function 功能类型
 * @param module_name 模块名称
 * @param description 描述信息
 * 
 * @return 
 *     - ESP_OK: 注册成功
 *     - ESP_ERR_INVALID_ARG: 无效的GPIO编号
 *     - ESP_ERR_INVALID_STATE: GPIO已被其他模块占用（冲突）
 */
esp_err_t gpio_manager_register(gpio_num_t gpio_num, 
                                gpio_function_t function,
                                const char *module_name,
                                const char *description);

/**
 * @brief 检查GPIO是否已被占用
 * 
 * @param gpio_num GPIO编号
 * @param[out] allocation 如果已占用，返回分配信息（可为NULL）
 * 
 * @return 
 *     - true: GPIO已被占用
 *     - false: GPIO未被占用
 */
bool gpio_manager_is_allocated(gpio_num_t gpio_num, gpio_allocation_t *allocation);

/**
 * @brief 打印GPIO分配表
 * 
 * 打印所有已分配的GPIO信息，用于调试和验证
 */
void gpio_manager_print_allocation_table(void);

/**
 * @brief 验证GPIO分配表的完整性
 * 
 * 检查是否有冲突或无效的分配
 * 
 * @return 
 *     - ESP_OK: 验证通过
 *     - ESP_FAIL: 发现冲突或错误
 */
esp_err_t gpio_manager_verify(void);

// ========================================
// 板级配置接口（新增）
// ========================================

/**
 * @brief 初始化板级配置系统
 * 
 * 包含GPIO管理器初始化、引脚注册和验证
 * 
 * @return 
 *     - ESP_OK: 初始化成功
 *     - ESP_FAIL: 初始化失败
 */
esp_err_t board_config_init(void);

/**
 * @brief 获取板级信息
 * 
 * @param[out] info 板级信息结构体指针
 * 
 * @return 
 *     - ESP_OK: 成功
 *     - ESP_ERR_INVALID_ARG: 参数无效
 */
esp_err_t board_config_get_info(board_info_t *info);

/**
 * @brief 打印板级配置摘要
 * 
 * 打印板子信息和GPIO分配统计
 */
void board_config_print_summary(void);

/**
 * @brief 注册所有系统GPIO
 * 
 * 根据pin_definitions.h中的定义，注册所有使用的GPIO
 * 
 * @return 
 *     - ESP_OK: 注册成功
 *     - ESP_FAIL: 发现冲突
 */
esp_err_t board_config_register_all_pins(void);

// ========================================
// 配置查询接口
// ========================================

/**
 * @brief 获取UART配置
 * 
 * @param uart_num UART端口号
 * @param[out] tx_pin 发送引脚
 * @param[out] rx_pin 接收引脚
 * @param[out] baud_rate 波特率
 * 
 * @return 
 *     - ESP_OK: 成功
 *     - ESP_ERR_INVALID_ARG: 无效的UART端口
 */
esp_err_t board_config_get_uart(uart_port_t uart_num, 
                                gpio_num_t *tx_pin, 
                                gpio_num_t *rx_pin,
                                uint32_t *baud_rate);

/**
 * @brief 获取I2C配置
 * 
 * @param i2c_num I2C端口号
 * @param[out] scl_pin 时钟引脚
 * @param[out] sda_pin 数据引脚
 * @param[out] freq_hz 频率
 * 
 * @return 
 *     - ESP_OK: 成功
 *     - ESP_ERR_INVALID_ARG: 无效的I2C端口
 */
esp_err_t board_config_get_i2c(i2c_port_t i2c_num,
                               gpio_num_t *scl_pin,
                               gpio_num_t *sda_pin,
                               uint32_t *freq_hz);

/**
 * @brief 获取RS485配置
 * 
 * @param[out] dir_pin 方向控制引脚
 * @param[out] tx_level 发送模式电平
 * @param[out] rx_level 接收模式电平
 * 
 * @return ESP_OK: 成功
 */
esp_err_t board_config_get_rs485(gpio_num_t *dir_pin,
                                 uint8_t *tx_level,
                                 uint8_t *rx_level);

#ifdef __cplusplus
}
#endif

#endif // BOARD_CONFIG_H
