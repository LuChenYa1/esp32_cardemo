/**
 * @file board_config.c
 * @brief 板级配置模块实现
 */

#include "board_config.h"
#include "gpio_manager.h"
#include "pin_definitions.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "BOARD_CONFIG";

// ========================================
// 板级配置接口实现
// ========================================

/**
 * @brief 注册所有系统GPIO
 * 
 * @deprecated 此函数已废弃，不再使用
 * 现在由各组件在自己的 xxx_init() 函数中注册GPIO
 */
esp_err_t board_config_register_all_pins(void)
{
    ESP_LOGW(TAG, "board_config_register_all_pins() 已废弃");
    ESP_LOGW(TAG, "请在各组件的 xxx_init() 函数中调用 gpio_manager_register()");
    return ESP_OK;
}

/**
 * @brief 初始化板级配置系统
 * 
 * 只初始化GPIO管理器，不预先注册任何GPIO
 * 各组件在自己的 xxx_init() 函数中调用 gpio_manager_register() 注册GPIO
 */
esp_err_t board_config_init(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "初始化板级配置系统");
    ESP_LOGI(TAG, "========================================");
    
    // 初始化GPIO管理器
    esp_err_t ret = gpio_manager_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO管理器初始化失败");
        return ret;
    }
    
    ESP_LOGI(TAG, "GPIO管理器已就绪，等待各组件注册GPIO");
    ESP_LOGI(TAG, "提示：各组件在初始化时会自动注册GPIO并检测冲突");
    return ESP_OK;
}

/**
 * @brief 获取板级信息
 */
esp_err_t board_config_get_info(board_info_t *info)
{
    if (info == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    info->board_name = "ESP32-S3 巡线小车";
    info->board_version = "v1.0 (飞线修改版)";
    info->total_gpio_count = 40;
    
    // 统计已分配的GPIO数量
    uint32_t count = 0;
    for (int i = 0; i < 40; i++) {
        if (gpio_manager_is_allocated(i, NULL)) {
            count++;
        }
    }
    info->allocated_gpio_count = count;
    
    return ESP_OK;
}

/**
 * @brief 打印板级配置摘要
 */
void board_config_print_summary(void)
{
    board_info_t info;
    board_config_get_info(&info);
    
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "板级配置摘要");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "板子名称: %s", info.board_name);
    ESP_LOGI(TAG, "硬件版本: %s", info.board_version);
    ESP_LOGI(TAG, "GPIO总数: %lu", info.total_gpio_count);
    ESP_LOGI(TAG, "已分配GPIO: %lu", info.allocated_gpio_count);
    ESP_LOGI(TAG, "未使用GPIO: %lu", info.total_gpio_count - info.allocated_gpio_count);
    ESP_LOGI(TAG, "========================================");
    
    // 打印详细的GPIO分配表
    gpio_manager_print_allocation_table();
}

/**
 * @brief 获取UART配置
 */
esp_err_t board_config_get_uart(uart_port_t uart_num, 
                                gpio_num_t *tx_pin, 
                                gpio_num_t *rx_pin,
                                uint32_t *baud_rate)
{
    if (tx_pin == NULL || rx_pin == NULL || baud_rate == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    switch (uart_num) {
        case UART_NUM_0:
            *tx_pin = UART0_TX_GPIO;
            *rx_pin = UART0_RX_GPIO;
            *baud_rate = UART0_BAUD_RATE;
            return ESP_OK;
            
        case UART_NUM_1:
            *tx_pin = UART1_TX_GPIO;
            *rx_pin = UART1_RX_GPIO;
            *baud_rate = UART1_BAUD_RATE;
            return ESP_OK;
            
        default:
            return ESP_ERR_INVALID_ARG;
    }
}

/**
 * @brief 获取I2C配置
 */
esp_err_t board_config_get_i2c(i2c_port_t i2c_num,
                               gpio_num_t *scl_pin,
                               gpio_num_t *sda_pin,
                               uint32_t *freq_hz)
{
    if (scl_pin == NULL || sda_pin == NULL || freq_hz == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (i2c_num == I2C_NUM_0) {
        *scl_pin = I2C_MASTER_SCL_GPIO;
        *sda_pin = I2C_MASTER_SDA_GPIO;
        *freq_hz = I2C_MASTER_FREQ_HZ;
        return ESP_OK;
    }
    
    return ESP_ERR_INVALID_ARG;
}

/**
 * @brief 获取RS485配置
 */
esp_err_t board_config_get_rs485(gpio_num_t *dir_pin,
                                 uint8_t *tx_level,
                                 uint8_t *rx_level)
{
    if (dir_pin == NULL || tx_level == NULL || rx_level == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *dir_pin = RS485_DIR_GPIO;
    *tx_level = RS485_DIR_TX_LEVEL;
    *rx_level = RS485_DIR_RX_LEVEL;
    
    return ESP_OK;
}
