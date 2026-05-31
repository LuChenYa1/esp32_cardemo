/**
 * @file main_display_test.c
 * @brief 数码管轮播显示任务测试程序
 *
 * 测试内容：
 * - 创建 display_task 显示任务
 * - 验证 TM1637 数码管每 5 秒自动切换显示模式
 * - 验证 DHT11 读取失败时继续显示上次有效温湿度
 *
 * 实际显示内容：
 * - DHT11 温湿度：TTHH 格式，前两位温度，后两位湿度
 * - HC-SR04 超声波测距：距离值，单位 cm
 * - 红外避障状态：0 表示无障碍，1 表示检测到障碍
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"

// 数码管显示任务
#include "display_task.h"

static const char *TAG = "DISPLAY_TEST";

/**
 * @brief 应用程序入口
 */
void app_main(void)
{
    // 初始化 NVS（部分驱动或系统组件可能需要）
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 创建数码管显示任务
    ret = display_task_create();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "数码管显示任务创建失败");
        return;
    }

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  数码管轮播显示任务测试程序");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "测试说明：");
    ESP_LOGI(TAG, "1. 观察 TM1637 数码管每 5 秒自动切换显示模式");
    ESP_LOGI(TAG, "2. 模式顺序：温湿度 -> 超声波测距 -> 红外避障 -> 温湿度（循环）");
    ESP_LOGI(TAG, "3. 温湿度模式：显示 TTHH 格式（温度 + 湿度），数据来自 DHT11");
    ESP_LOGI(TAG, "4. 测距模式：显示 HC-SR04 距离值（0-9999 cm）");
    ESP_LOGI(TAG, "5. 红外避障模式：显示 0/1（0=无障碍，1=检测到障碍）");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "测试验证：");
    ESP_LOGI(TAG, "- 改变 DHT11 周围温湿度，观察温湿度模式数值变化");
    ESP_LOGI(TAG, "- 在 HC-SR04 前方放置物体，观察测距模式数值变化");
    ESP_LOGI(TAG, "- 遮挡红外避障传感器，观察红外避障模式在 0 和 1 之间变化");
    ESP_LOGI(TAG, "- 断开或干扰 DHT11，观察温湿度模式是否保持上次有效值");
    ESP_LOGI(TAG, "========================================");

    // 主循环：监控任务状态
    uint32_t loop_count = 0;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));  // 每 10 秒打印一次状态
        loop_count++;

        ESP_LOGI(TAG, "运行时间：%lu 秒，display_task 正常运行中...", loop_count * 10);

        // 打印主任务栈使用情况（用于检测栈溢出）
        UBaseType_t stack_high_water_mark = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI(TAG, "主任务剩余栈空间：%u 字节", stack_high_water_mark * sizeof(StackType_t));
    }
}
