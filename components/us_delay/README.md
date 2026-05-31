# 微秒延时模块 (US Delay)

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: ⏸️ 未使用

---

## 组件简介

微秒延时模块提供精确的微秒级和毫秒级延时功能。该模块基于ESP32的高精度定时器，可用于需要精确时序控制的场景。

### 主要特性

- 微秒级延时（udelay）
- 毫秒级延时（mdelay）
- 高精度时序控制
- 简单易用的接口

### 适用场景

适用于需要精确延时的场景，如传感器时序控制、通信协议实现、信号采样等。

---

## 功能说明

### 核心功能

#### 功能1：微秒延时

提供微秒级的精确延时，适用于需要高精度时序的场景。

#### 功能2：毫秒延时

提供毫秒级的延时，适用于一般的延时需求。

---

## API接口

### 微秒延时函数

```c
/**
 * @brief 微秒延时
 * 
 * @param us 延时时间（微秒）
 */
void udelay(int us);
```

**参数说明**：
- `us`: 延时时间（微秒）

**返回值**：
- 无

**使用说明**：
提供微秒级的精确延时。注意：延时期间会阻塞当前任务。

---

### 毫秒延时函数

```c
/**
 * @brief 毫秒延时
 * 
 * @param ms 延时时间（毫秒）
 */
void mdelay(int ms);
```

**参数说明**：
- `ms`: 延时时间（毫秒）

**返回值**：
- 无

**使用说明**：
提供毫秒级的延时。注意：延时期间会阻塞当前任务。

---

## 使用示例

### 基本使用示例

```c
#include "us_delay.h"
#include "esp_log.h"

static const char *TAG = "DELAY_EXAMPLE";

void delay_example(void)
{
    ESP_LOGI(TAG, "开始延时测试");
    
    // 微秒延时
    ESP_LOGI(TAG, "延时100微秒");
    udelay(100);
    
    // 毫秒延时
    ESP_LOGI(TAG, "延时10毫秒");
    mdelay(10);
    
    ESP_LOGI(TAG, "延时测试完成");
}
```

### 传感器时序控制示例

```c
#include "us_delay.h"
#include "driver/gpio.h"

void sensor_timing_example(void)
{
    // 发送触发信号
    gpio_set_level(TRIGGER_PIN, 1);
    udelay(10);  // 10微秒高电平
    gpio_set_level(TRIGGER_PIN, 0);
    
    // 等待传感器响应
    udelay(50);
    
    // 读取传感器数据
    int sensor_value = gpio_get_level(SENSOR_PIN);
}
```

### 通信协议时序示例

```c
void protocol_timing_example(void)
{
    // 发送起始位
    gpio_set_level(DATA_PIN, 0);
    udelay(100);
    
    // 发送数据位
    for (int i = 0; i < 8; i++) {
        gpio_set_level(DATA_PIN, (data >> i) & 1);
        udelay(50);  // 每位50微秒
    }
    
    // 发送停止位
    gpio_set_level(DATA_PIN, 1);
    udelay(100);
}
```

---

## 注意事项

### 软件限制

- ⚠️ **阻塞延时**：延时期间会阻塞当前任务，不适合长时间延时
- ⚠️ **精度限制**：实际延时精度受系统负载和中断影响
- ⚠️ **中断影响**：在中断中使用延时可能影响系统实时性

### 使用建议

- 对于长时间延时（>10ms），建议使用FreeRTOS的vTaskDelay
- 在中断服务程序中避免使用延时函数
- 精确时序要求高的场景，建议使用硬件定时器

### 性能考虑

- 微秒延时会占用CPU，避免频繁调用
- 长时间延时会影响系统响应性
- 建议根据实际需求选择合适的延时方式

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [FreeRTOS延时函数文档](https://www.freertos.org/a00127.html)

### 替代方案

- FreeRTOS vTaskDelay: 适用于毫秒级以上的延时
- ESP-IDF esp_timer: 适用于定时器回调
- 硬件定时器: 适用于高精度周期性任务

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/us_delay/`  
**文档类型**: 组件使用说明
