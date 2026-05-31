/**
 * @file main_pcf8574_test.c
 * @brief PCF8574 I2C GPIO 扩展芯片测试程序
 * 
 * 测试内容：
 * 1. I2C 总线扫描，查找 PCF8574 设备
 * 2. 端口读写测试（8位并行操作）
 * 3. 单个引脚读写测试
 * 4. PWM 呼吸灯效果（软件模拟）
 * 5. LED 流水灯效果
 * 
 * 硬件连接：
 * - PCF8574 地址：0x20（默认）
 * - SDA：GPIO20
 * - SCL：GPIO21
 * - P0-P7：连接 LED 或其他负载
 * 
 * 注意事项：
 * - 确保 I2C 总线有上拉电阻（4.7kΩ）
 * - PCF8574 引脚默认为高电平
 * - 本测试使用新的独立 pcf8574 组件（不依赖 managed_components）
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "pcf8574.h"

static const char *TAG = "PCF8574_TEST";

// PCF8574 设备描述符
static i2c_dev_t pcf_dev;
static bool pcf_initialized = false;

// 二进制显示宏（需要在使用前定义）
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

// ==================== I2C 总线扫描 ====================

/**
 * @brief 扫描 I2C 总线上的所有设备
 * 
 * 扫描地址范围 0x00-0x7F，检测哪些地址有响应
 */
void i2c_scan_bus(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "开始扫描 I2C 总线");
    ESP_LOGI(TAG, "  SDA: GPIO20");
    ESP_LOGI(TAG, "  SCL: GPIO21");
    ESP_LOGI(TAG, "========================================");
    
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");

    int device_count = 0;
    
    for (uint8_t addr = 0; addr < 128; addr++)
    {
        if (addr % 16 == 0)
        {
            printf("%02x: ", addr);
        }

        // 尝试初始化设备（静默模式，不打印日志）
        i2c_dev_t test_dev;
        esp_log_level_set("PCF8574", ESP_LOG_NONE);  // 临时关闭日志
        esp_err_t ret = pcf8574_init_desc(&test_dev, addr, I2C_NUM_0, 
                                          GPIO_NUM_20, GPIO_NUM_21);
        
        if (ret == ESP_OK)
        {
            // 尝试读取一个字节
            uint8_t dummy;
            ret = pcf8574_port_read(&test_dev, &dummy);
            pcf8574_free_desc(&test_dev);

            if (ret == ESP_OK)
            {
                printf("%02x ", addr);
                device_count++;
            }
            else
            {
                printf("-- ");
            }
        }
        else
        {
            printf("-- ");
        }

        if ((addr + 1) % 16 == 0)
        {
            printf("\n");
        }
    }
    
    esp_log_level_set("PCF8574", ESP_LOG_INFO);  // 恢复日志级别

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "I2C 扫描完成，发现 %d 个设备", device_count);
    ESP_LOGI(TAG, "========================================");
}

// ==================== PCF8574 初始化 ====================

/**
 * @brief 初始化 PCF8574 设备
 */
esp_err_t pcf8574_test_init(void)
{
    if (pcf_initialized) {
        ESP_LOGW(TAG, "PCF8574 已初始化");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "初始化 PCF8574 设备");
    ESP_LOGI(TAG, "  地址: 0x20");
    ESP_LOGI(TAG, "  SDA: GPIO20");
    ESP_LOGI(TAG, "  SCL: GPIO21");

    esp_err_t ret = pcf8574_init_desc(&pcf_dev, 0x20, I2C_NUM_0, 
                                      GPIO_NUM_20, GPIO_NUM_21);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCF8574 初始化失败: %s", esp_err_to_name(ret));
        return ret;
    }

    pcf_initialized = true;
    ESP_LOGI(TAG, "PCF8574 初始化成功");
    return ESP_OK;
}

// ==================== 测试任务 1：端口读写测试 ====================

/**
 * @brief 端口读写测试任务
 * 
 * 测试 8 位并行读写功能
 */
