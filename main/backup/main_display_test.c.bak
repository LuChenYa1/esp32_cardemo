// /**
//  * @file main_display_test.c
//  * @brief 数码管轮播显示任务测试程序
//  * 
//  * 测试内容：
//  * - 验证三种显示模式的数据读取
//  * - 验证5秒自动切换显示模式
//  * - 验证DHT11读取失败时保持上次有效值
//  */

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_log.h"
// #include "nvs_flash.h"

// // 数码管显示任务
// #include "display_task.h"

// // I2C初始化（VL53L0X和TM1637需要）
// #include "i2c.h"

// static const char *TAG = "DISPLAY_TEST";

// /**
//  * @brief 应用程序入口
//  */
// void app_main(void)
// {
//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "  数码管轮播显示任务测试程序");
//     ESP_LOGI(TAG, "========================================");
    
//     // 初始化NVS（某些驱动可能需要）
//     esp_err_t ret = nvs_flash_init();
//     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//         ESP_ERROR_CHECK(nvs_flash_erase());
//         ret = nvs_flash_init();
//     }
//     ESP_ERROR_CHECK(ret);
    
//     // 初始化I2C GPIO（VL53L0X和TM1637使用）
//     Distance_IO_Init();
//     ESP_LOGI(TAG, "I2C GPIO初始化完成");
    
//     // 创建数码管显示任务
//     ret = display_task_create();
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "数码管显示任务创建失败");
//         return;
//     }
    
//     ESP_LOGI(TAG, "数码管显示任务已启动");
//     ESP_LOGI(TAG, "");
//     ESP_LOGI(TAG, "测试说明：");
//     ESP_LOGI(TAG, "1. 观察数码管每5秒自动切换显示模式");
//     ESP_LOGI(TAG, "2. 模式顺序：光照 -> 温湿度 -> 距离 -> 光照（循环）");
//     ESP_LOGI(TAG, "3. 光照模式：显示ADC值（0-4095）");
//     ESP_LOGI(TAG, "4. 温湿度模式：显示TTHH格式（温度+湿度）");
//     ESP_LOGI(TAG, "5. 距离模式：显示距离值（0-9999 cm）");
//     ESP_LOGI(TAG, "");
//     ESP_LOGI(TAG, "测试验证：");
//     ESP_LOGI(TAG, "- 遮挡光照传感器，观察光照模式数值变化");
//     ESP_LOGI(TAG, "- 在VL53L0X前放置物体，观察距离模式数值变化");
//     ESP_LOGI(TAG, "- 断开DHT11，观察温湿度模式是否保持上次值");
//     ESP_LOGI(TAG, "========================================");
    
//     // 主循环：监控任务状态
//     uint32_t loop_count = 0;
//     while (1) {
//         vTaskDelay(pdMS_TO_TICKS(10000));  // 每10秒打印一次状态
//         loop_count++;
        
//         ESP_LOGI(TAG, "运行时间：%lu秒，任务正常运行中...", loop_count * 10);
        
//         // 打印任务栈使用情况（用于检测栈溢出）
//         UBaseType_t stack_high_water_mark = uxTaskGetStackHighWaterMark(NULL);
//         ESP_LOGI(TAG, "主任务剩余栈空间：%u字节", stack_high_water_mark * sizeof(StackType_t));
//     }
// }
