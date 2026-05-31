/**
 * @file main_servo_test.c
 * @brief 舵机自动往复控制任务测试程序
 * 
 * 测试舵机任务的基本功能：
 * 1. 任务创建和初始化
 * 2. 3秒往复运动
 * 3. 错误重试机制
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "servo_task.h"

static const char *TAG = "servo_test";

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "舵机自动往复控制任务测试程序");
    ESP_LOGI(TAG, "========================================");
    
    // 注意：使用UART0前需要关闭ESP_LOG输出到UART0
    // 如果需要看到日志，可以配置ESP_LOG输出到USB_SERIAL_JTAG
    ESP_LOGI(TAG, "提示：如果使用UART0作为舵机通信，建议将日志输出配置到USB_SERIAL_JTAG");
    
    // 等待1秒让系统稳定
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(TAG, "创建舵机任务...");
    
    // 创建舵机任务
    TaskHandle_t servo_handle = servo_task_create();
    
    if (servo_handle == NULL) {
        ESP_LOGE(TAG, "舵机任务创建失败！");
        return;
    }
    
    ESP_LOGI(TAG, "舵机任务创建成功");
    ESP_LOGI(TAG, "舵机将每3秒在两组位置之间切换");
    ESP_LOGI(TAG, "位置组1: (800, 650, 850, 750)");
    ESP_LOGI(TAG, "位置组2: (350, 750, 750, 680)");
    
    // 主任务进入监控循环
    uint32_t elapsed_seconds = 0;
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));  // 每10秒打印一次状态
        elapsed_seconds += 10;
        
        ESP_LOGI(TAG, "舵机任务运行中... (已运行 %lu 秒)", elapsed_seconds);
        
        // 可选：在这里添加任务健康检查
        // 例如：检查任务是否还在运行
        eTaskState task_state = eTaskGetState(servo_handle);
        if (task_state == eDeleted || task_state == eInvalid) {
            ESP_LOGE(TAG, "舵机任务异常终止！");
            break;
        }
    }
    
    // 如果需要停止任务（正常情况下不会执行到这里）
    ESP_LOGI(TAG, "停止舵机任务...");
    servo_task_delete(servo_handle);
    ESP_LOGI(TAG, "测试结束");
}
