/**
 * @file gray_calibration_test.c
 * @brief 灰度传感器校准测试程序
 *
 * 功能：
 * - 实时显示左右灰度传感器的原始ADC值
 * - 引导用户进行交互式校准
 * - 显示校准后的归一化值
 * - 测试黑线检测功能
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "gray_sensor.h"
#include "board_config.h"

static const char *TAG = "GRAY_CALIBRATION";

/**
 * @brief 显示原始ADC值（持续监控模式）
 * 
 * @param duration_sec 监控持续时间（秒）
 */
static void display_raw_values(uint32_t duration_sec)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "开始显示原始ADC值（持续%lu秒）", duration_sec);
    ESP_LOGI(TAG, "========================================");
    
    uint32_t count = duration_sec * 10; // 每100ms读取一次
    
    for (uint32_t i = 0; i < count; i++) {
        uint16_t left_raw, right_raw;
        
        if (gray_sensor_read_both_raw_direct(&left_raw, &right_raw)) {
            ESP_LOGI(TAG, "[%3lu.%1lu秒] 左传感器: %4d | 右传感器: %4d", 
                     i / 10, i % 10, left_raw, right_raw);
        } else {
            ESP_LOGW(TAG, "读取ADC失败");
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    ESP_LOGI(TAG, "========================================");
}

/**
 * @brief 采集校准数据
 * 
 * @param white_left 白色区域左传感器值输出
 * @param white_right 白色区域右传感器值输出
 * @param black_left 黑线区域左传感器值输出
 * @param black_right 黑线区域右传感器值输出
 */
static void collect_calibration_data(uint16_t *white_left, uint16_t *white_right,
                                     uint16_t *black_left, uint16_t *black_right)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "开始校准数据采集");
    ESP_LOGI(TAG, "========================================");
    
    // ========== 采集白色区域数据 ==========
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "步骤1: 请将传感器放置在白色区域上");
    ESP_LOGI(TAG, "5秒后开始采集白色数据...");
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    *white_left = 0;
    *white_right = 0;
    
    ESP_LOGI(TAG, "正在采集白色区域数据（100次采样）...");
    
    for (int i = 0; i < 100; i++) {
        uint16_t left, right;
        
        if (gray_sensor_read_both_raw_direct(&left, &right)) {
            // 取最大值作为白色基准
            if (left > *white_left) *white_left = left;
            if (right > *white_right) *white_right = right;
            
            // 每10次打印一次进度
            if (i % 10 == 0) {
                ESP_LOGI(TAG, "采样进度: %d/100 - 左: %d, 右: %d", 
                         i, left, right);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    ESP_LOGI(TAG, "白色区域采集完成: 左=%d, 右=%d", *white_left, *white_right);
    
    // ========== 采集黑线区域数据 ==========
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "步骤2: 请将传感器放置在黑线上");
    ESP_LOGI(TAG, "5秒后开始采集黑线数据...");
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    *black_left = 4095;
    *black_right = 4095;
    
    ESP_LOGI(TAG, "正在采集黑线数据（100次采样）...");
    
    for (int i = 0; i < 100; i++) {
        uint16_t left, right;
        
        if (gray_sensor_read_both_raw_direct(&left, &right)) {
            // 取最小值作为黑线基准
            if (left < *black_left) *black_left = left;
            if (right < *black_right) *black_right = right;
            
            // 每10次打印一次进度
            if (i % 10 == 0) {
                ESP_LOGI(TAG, "采样进度: %d/100 - 左: %d, 右: %d", 
                         i, left, right);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    ESP_LOGI(TAG, "黑线区域采集完成: 左=%d, 右=%d", *black_left, *black_right);
    ESP_LOGI(TAG, "========================================");
}

/**
 * @brief 测试校准效果
 * 
 * @param duration_sec 测试持续时间（秒）
 */
static void test_calibration(uint32_t duration_sec)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "开始测试校准效果（持续%lu秒）", duration_sec);
    ESP_LOGI(TAG, "========================================");
    
    uint32_t count = duration_sec * 10; // 每100ms读取一次
    
    for (uint32_t i = 0; i < count; i++) {
        uint16_t left_raw, right_raw;
        float left_norm, right_norm;
        
        // 读取原始值
        if (gray_sensor_read_both_raw_direct(&left_raw, &right_raw)) {
            // 读取归一化值
            gray_sensor_read_both_normalized(&left_norm, &right_norm);
            
            // 判断是否在黑线上
            bool left_on_black = gray_sensor_is_on_black_line(GRAY_SENSOR_LEFT, 0.5f);
            bool right_on_black = gray_sensor_is_on_black_line(GRAY_SENSOR_RIGHT, 0.5f);
            
            ESP_LOGI(TAG, "[%3lu.%1lu秒] 左: %4d (%.3f) %s | 右: %4d (%.3f) %s", 
                     i / 10, i % 10,
                     left_raw, left_norm, left_on_black ? "[黑线]" : "[白色]",
                     right_raw, right_norm, right_on_black ? "[黑线]" : "[白色]");
        } else {
            ESP_LOGW(TAG, "读取ADC失败");
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    ESP_LOGI(TAG, "========================================");
}

/**
 * @brief 主函数
 */
void app_main(void)
{
    esp_err_t ret;
    
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "灰度传感器校准测试程序");
    ESP_LOGI(TAG, "========================================");
    
    // 初始化GPIO管理器
    ESP_LOGI(TAG, "初始化GPIO管理器...");
    ret = board_config_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO管理器初始化失败");
        return;
    }
    
    // 初始化灰度传感器（简化模式）
    ESP_LOGI(TAG, "初始化灰度传感器...");
    gray_sensor_init_simple();
    
    // 等待硬件稳定
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // ========== 阶段1: 显示原始ADC值 ==========
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "阶段1: 显示原始ADC值");
    ESP_LOGI(TAG, "请观察传感器在白色和黑线上的ADC值变化");
    display_raw_values(10);
    
    // ========== 阶段2: 交互式校准 ==========
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "阶段2: 交互式校准");
    
    uint16_t white_left, white_right, black_left, black_right;
    collect_calibration_data(&white_left, &white_right, &black_left, &black_right);
    
    // 设置校准参数
    gray_sensor_set_calibration(GRAY_SENSOR_LEFT, white_left, black_left);
    gray_sensor_set_calibration(GRAY_SENSOR_RIGHT, white_right, black_right);
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "校准参数已设置：");
    ESP_LOGI(TAG, "左传感器: 白色=%d, 黑线=%d, 差值=%d", 
             white_left, black_left, white_left - black_left);
    ESP_LOGI(TAG, "右传感器: 白色=%d, 黑线=%d, 差值=%d", 
             white_right, black_right, white_right - black_right);
    
    // ========== 阶段3: 测试校准效果 ==========
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "阶段3: 测试校准效果");
    ESP_LOGI(TAG, "请将传感器在白色和黑线之间移动，观察归一化值和检测结果");
    test_calibration(20);
    
    // ========== 完成 ==========
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "校准测试完成！");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "建议的校准参数（可复制到main.c中）：");
    ESP_LOGI(TAG, "#define LEFT_WHITE_VALUE %d", white_left);
    ESP_LOGI(TAG, "#define LEFT_BLACK_VALUE %d", black_left);
    ESP_LOGI(TAG, "#define RIGHT_WHITE_VALUE %d", white_right);
    ESP_LOGI(TAG, "#define RIGHT_BLACK_VALUE %d", black_right);
    ESP_LOGI(TAG, "");
    
    // 持续显示实时数据
    ESP_LOGI(TAG, "进入持续监控模式（按Ctrl+C退出）...");
    
    while (1) {
        uint16_t left_raw, right_raw;
        float left_norm, right_norm;
        
        if (gray_sensor_read_both_raw_direct(&left_raw, &right_raw)) {
            gray_sensor_read_both_normalized(&left_norm, &right_norm);
            
            bool left_on_black = gray_sensor_is_on_black_line(GRAY_SENSOR_LEFT, 0.5f);
            bool right_on_black = gray_sensor_is_on_black_line(GRAY_SENSOR_RIGHT, 0.5f);
            
            ESP_LOGI(TAG, "左: %4d (%.3f) %s | 右: %4d (%.3f) %s", 
                     left_raw, left_norm, left_on_black ? "[黑线]" : "[白色]",
                     right_raw, right_norm, right_on_black ? "[黑线]" : "[白色]");
        }
        
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
