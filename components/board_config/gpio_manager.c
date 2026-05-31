/**
 * @file gpio_manager.c
 * @brief GPIO资源分配和冲突检测模块实现
 */

#include "board_config.h"
#include "gpio_manager.h"
#include "pin_definitions.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "GPIO_MANAGER";

// GPIO分配表（ESP32有40个GPIO，编号0-39）
#define MAX_GPIO_NUM 40
static gpio_allocation_t gpio_allocation_table[MAX_GPIO_NUM];

// 初始化标志
static bool is_initialized = false;

/**
 * @brief 获取功能类型的字符串描述
 */
static const char* get_function_name(gpio_function_t function) {
    switch (function) {
        case GPIO_FUNC_UNUSED:      return "未使用";
        case GPIO_FUNC_ADC:         return "ADC输入";
        case GPIO_FUNC_PWM:         return "PWM输出";
        case GPIO_FUNC_UART_TX:     return "UART发送";
        case GPIO_FUNC_UART_RX:     return "UART接收";
        case GPIO_FUNC_I2C_SDA:     return "I2C数据";
        case GPIO_FUNC_I2C_SCL:     return "I2C时钟";
        case GPIO_FUNC_GPIO_OUT:    return "GPIO输出";
        case GPIO_FUNC_GPIO_IN:     return "GPIO输入";
        case GPIO_FUNC_TOUCH:       return "触摸传感器";
        case GPIO_FUNC_DHT11:       return "DHT11";
        case GPIO_FUNC_TM1637_CLK:  return "TM1637时钟";
        case GPIO_FUNC_TM1637_DIO:  return "TM1637数据";
        default:                    return "未知";
    }
}

esp_err_t gpio_manager_init(void) {
    if (is_initialized) {
        ESP_LOGW(TAG, "GPIO管理器已初始化");
        return ESP_OK;
    }

    // 初始化分配表
    for (int i = 0; i < MAX_GPIO_NUM; i++) {
        gpio_allocation_table[i].gpio_num = i;
        gpio_allocation_table[i].function = GPIO_FUNC_UNUSED;
        gpio_allocation_table[i].module_name = NULL;
        gpio_allocation_table[i].description = NULL;
    }

    is_initialized = true;
    ESP_LOGI(TAG, "GPIO管理器初始化成功");
    return ESP_OK;
}

esp_err_t gpio_manager_register(gpio_num_t gpio_num, 
                                gpio_function_t function,
                                const char *module_name,
                                const char *description) {
    // 检查初始化状态
    if (!is_initialized) {
        ESP_LOGE(TAG, "GPIO管理器未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    // 检查GPIO编号有效性
    if (gpio_num < 0 || gpio_num >= MAX_GPIO_NUM) {
        ESP_LOGE(TAG, "无效的GPIO编号: %d", gpio_num);
        return ESP_ERR_INVALID_ARG;
    }

    // 检查是否已被占用
    if (gpio_allocation_table[gpio_num].function != GPIO_FUNC_UNUSED) {
        ESP_LOGE(TAG, "GPIO冲突检测到！");
        ESP_LOGE(TAG, "  GPIO%d 已被占用:", gpio_num);
        ESP_LOGE(TAG, "    现有模块: %s", gpio_allocation_table[gpio_num].module_name);
        ESP_LOGE(TAG, "    现有功能: %s", get_function_name(gpio_allocation_table[gpio_num].function));
        ESP_LOGE(TAG, "    现有描述: %s", gpio_allocation_table[gpio_num].description);
        ESP_LOGE(TAG, "  尝试分配:");
        ESP_LOGE(TAG, "    新模块: %s", module_name);
        ESP_LOGE(TAG, "    新功能: %s", get_function_name(function));
        ESP_LOGE(TAG, "    新描述: %s", description);
        return ESP_ERR_INVALID_STATE;
    }

    // 注册GPIO
    gpio_allocation_table[gpio_num].function = function;
    gpio_allocation_table[gpio_num].module_name = module_name;
    gpio_allocation_table[gpio_num].description = description;

    ESP_LOGI(TAG, "注册GPIO%d: %s - %s (%s)", 
             gpio_num, module_name, description, get_function_name(function));

    return ESP_OK;
}

bool gpio_manager_is_allocated(gpio_num_t gpio_num, gpio_allocation_t *allocation) {
    // 检查GPIO编号有效性
    if (gpio_num < 0 || gpio_num >= MAX_GPIO_NUM) {
        return false;
    }

    bool allocated = (gpio_allocation_table[gpio_num].function != GPIO_FUNC_UNUSED);

    // 如果需要返回分配信息
    if (allocated && allocation != NULL) {
        *allocation = gpio_allocation_table[gpio_num];
    }

    return allocated;
}

void gpio_manager_print_allocation_table(void) {
    ESP_LOGI(TAG, "========== GPIO分配表 ==========");
    ESP_LOGI(TAG, "GPIO | 功能类型      | 模块名称           | 描述");
    ESP_LOGI(TAG, "-----+---------------+--------------------+------------------------");

    int allocated_count = 0;
    for (int i = 0; i < MAX_GPIO_NUM; i++) {
        if (gpio_allocation_table[i].function != GPIO_FUNC_UNUSED) {
            ESP_LOGI(TAG, "%-4d | %-13s | %-18s | %s",
                     i,
                     get_function_name(gpio_allocation_table[i].function),
                     gpio_allocation_table[i].module_name,
                     gpio_allocation_table[i].description);
            allocated_count++;
        }
    }

    ESP_LOGI(TAG, "================================");
    ESP_LOGI(TAG, "总计: %d个GPIO已分配", allocated_count);
}

esp_err_t gpio_manager_verify(void) {
    ESP_LOGI(TAG, "开始验证GPIO分配表...");

    bool has_conflict = false;
    int allocated_count = 0;

    // 检查每个GPIO
    for (int i = 0; i < MAX_GPIO_NUM; i++) {
        if (gpio_allocation_table[i].function != GPIO_FUNC_UNUSED) {
            allocated_count++;

            // 验证模块名称和描述不为空
            if (gpio_allocation_table[i].module_name == NULL) {
                ESP_LOGE(TAG, "GPIO%d: 模块名称为空", i);
                has_conflict = true;
            }

            if (gpio_allocation_table[i].description == NULL) {
                ESP_LOGE(TAG, "GPIO%d: 描述信息为空", i);
                has_conflict = true;
            }
        }
    }

    if (has_conflict) {
        ESP_LOGE(TAG, "GPIO分配表验证失败！");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "GPIO分配表验证通过（%d个GPIO已分配）", allocated_count);
    return ESP_OK;
}
