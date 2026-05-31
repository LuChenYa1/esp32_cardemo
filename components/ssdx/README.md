# SSD接口模块 (SSDX)

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: ⏸️ 未使用

---

## 组件简介

SSD接口模块提供对SSD（Smart Sensor Digital）接口的ADC读取功能。该模块通过ESP32的ADC外设读取模拟传感器的电压值，支持多通道ADC采样。

### 主要特性

- 支持多通道ADC采样
- 电压值读取（mV）
- 简单的初始化和读取接口
- 适用于模拟传感器数据采集

### 适用场景

适用于需要读取模拟传感器数据的场景，如电压监测、模拟传感器采集、信号测量等。

---

## 硬件连接

### 引脚分配

| 功能 | GPIO引脚 | 接口标识 | 说明 |
|------|---------|---------|------|
| ADC通道0 | GPIO1 | SSD1 | 模拟输入通道0 |
| ADC通道1 | GPIO2 | SSD2 | 模拟输入通道1 |

### 接线说明

1. 将模拟传感器的输出连接到ESP32的ADC引脚（GPIO1或GPIO2）
2. 确保传感器的电源和地线正确连接
3. 输入电压范围：0-3.3V

### 注意事项

- ⚠️ **电压范围**：ADC输入电压不能超过3.3V，否则会损坏芯片
- ⚠️ **ADC精度**：ESP32 ADC精度有限，建议进行校准
- ⚠️ **WiFi冲突**：ADC2与WiFi共用，使用WiFi时ADC2不可用

---

## 功能说明

### 核心功能

#### 功能1：ADC初始化

配置ADC通道和参数，初始化ADC外设。

#### 功能2：电压读取

读取指定ADC通道的电压值，返回mV单位的电压。

---

## API接口

### 初始化函数

```c
/**
 * @brief 初始化ADC
 */
void ssdx_adc_init(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
在使用ADC前必须先调用此函数进行初始化。

---

### 电压读取函数

```c
/**
 * @brief 读取ADC电压
 * 
 * @param channel ADC通道号（0: GPIO1, 1: GPIO2）
 * @return 电压值（mV）
 */
int ssdx_read_voltage(int channel);
```

**参数说明**：
- `channel`: ADC通道号
  - 0: GPIO1
  - 1: GPIO2

**返回值**：
- 电压值（mV）

**使用说明**：
读取指定通道的ADC电压值，返回以mV为单位的电压。

---

## 使用示例

### 基本使用示例

```c
#include "ssdx.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SSDX_EXAMPLE";

void ssdx_example_task(void *pvParameters)
{
    // 1. 初始化ADC
    ssdx_adc_init();
    ESP_LOGI(TAG, "ADC初始化完成");
    
    // 2. 循环读取电压
    while (1) {
        // 读取通道0电压
        int voltage_ch0 = ssdx_read_voltage(0);
        ESP_LOGI(TAG, "通道0电压: %d mV", voltage_ch0);
        
        // 读取通道1电压
        int voltage_ch1 = ssdx_read_voltage(1);
        ESP_LOGI(TAG, "通道1电压: %d mV", voltage_ch1);
        
        vTaskDelay(pdMS_TO_TICKS(1000));  // 1秒采样间隔
    }
}

void app_main(void)
{
    xTaskCreate(ssdx_example_task, "ssdx_task", 2048, NULL, 5, NULL);
}
```

### 电压监测示例

```c
void voltage_monitor_example(void)
{
    ssdx_adc_init();
    
    while (1) {
        int voltage = ssdx_read_voltage(0);
        
        // 电压阈值检测
        if (voltage < 500) {
            ESP_LOGW(TAG, "电压过低: %d mV", voltage);
        } else if (voltage > 3000) {
            ESP_LOGW(TAG, "电压过高: %d mV", voltage);
        } else {
            ESP_LOGI(TAG, "电压正常: %d mV", voltage);
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

---

## 注意事项

### 硬件限制

- ⚠️ **电压范围**：输入电压必须在0-3.3V范围内
- ⚠️ **ADC2限制**：如果使用WiFi，ADC2通道不可用
- ⚠️ **精度限制**：ESP32 ADC精度约为12位，但实际精度受多种因素影响

### 软件限制

- ⚠️ **采样率**：高速采样可能影响系统性能
- ⚠️ **校准**：建议进行ADC校准以提高精度

### 性能考虑

- ADC采样需要一定时间，避免过高的采样频率
- 建议添加滤波算法减少噪声影响

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [GPIO引脚分配文档](../../docs/GPIO_PIN_ALLOCATION.md)
- [ESP-IDF ADC文档](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/peripherals/adc.html)

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/ssdx/`  
**文档类型**: 组件使用说明
