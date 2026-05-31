/**
 * @file main_voice_test.c
 * @brief 语音控制模块测试程序
 * 
 * 功能：
 * - 测试语音模块初始化
 * - 测试UART接收和命令解析
 * - 测试运行模式标志的读写
 * - 模拟语音命令的接收和处理
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "voice_module.h"
#include "driver/uart.h"

static const char *TAG = "voice_test";

/**
 * @brief 模拟发送语音命令（用于测试）
 * 
 * @param command 命令字节
 */
static void simulate_voice_command(uint8_t command)
{
    // 构造命令帧：[帧头] [命令] [校验和] [帧尾]
    uint8_t frame[4] = {
        0xAA,       // 帧头
        command,    // 命令字节
        command,    // 校验和（简单校验：等于命令字节）
        0x55        // 帧尾
    };
    
    // 通过UART1发送（模拟语音模块发送）
    int sent = uart_write_bytes(VOICE_UART_PORT, frame, sizeof(frame));
    
    if (sent == sizeof(frame)) {
        ESP_LOGI(TAG, "模拟发送命令: 0x%02X", command);
    } else {
        ESP_LOGE(TAG, "发送命令失败");
    }
}

/**
 * @brief 监控任务：定期检查运行模式
 */
static void monitor_task(void *pvParameters)
{
    uint8_t last_mode = 0xFF;  // 初始化为无效值
    
    while (1) {
        uint8_t current_mode = voice_module_get_run_mode();
        
        // 只在模式变化时打印
        if (current_mode != last_mode) {
            if (current_mode == 1) {
                ESP_LOGI(TAG, ">>> 运行模式: 启动 <<<");
            } else {
                ESP_LOGI(TAG, ">>> 运行模式: 停止 <<<");
            }
            last_mode = current_mode;
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief 主函数
 */
void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "语音控制模块测试程序");
    ESP_LOGI(TAG, "========================================");
    
    // 1. 初始化语音模块
    ESP_LOGI(TAG, "步骤1: 初始化语音模块");
    esp_err_t err = voice_module_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "语音模块初始化失败");
        return;
    }
    ESP_LOGI(TAG, "✓ 语音模块初始化成功");
    
    // 2. 创建语音模块任务
    ESP_LOGI(TAG, "步骤2: 创建语音模块任务");
    err = voice_module_task_create();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "创建语音模块任务失败");
        return;
    }
    ESP_LOGI(TAG, "✓ 语音模块任务创建成功");
    
    // 3. 创建监控任务
    ESP_LOGI(TAG, "步骤3: 创建监控任务");
    xTaskCreate(monitor_task, "monitor", 2048, NULL, 3, NULL);
    ESP_LOGI(TAG, "✓ 监控任务创建成功");
    
    // 等待任务启动
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 4. 测试手动设置运行模式
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "测试1: 手动设置运行模式");
    ESP_LOGI(TAG, "========================================");
    
    ESP_LOGI(TAG, "设置运行模式为: 启动(1)");
    voice_module_set_run_mode(1);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    ESP_LOGI(TAG, "设置运行模式为: 停止(0)");
    voice_module_set_run_mode(0);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 5. 测试模拟语音命令
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "测试2: 模拟语音命令");
    ESP_LOGI(TAG, "========================================");
    
    ESP_LOGI(TAG, "发送启动命令 (0x01)");
    simulate_voice_command(VOICE_CMD_START);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "发送停止命令 (0x02)");
    simulate_voice_command(VOICE_CMD_STOP);
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // 6. 循环测试
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "测试3: 循环测试（每5秒切换一次）");
    ESP_LOGI(TAG, "========================================");
    
    uint8_t toggle = 0;
    while (1) {
        if (toggle == 0) {
            ESP_LOGI(TAG, "发送启动命令");
            simulate_voice_command(VOICE_CMD_START);
            toggle = 1;
        } else {
            ESP_LOGI(TAG, "发送停止命令");
            simulate_voice_command(VOICE_CMD_STOP);
            toggle = 0;
        }
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
