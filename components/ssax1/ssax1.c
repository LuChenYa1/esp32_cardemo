/**
 * @file ssax1.c
 * @brief SSA扩展接口数字信号采集模块实现
 * 
 * 功能：
 * - 通过GPIO读取SSA接口的数字信号
 * - 支持触摸传感器等数字输入设备
 * 
 * 接口：
 * - SSA2: GPIO37 - 触摸传感器（数字信号）
 * 
 * 注意：
 * - GPIO37只能作为输入使用
 */

#include "ssax1.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "SSAX1";

// 定义需要监控的GPIO引脚列表
// 只配置未被其他传感器占用的数字输入引脚
// SSA2: GPIO37 - 触摸传感器（数字信号）
static const gpio_num_t gpio_pins[] = {
    GPIO_NUM_37   // SSA2 - 触摸传感器（数字信号采集）
};
// 计算引脚数组的大小
static const int num_pins = sizeof(gpio_pins) / sizeof(gpio_pins[0]);

/**
 * @brief 初始化GPIO引脚配置
 * 
 * 该函数将指定的GPIO引脚配置为输入模式，用于读取外部信号状态
 */
void ssax1_gpio_init(void) {
    // 定义GPIO配置结构体
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,  // 禁用GPIO中断功能
        .mode = GPIO_MODE_INPUT,         // 设置为输入模式
        .pin_bit_mask = 0,               // 引脚位掩码，将在后续循环中动态设置
        .pull_down_en = GPIO_PULLDOWN_DISABLE,  // 禁用下拉电阻
        .pull_up_en = GPIO_PULLUP_DISABLE,      // 禁用上拉电阻
    };

    // 循环遍历所有GPIO引脚，构建位掩码
    for (int i = 0; i < num_pins; i++) {
        // 使用位运算将每个引脚号转换为对应位置的位掩码
        io_conf.pin_bit_mask |= (1ULL << gpio_pins[i]);
    }

    // 应用GPIO配置到硬件
    gpio_config(&io_conf);

    // 输出初始化成功的日志信息
    ESP_LOGI(TAG, "数字信号采集GPIO初始化完成: SSA2(GPIO37) - 触摸传感器");
}

/**
 * @brief 持续轮询GPIO引脚状态
 * 
 * 该函数会无限循环地读取所有配置的GPIO引脚的状态，并输出到日志
 * 这个函数通常在一个独立的任务中运行
 */
void ssax1_poll_gpio_status(void) {
    // 无限循环，持续监测GPIO状态
    while (1) {
        // 遍历所有GPIO引脚并读取其电平状态
        for (int i = 0; i < num_pins; i++) {
            // 获取当前引脚的电平状态（高电平=1，低电平=0）
            int level = gpio_get_level(gpio_pins[i]);
            // 打印引脚编号和当前电平状态到日志
            ESP_LOGI(TAG, "GPIO %d: %d", gpio_pins[i], level);
        }
        // 延迟一段时间，避免日志输出过于频繁
        vTaskDelay(pdMS_TO_TICKS(1000));  // 延迟1000毫秒（1秒）
    }
}

//
void ssax1_read(void)
{
        // 遍历所有GPIO引脚并读取其电平状态
        for (int i = 0; i < num_pins; i++) {
            // 获取当前引脚的电平状态（高电平=1，低电平=0）
            int level = gpio_get_level(gpio_pins[i]);
            // 打印引脚编号和当前电平状态到日志
            ESP_LOGI(TAG, "GPIO %d: %d", gpio_pins[i], level);
        }
}