/**
 * @file main_tm1640_test.c
 * @brief TM1640 LED点阵屏测试程序
 * 
 * 测试内容：
 * - 基本初始化和显示功能
 * - 像素点控制
 * - 亮度调节
 * - GSTEM滚动动画
 * - 各种图案显示
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "board_config.h"
#include "tm1640.h"

static const char *TAG = "TM1640_TEST";

/**
 * @brief 测试1：基本初始化和清屏
 */
static void test_basic_init(void)
{
    ESP_LOGI(TAG, "【测试1】基本初始化和清屏");
    
    tm1640_init();
    ESP_LOGI(TAG, "✓ 初始化完成");
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    tm1640_clear();
    tm1640_refresh();
    ESP_LOGI(TAG, "✓ 清屏完成");
    
    vTaskDelay(pdMS_TO_TICKS(1000));
}

/**
 * @brief 测试2：单个像素点控制
 */
static void test_single_pixel(void)
{
    ESP_LOGI(TAG, "【测试2】单个像素点控制");
    
    tm1640_clear();
    
    // 点亮四个角的像素
    ESP_LOGI(TAG, "点亮四个角...");
    tm1640_set_led(0, 0, 1);    // 左上角
    tm1640_set_led(0, 15, 1);   // 右上角
    tm1640_set_led(7, 0, 1);    // 左下角
    tm1640_set_led(7, 15, 1);   // 右下角
    tm1640_refresh();
    
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 点亮中心点
    ESP_LOGI(TAG, "点亮中心点...");
    tm1640_clear();
    tm1640_set_led(3, 7, 1);
    tm1640_set_led(3, 8, 1);
    tm1640_set_led(4, 7, 1);
    tm1640_set_led(4, 8, 1);
    tm1640_refresh();
    
    vTaskDelay(pdMS_TO_TICKS(2000));
}

/**
 * @brief 测试3：绘制十字
 */
static void test_draw_cross(void)
{
    ESP_LOGI(TAG, "【测试3】绘制十字");
    
    tm1640_clear();
    
    // 水平线
    for (uint8_t c = 0; c < 16; c++) {
        tm1640_set_led(4, c, 1);
    }
    
    // 垂直线
    for (uint8_t r = 0; r < 8; r++) {
        tm1640_set_led(r, 8, 1);
    }
    
    tm1640_refresh();
    ESP_LOGI(TAG, "✓ 十字绘制完成");
    
    vTaskDelay(pdMS_TO_TICKS(2000));
}

/**
 * @brief 测试4：绘制边框
 */
static void test_draw_border(void)
{
    ESP_LOGI(TAG, "【测试4】绘制边框");
    
    tm1640_clear();
    
    // 上下边框
    for (uint8_t c = 0; c < 16; c++) {
        tm1640_set_led(0, c, 1);  // 上边
        tm1640_set_led(7, c, 1);  // 下边
    }
    
    // 左右边框
    for (uint8_t r = 0; r < 8; r++) {
        tm1640_set_led(r, 0, 1);   // 左边
        tm1640_set_led(r, 15, 1);  // 右边
    }
    
    tm1640_refresh();
    ESP_LOGI(TAG, "✓ 边框绘制完成");
    
    vTaskDelay(pdMS_TO_TICKS(2000));
}

/**
 * @brief 测试5：绘制笑脸
 */
static void test_draw_smile(void)
{
    ESP_LOGI(TAG, "【测试5】绘制笑脸");
    
    tm1640_clear();
    
    // 脸部轮廓
    for (uint8_t c = 3; c <= 12; c++) {
        tm1640_set_led(1, c, 1);  // 上边
        tm1640_set_led(6, c, 1);  // 下边
    }
    for (uint8_t r = 2; r <= 5; r++) {
        tm1640_set_led(r, 2, 1);   // 左边
        tm1640_set_led(r, 13, 1);  // 右边
    }
    
    // 左眼
    tm1640_set_led(3, 5, 1);
    tm1640_set_led(3, 6, 1);
    
    // 右眼
    tm1640_set_led(3, 9, 1);
    tm1640_set_led(3, 10, 1);
    
    // 嘴巴
    for (uint8_t c = 5; c <= 10; c++) {
        tm1640_set_led(5, c, 1);
    }
    tm1640_set_led(4, 6, 1);
    tm1640_set_led(4, 9, 1);
    
    tm1640_refresh();
    ESP_LOGI(TAG, "✓ 笑脸绘制完成");
    
    vTaskDelay(pdMS_TO_TICKS(3000));
}

/**
 * @brief 测试6：亮度调节
 */
