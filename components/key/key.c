#include "key.h"
#include "led.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// 静态变量，用于跟踪按键的状态，防止重复触发
static bool key1_pressed = false;  // 记录按键1是否处于按下状态
static bool key2_pressed = false;  // 记录按键2是否处于按下状态

/**
 * @brief 初始化按键GPIO引脚
 * 
 * 配置KEY1和KEY2为输入模式，并启用内部上拉电阻
 */
void key_init(void) {
    // 配置GPIO37作为KEY1
    gpio_config_t io_conf1 = {
        .pin_bit_mask = (1ULL << KEY1_GPIO),  // 设置GPIO37位掩码
        .mode = GPIO_MODE_INPUT,              // 设置为输入模式
        .pull_up_en = GPIO_PULLUP_ENABLE,     // 启用内部上拉电阻
        .pull_down_en = GPIO_PULLDOWN_DISABLE,// 禁用内部下拉电阻
        .intr_type = GPIO_INTR_DISABLE        // 禁用GPIO中断
    };
    gpio_config(&io_conf1);  // 应用GPIO配置

    // 配置GPIO38作为KEY2
    gpio_config_t io_conf2 = {
        .pin_bit_mask = (1ULL << KEY2_GPIO),  // 设置GPIO38位掩码
        .mode = GPIO_MODE_INPUT,              // 设置为输入模式
        .pull_up_en = GPIO_PULLUP_ENABLE,     // 启用内部上拉电阻
        .pull_down_en = GPIO_PULLDOWN_DISABLE,// 禁用内部下拉电阻
        .intr_type = GPIO_INTR_DISABLE        // 禁用GPIO中断
    };
    gpio_config(&io_conf2);  // 应用GPIO配置
}

/**
 * @brief 按键检测和处理任务
 * 
 * 检测按键状态，当检测到按键按下时执行相应的LED控制操作
 * 实现了按键消抖功能，防止重复触发
 */
void key_task(void) {
    // 共享的LED状态：true表示开启，false表示关闭
    static bool led_state = false;  // 初始状态：关闭

    // 检测KEY1 (GPIO37) 按键，用于LED开关控制
    if (gpio_get_level(KEY1_GPIO) == 0) {  // 假设低电平有效（按下时为0）
        if (!key1_pressed) {  // 如果按键之前没有被按下（防抖处理）
            key1_pressed = true;  // 标记按键已被按下
            if (led_state) {      // 如果LED当前是开启状态
                led_off();        // 关闭LED
                led_state = false; // 更新LED状态为关闭
            } else {              // 如果LED当前是关闭状态
                led_on();         // 开启LED
                led_state = true; // 更新LED状态为开启
            }
        }
    } else {
        key1_pressed = false;  // 按键释放，重置按下状态
    }

    // 检测KEY2 (GPIO38) 按键，用于LED翻转控制
    if (gpio_get_level(KEY2_GPIO) == 0) {  // 假设低电平有效（按下时为0）
        if (!key2_pressed) {  // 如果按键之前没有被按下（防抖处理）
            key2_pressed = true;      // 标记按键已被按下
            led_toggle();             // 翻转LED状态（开变关，关变开）
            led_state = !led_state;   // 更新共享状态
        }
    } else {
        key2_pressed = false;  // 按键释放，重置按下状态
    }

    // 添加延时用于按键消抖，延时50毫秒
    vTaskDelay(50 / portTICK_PERIOD_MS);
}