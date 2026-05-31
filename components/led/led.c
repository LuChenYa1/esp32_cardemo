#include "led.h"                    // 包含LED驱动的头文件，声明了LED相关函数
#include "gpio_manager.h"           // GPIO管理器，用于注册和检测GPIO冲突
#include "esp_log.h"                // ESP-IDF日志库，用于输出调试信息

static const char *TAG = "LED";     // 定义日志标签，用于标识日志来源

/**
 * @brief 初始化LED GPIO引脚
 * 
 * 配置指定的GPIO引脚作为输出引脚，用于控制LED
 * 设置GPIO参数包括：引脚掩码、工作模式、上下拉电阻和中断类型
 */
void led_init(void) {
    // 1. 注册GPIO到gpio_manager（检测冲突）
    esp_err_t ret = gpio_manager_register(LED_GPIO, GPIO_FUNC_GPIO_OUT, 
                                          "led", "LED指示灯");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO%d注册失败，可能与其他模块冲突", LED_GPIO);
        return;
    }
    
    // 2. 配置GPIO0为输出模式
    gpio_config_t io_conf = {                           // 定义GPIO配置结构体
        .pin_bit_mask = (1ULL << LED_GPIO),             // 指定要配置的GPIO引脚，通过位掩码表示
        .mode = GPIO_MODE_OUTPUT,                       // 设置GPIO为输出模式
        .pull_up_en = GPIO_PULLUP_DISABLE,              // 禁用内部上拉电阻
        .pull_down_en = GPIO_PULLDOWN_DISABLE,          // 禁用内部下拉电阻
        .intr_type = GPIO_INTR_DISABLE                  // 禁用GPIO中断功能
    };
    gpio_config(&io_conf);                              // 应用GPIO配置
    
    ESP_LOGI(TAG, "LED初始化成功 (GPIO%d)", LED_GPIO);  // 输出初始化成功日志
}

/**
 * @brief 打开LED
 * 
 * 将GPIO引脚设置为低电平，使LED点亮
 * 同时输出操作成功的日志信息
 */
void led_on(void) {
    gpio_set_level(LED_GPIO, 0);                        // 设置GPIO引脚为低电平(0)，LED点亮
    ESP_LOGI(TAG, "LED 打开");                     // 输出LED打开的日志信息
}

/**
 * @brief 关闭LED
 * 
 * 将GPIO引脚设置为高电平，使LED熄灭
 * 同时输出操作成功的日志信息
 */
void led_off(void) {
    gpio_set_level(LED_GPIO, 1);                        // 设置GPIO引脚为高电平(1)，LED熄灭
    ESP_LOGI(TAG, "LED 关闭");                    // 输出LED关闭的日志信息
}

/**
 * @brief 翻转LED状态
 * 
 * 读取当前GPIO引脚电平，然后将其翻转
 * 如果LED当前是灭的，则点亮；如果LED当前是亮的，则熄灭
 * 同时输出操作成功的日志信息，显示新的LED状态
 */
void led_toggle(void) {
    int current_level = gpio_get_level(LED_GPIO);               // 获取当前GPIO引脚的电平状态(0或1)
    int new_level = !current_level;                   // 计算新的电平
    gpio_set_level(LED_GPIO, new_level);                   // 将GPIO引脚设置为相反的状态
    ESP_LOGI(TAG, "LED 电平翻转至 %s", new_level == 0 ? "ON" : "OFF"); // 输出LED翻转后的状态日志
}