static void test_brightness(void)
{
    ESP_LOGI(TAG, "【测试6】亮度调节");
    
    // 全屏点亮
    tm1640_clear();
    for (uint8_t r = 0; r < 8; r++) {
        for (uint8_t c = 0; c < 16; c++) {
            tm1640_set_led(r, c, 1);
        }
    }
    tm1640_refresh();
    
    // 从暗到亮
    ESP_LOGI(TAG, "从暗到亮渐变...");
    for (uint8_t level = 0; level <= 7; level++) {
        tm1640_set_brightness(level);
        ESP_LOGI(TAG, "亮度等级: %d/7", level);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 从亮到暗
    ESP_LOGI(TAG, "从亮到暗渐变...");
    for (int8_t level = 7; level >= 0; level--) {
        tm1640_set_brightness(level);
        ESP_LOGI(TAG, "亮度等级: %d/7", level);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    // 恢复最高亮度
    tm1640_set_brightness(7);
    vTaskDelay(pdMS_TO_TICKS(1000));
}

/**
 * @brief 测试7：棋盘图案
 */
static void test_checkerboard(void)
{
    ESP_LOGI(TAG, "【测试7】棋盘图案");
    
    tm1640_clear();
    
    // 绘制棋盘
    for (uint8_t r = 0; r < 8; r++) {
        for (uint8_t c = 0; c < 16; c++) {
            if ((r + c) % 2 == 0) {
                tm1640_set_led(r, c, 1);
            }
        }
    }
    
    tm1640_refresh();
    ESP_LOGI(TAG, "✓ 棋盘图案绘制完成");
    
    vTaskDelay(pdMS_TO_TICKS(2000));
}

/**
 * @brief 测试8：进度条动画
 */
static void test_progress_bar(void)
{
    ESP_LOGI(TAG, "【测试8】进度条动画");
    
    // 从0%到100%
    for (uint8_t percent = 0; percent <= 100; percent += 5) {
        tm1640_clear();
        
        uint8_t cols = (percent * 16) / 100;
        
        // 绘制进度条（使用中间3行）
        for (uint8_t c = 0; c < cols; c++) {
            tm1640_set_led(3, c, 1);
            tm1640_set_led(4, c, 1);
            tm1640_set_led(5, c, 1);
        }
        
        tm1640_refresh();
        ESP_LOGI(TAG, "进度: %d%%", percent);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    vTaskDelay(pdMS_TO_TICKS(1000));
}

/**
 * @brief 测试9：闪烁效果
 */
static void test_blink(void)
{
    ESP_LOGI(TAG, "【测试9】闪烁效果");
    
    // 绘制图案
    tm1640_clear();
    for (uint8_t c = 0; c < 16; c += 2) {
        for (uint8_t r = 0; r < 8; r++) {
            tm1640_set_led(r, c, 1);
        }
    }
    
    // 闪烁5次
    for (int i = 0; i < 5; i++) {
        ESP_LOGI(TAG, "闪烁 %d/5", i + 1);
        
        tm1640_refresh();
        vTaskDelay(pdMS_TO_TICKS(500));
        
        tm1640_clear();
        tm1640_refresh();
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // 恢复图案
        for (uint8_t c = 0; c < 16; c += 2) {
            for (uint8_t r = 0; r < 8; r++) {
                tm1640_set_led(r, c, 1);
            }
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(1000));
}

/**
 * @brief 测试10：GSTEM滚动动画
 */
static void test_gstem_animation(void)
{
    ESP_LOGI(TAG, "【测试10】GSTEM滚动动画");
    ESP_LOGI(TAG, "动画将持续约15秒...");
    
    tm1640_clear();
    tm1640_refresh();
    
    // 运行动画约15秒（完整循环一次）
    uint32_t start_time = xTaskGetTickCount();
    uint32_t duration_ms = 15000;
    
    while ((xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS < duration_ms) {
        tm1640_show_gstem_scroll_step(50);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    ESP_LOGI(TAG, "✓ GSTEM动画演示完成");
    vTaskDelay(pdMS_TO_TICKS(1000));
}

/**
 * @brief 应用程序入口
 */
void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  TM1640 LED点阵屏测试程序");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "硬件连接：");
    ESP_LOGI(TAG, "  CLK: GPIO34 [SSA3]");
    ESP_LOGI(TAG, "  DIN: GPIO37 [SSA2]");
    ESP_LOGI(TAG, "========================================");
    
    // 初始化板级配置（包含GPIO管理器）
    esp_err_t ret = board_config_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "板级配置初始化失败");
        return;
    }
    
    // 等待系统稳定
    vTaskDelay(pdMS_TO_TICKS(500));
    
    ESP_LOGI(TAG, "开始测试...\n");
    
    // 执行所有测试
    test_basic_init();
    test_single_pixel();
    test_draw_cross();
    test_draw_border();
    test_draw_smile();
    test_brightness();
    test_checkerboard();
    test_progress_bar();
    test_blink();
    test_gstem_animation();
    
    ESP_LOGI(TAG, "\n========================================");
    ESP_LOGI(TAG, "所有测试完成！");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "进入循环演示模式...\n");
    
    // 循环演示模式
    uint32_t loop_count = 0;
    while (1) {
        loop_count++;
        ESP_LOGI(TAG, "========== 循环 %lu ==========", loop_count);
        
        // 笑脸显示
        ESP_LOGI(TAG, "显示笑脸...");
        test_draw_smile();
        
        // 进度条动画
        ESP_LOGI(TAG, "进度条动画...");
        test_progress_bar();
        
        // GSTEM滚动
        ESP_LOGI(TAG, "GSTEM滚动动画...");
        uint32_t start_time = xTaskGetTickCount();
        while ((xTaskGetTickCount() - start_time) * portTICK_PERIOD_MS < 10000) {
            tm1640_show_gstem_scroll_step(50);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
        
        // 打印任务栈使用情况
        UBaseType_t stack_high_water_mark = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI(TAG, "主任务剩余栈空间：%u字节\n", 
                 stack_high_water_mark * sizeof(StackType_t));
    }
}
