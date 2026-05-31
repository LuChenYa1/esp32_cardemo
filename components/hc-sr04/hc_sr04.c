/**
 * @file hc_sr04.c
 * @brief HC-SR04超声波测距传感器驱动实现
 * 
 * 功能：
 * - 超声波测距（2cm-400cm）
 * - 基于回声时间计算距离
 * - 使用GPIO触发和接收
 * 
 * 接口：
 * - TRIG: GPIO48 [SSD4]
 * - ECHO: GPIO47 [SSA4]
 * 
 * 注意：
 * - 测量精度受温度影响
 * - 超时时间设置为100ms
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "esp_log.h"
#include "hc_sr04.h"
#include "pin_definitions.h"  // 引入统一的引脚定义

// 日志标签，用于ESP-IDF日志输出
static const char *TAG = "HCSR04";

// 超时时间定义：100ms，避免程序无限等待
#define TIMEOUT_US  (100 * 1000)   /* 100 ms */

// 使用pin_definitions.h中的引脚定义
#define TRIG_PIN  HCSR04_TRIG_GPIO  // 触发引脚 GPIO48 [SSD4]
#define ECHO_PIN  HCSR04_ECHO_GPIO  // 回响引脚 GPIO47 [SSA4]

/**
 * @brief 测量距离函数
 * @return 返回距离值(单位：cm)，测量失败返回-1
 */
float measure_distance_cm(void)
{
    uint64_t t_start = 0, t_end = 0;

    // 1. 发送触发信号：给TRIG引脚一个至少10微秒的高电平脉冲
    gpio_set_level(TRIG_PIN, 1);

    /* 2. 等待ECHO引脚变高电平（超时保护）
     * HC-SR04在接收到超声波信号后，ECHO引脚会变为高电平
     * 这里等待ECHO引脚从低电平变为高电平
     */
    uint64_t t0 = esp_timer_get_time();  // 记录开始时间
    while (gpio_get_level(ECHO_PIN) == 0) {
        // 检查是否超时（超过100ms）
        if (esp_timer_get_time() - t0 > 100000) {
            ESP_LOGW(TAG, "echo 等待高电平超时");  // 记录超时警告
            return -1;  // 超时返回-1
        }
    }
    t_start = esp_timer_get_time();  // 记录ECHO变高的时间点

    /* 3. 等待ECHO引脚变低电平（超时保护）
     * ECHO引脚保持高电平的时间与障碍物距离成正比
     * 这里测量ECHO引脚高电平持续的时间
     */
    while (gpio_get_level(ECHO_PIN) == 1) {
        // 检查是否超时
        if (esp_timer_get_time() - t_start > TIMEOUT_US) {
            ESP_LOGW(TAG, "超时");  // 记录超时警告
            return -1;  // 超时返回-1
        }
    }
    t_end = esp_timer_get_time();  // 记录ECHO变低的时间点

    /* 4. 计算距离
     * 距离计算公式：距离 = 时间 × 声速 / 2
     * 声速约为340m/s = 0.034cm/μs
     * 除以2是因为超声波往返两次距离
     */
    uint32_t dt_us = t_end - t_start;  // 计算高电平持续时间（微秒）
    float dist = dt_us * 0.034f / 2.0f;   /* 0.034 cm/µs，计算实际距离 */
    
    // 限制测量范围在400cm以内，超出范围则返回-1
    return dist > 400 ? -1 : dist;        
}

/**
 * @brief HC-SR04初始化及测量任务
 * @return 返回测量的距离值
 */
float hc_sr04_task(void)
{
    /* 配置TRIG引脚为输出模式
     * TRIG引脚用于发送触发信号启动超声波测距
     */
    gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(TRIG_PIN, 1);  // 初始化为高电平

    /* 配置ECHO引脚为输入模式，并启用上拉电阻
     * ECHO引脚接收超声波返回信号
     * 启用上拉电阻确保空闲时引脚保持稳定的高电平状态
     */
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(ECHO_PIN, GPIO_PULLUP_ONLY);

    // 执行距离测量并返回结果
    return measure_distance_cm();
}