void pcf8574_port_rw_test_task(void *pvParameters)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "测试 1：端口读写测试");
    ESP_LOGI(TAG, "========================================");

    // 测试写入不同的值
    uint8_t test_values[] = {0x00, 0xFF, 0xAA, 0x55, 0x0F, 0xF0};
    
    for (int i = 0; i < sizeof(test_values); i++) {
        uint8_t write_val = test_values[i];
        
        // 写入
        esp_err_t ret = pcf8574_port_write(&pcf_dev, write_val);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "写入失败: %s", esp_err_to_name(ret));
            continue;
        }
        
        ESP_LOGI(TAG, "写入: 0x%02X (二进制: " BYTE_TO_BINARY_PATTERN ")", 
                 write_val, BYTE_TO_BINARY(write_val));
        
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // 读取
        uint8_t read_val;
        ret = pcf8574_port_read(&pcf_dev, &read_val);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "读取: 0x%02X (二进制: " BYTE_TO_BINARY_PATTERN ")", 
                     read_val, BYTE_TO_BINARY(read_val));
        } else {
            ESP_LOGE(TAG, "读取失败: %s", esp_err_to_name(ret));
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGI(TAG, "端口读写测试完成");
    vTaskDelete(NULL);
}

// ==================== 测试任务 2：单引脚读写测试 ====================

/**
 * @brief 单引脚读写测试任务
 * 
 * 测试单个引脚的读写功能
 */
void pcf8574_pin_rw_test_task(void *pvParameters)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "测试 2：单引脚读写测试");
    ESP_LOGI(TAG, "========================================");

    // 依次测试 P0-P7
    for (uint8_t pin = 0; pin < 8; pin++) {
        ESP_LOGI(TAG, "测试引脚 P%d", pin);
        
        // 设置为高电平
        esp_err_t ret = pcf8574_set_level(&pcf_dev, pin, 1);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "设置 P%d 为高电平失败", pin);
            continue;
        }
        ESP_LOGI(TAG, "  P%d -> 高电平", pin);
        vTaskDelay(pdMS_TO_TICKS(300));
        
        // 读取电平
        uint32_t level;
        ret = pcf8574_get_level(&pcf_dev, pin, &level);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "  P%d 读取: %s", pin, level ? "高" : "低");
        }
        
        // 设置为低电平
        ret = pcf8574_set_level(&pcf_dev, pin, 0);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "设置 P%d 为低电平失败", pin);
            continue;
        }
        ESP_LOGI(TAG, "  P%d -> 低电平", pin);
        vTaskDelay(pdMS_TO_TICKS(300));
        
        // 读取电平
        ret = pcf8574_get_level(&pcf_dev, pin, &level);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "  P%d 读取: %s", pin, level ? "高" : "低");
        }
    }

    ESP_LOGI(TAG, "单引脚读写测试完成");
    vTaskDelete(NULL);
}

// ==================== 测试任务 3：LED 流水灯 ====================

/**
 * @brief LED 流水灯测试任务
 * 
 * P0-P7 依次点亮，形成流水灯效果
 */
