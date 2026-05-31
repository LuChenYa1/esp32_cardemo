#include "buzzer.h"
#include "gpio_manager.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "BUZZER";

/**
 * @brief 蜂鸣器初始化函数
 * 
 * 配置蜂鸣器GPIO引脚为输出模式，并设置初始状态为关闭
 */
void buzzer_init(void) {
    // 1. 注册GPIO到gpio_manager（检测冲突）
    esp_err_t ret = gpio_manager_register(BUZZER_GPIO, GPIO_FUNC_GPIO_OUT, 
                                          "buzzer", "蜂鸣器");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO%d注册失败，可能与其他模块冲突", BUZZER_GPIO);
        return;
    }
    
    // 2. 配置GPIO参数结构体
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUZZER_GPIO),      // 设置要配置的GPIO引脚
        .mode = GPIO_MODE_OUTPUT,                    // 设置引脚为输出模式
        .pull_up_en = GPIO_PULLUP_DISABLE,           // 禁用上拉电阻
        .pull_down_en = GPIO_PULLDOWN_DISABLE,       // 禁用下拉电阻
        .intr_type = GPIO_INTR_DISABLE               // 禁用中断功能
    };
    
    // 3. 应用GPIO配置
    gpio_config(&io_conf);
    
    // 4. 初始化时关闭蜂鸣器
    gpio_set_level(BUZZER_GPIO, 0);
    
    ESP_LOGI(TAG, "蜂鸣器初始化完成 (GPIO%d)", BUZZER_GPIO);
}

/**
 * @brief 打开蜂鸣器
 * 
 * 将蜂鸣器GPIO引脚设置为高电平，使蜂鸣器发声
 */
void buzzer_on(void) {
    gpio_set_level(BUZZER_GPIO, 1);
}

/**
 * @brief 关闭蜂鸣器
 * 
 * 将蜂鸣器GPIO引脚设置为低电平，停止蜂鸣器发声
 */
void buzzer_off(void) {
    gpio_set_level(BUZZER_GPIO, 0);
}

/**
 * @brief 蜂鸣器短响一次
 * 
 * 让蜂鸣器发声指定的时间长度后自动关闭
 * 
 * @param duration_ms 响声持续时间，单位毫秒
 */
void buzzer_beep(int duration_ms) {
    buzzer_on();                                    // 开启蜂鸣器
    vTaskDelay(pdMS_TO_TICKS(duration_ms));         // 延时指定时间
    buzzer_off();                                   // 关闭蜂鸣器
}