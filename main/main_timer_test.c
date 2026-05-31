/**
 * @file main_timer_test.c
 * @brief 定时器系统测试程序
 * 
 * 功能：
 * - 测试Timer 0和Timer 1的初始化
 * - 测试中断计数器功能
 * - 验证定时器周期精度
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "timer_system.h"

static const char *TAG = "timer_test";

void app_main(void)
{
    ESP_LOGI(TAG, "定时器系统测试程序启动");
    
    // 初始化定时器系统
    esp_err_t ret = timer_system_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "定时器系统初始化失败");
        return;
    }
    
    // 启动定时器系统
    ret = timer_system_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "定时器系统启动失败");
        return;
    }
    
    ESP_LOGI(TAG, "定时器系统已启动，开始监控中断计数...");
    
    // 监控中断计数器
    uint32_t last_timer0_count = 0;
    uint32_t last_timer1_count = 0;
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000)); // 每秒打印一次
        
        uint32_t timer0_count = timer_system_get_timer0_count();
        uint32_t timer1_count = timer_system_get_timer1_count();
        
        uint32_t timer0_delta = timer0_count - last_timer0_count;
        uint32_t timer1_delta = timer1_count - last_timer1_count;
        
        ESP_LOGI(TAG, "Timer 0: %lu 次中断 (+%lu/s, 预期1000/s)", 
                 timer0_count, timer0_delta);
        ESP_LOGI(TAG, "Timer 1: %lu 次中断 (+%lu/s, 预期100/s)", 
                 timer1_count, timer1_delta);
        
        // 验证周期精度
        if (timer0_delta < 950 || timer0_delta > 1050) {
            ESP_LOGW(TAG, "Timer 0周期偏差较大！");
        }
        if (timer1_delta < 95 || timer1_delta > 105) {
            ESP_LOGW(TAG, "Timer 1周期偏差较大！");
        }
        
        last_timer0_count = timer0_count;
        last_timer1_count = timer1_count;
    }
}