void pcf8574_led_running_task(void *pvParameters)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "测试 3：LED 流水灯效果");
    ESP_LOGI(TAG, "========================================");

    for (int round = 0; round < 5; round++) {
        ESP_LOGI(TAG, "第 %d 轮流水灯", round + 1);
        
        // 正向流水
        for (uint8_t pin = 0; pin < 8; pin++) {
            uint8_t value = (1 << pin);
            pcf8574_port_write(&pcf_dev, ~value);  // 低电平点亮
            ESP_LOGI(TAG, "  点亮 P%d", pin);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        // 反向流水
        for (int pin = 7; pin >= 0; pin--) {
            uint8_t value = (1 << pin);
            pcf8574_port_write(&pcf_dev, ~value);  // 低电平点亮
            ESP_LOGI(TAG, "  点亮 P%d", pin);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }

    // 全部熄灭
    pcf8574_port_write(&pcf_dev, 0xFF);
    ESP_LOGI(TAG, "LED 流水灯测试完成");
    vTaskDelete(NULL);
}

// ==================== 测试任务 4：PWM 呼吸灯 ====================

/**
 * @brief 使用 PCF8574 模拟 PWM 输出
 * 
 * @param pin_mask 引脚掩码（例如 0x01 表示 P0）
 * @param duty_cycle 占空比 (0-100)
 */
void pcf8574_pwm_simulation(uint8_t pin_mask, int duty_cycle)
{
    uint8_t state;
    pcf8574_port_read(&pcf_dev, &state);

    if (duty_cycle >= 100) {
        // 100% 占空比，持续高电平
        state |= pin_mask;
        pcf8574_port_write(&pcf_dev, state);
    } else if (duty_cycle <= 0) {
        // 0% 占空比，持续低电平
        state &= ~pin_mask;
        pcf8574_port_write(&pcf_dev, state);
    } else {
        // 软件 PWM：周期约 30ms，每次切换 300us
        int high_count = duty_cycle;
        int low_count = 100 - high_count;

        state |= pin_mask;
        for (int i = 0; i < high_count; i++) {
            pcf8574_port_write(&pcf_dev, state);
            esp_rom_delay_us(300);
        }
        
        state &= ~pin_mask;
        for (int i = 0; i < low_count; i++) {
            pcf8574_port_write(&pcf_dev, state);
            esp_rom_delay_us(300);
        }
    }
}

/**
 * @brief PWM 呼吸灯测试任务
 * 
 * 在 P2 引脚上实现呼吸灯效果
 */
void pcf8574_pwm_breathing_task(void *pvParameters)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "测试 4：PWM 呼吸灯效果（P2 引脚）");
    ESP_LOGI(TAG, "========================================");

    uint8_t p2_mask = (1 << 2);  // P2 引脚
    int brightness = 0;
    int direction = 1;

    for (int cycle = 0; cycle < 10; cycle++) {
        ESP_LOGI(TAG, "呼吸灯周期 %d/10", cycle + 1);
        
        while (1) {
            pcf8574_pwm_simulation(p2_mask, brightness);
            
            brightness += direction;
            
            if (brightness >= 100) {
                brightness = 100;
                direction = -1;
            } else if (brightness <= 0) {
                brightness = 0;
                direction = 1;
                break;  // 一个周期完成
            }
        }
    }

    // 关闭 LED
    pcf8574_port_write(&pcf_dev, 0xFF);
    ESP_LOGI(TAG, "PWM 呼吸灯测试完成");
    vTaskDelete(NULL);
}

// ==================== 主程序 ====================

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "PCF8574 组件测试程序");
    ESP_LOGI(TAG, "版本: 独立组件（不依赖 managed_components）");
    ESP_LOGI(TAG, "========================================");

    // 延时，等待系统稳定
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 步骤 1：扫描 I2C 总线
    i2c_scan_bus();
    vTaskDelay(pdMS_TO_TICKS(2000));

    // 步骤 2：初始化 PCF8574
    esp_err_t ret = pcf8574_test_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCF8574 初始化失败，测试终止");
        return;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));

    // 步骤 3：运行测试任务（依次执行）
    
    // 测试 1：端口读写
    xTaskCreate(pcf8574_port_rw_test_task, "port_rw_test", 4096, NULL, 5, NULL);
    vTaskDelay(pdMS_TO_TICKS(8000));  // 等待测试完成

    // 测试 2：单引脚读写
    xTaskCreate(pcf8574_pin_rw_test_task, "pin_rw_test", 4096, NULL, 5, NULL);
    vTaskDelay(pdMS_TO_TICKS(6000));  // 等待测试完成

    // 测试 3：LED 流水灯
    xTaskCreate(pcf8574_led_running_task, "led_running", 4096, NULL, 5, NULL);
    vTaskDelay(pdMS_TO_TICKS(12000));  // 等待测试完成

    // 测试 4：PWM 呼吸灯
    xTaskCreate(pcf8574_pwm_breathing_task, "pwm_breathing", 4096, NULL, 5, NULL);
    vTaskDelay(pdMS_TO_TICKS(15000));  // 等待测试完成

    // 所有测试完成
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "所有测试完成！");
    ESP_LOGI(TAG, "========================================");

    // 清理资源
    pcf8574_free_desc(&pcf_dev);
    ESP_LOGI(TAG, "PCF8574 资源已释放");
}
