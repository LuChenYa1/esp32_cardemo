# 按键检测模块 (Key)

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: ⏸️ 未使用

---

## 组件简介

按键检测模块提供两个独立按键的检测功能，支持按键消抖和状态跟踪。该模块采用轮询方式检测按键状态，并提供了防重复触发机制，确保每次按键按下只触发一次操作。

### 主要特性

- 支持2个独立按键输入（KEY1、KEY2）
- 内置软件消抖功能（50ms延时）
- 防重复触发机制，避免按键抖动导致的误触发
- 低电平有效检测（按下时为0）
- 使用内部上拉电阻，简化硬件连接
- 示例代码包含LED控制功能

### 适用场景

适用于需要用户输入的场景，如模式切换、功能启停控制、参数调整等。可与LED、蜂鸣器等输出设备配合使用，实现人机交互功能。

---

## 硬件连接

### 引脚分配

| 功能 | GPIO引脚 | 接口标识 | 说明 |
|------|---------|---------|------|
| KEY1 | GPIO38 | - | 按键1输入 |
| KEY2 | GPIO37 | - | 按键2输入 |

### 接线说明

1. 将按键1的一端连接到ESP32的GPIO38，另一端连接到GND
2. 将按键2的一端连接到ESP32的GPIO37，另一端连接到GND
3. 无需外部上拉电阻，模块内部已启用上拉电阻
4. 按键按下时GPIO引脚被拉低至GND（低电平有效）

### 电路原理

```
VCC (3.3V)
  |
  ├─── 内部上拉电阻 ───┬─── GPIO38 (KEY1)
  |                    |
  └─── 内部上拉电阻 ───┼─── GPIO37 (KEY2)
                       |
                    按键开关
                       |
                      GND
```

### 注意事项

- ⚠️ **低电平有效**：按键按下时GPIO读取为0（低电平）
- ⚠️ **上拉电阻**：已启用内部上拉电阻，无需外接
- ⚠️ **引脚选择**：GPIO38/37为普通GPIO，可根据需要修改

---

## 功能说明

### 核心功能

#### 功能1：按键初始化

配置GPIO38和GPIO37为输入模式，启用内部上拉电阻，禁用GPIO中断。初始化后按键处于未按下状态（高电平）。

#### 功能2：按键状态检测

通过轮询方式检测按键状态，当检测到GPIO电平从高变低时，判定为按键按下。使用静态变量记录按键状态，防止重复触发。

#### 功能3：按键消抖

采用软件消抖方式，在检测到按键状态变化后延时50ms，避免按键机械抖动导致的误触发。

#### 功能4：LED控制示例

示例代码中实现了两种LED控制方式：
- KEY1：切换LED开关状态（开→关，关→开）
- KEY2：翻转LED状态（使用led_toggle函数）

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| KEY1_GPIO | GPIO38 | GPIO0-48 | 按键1引脚号 |
| KEY2_GPIO | GPIO37 | GPIO0-48 | 按键2引脚号 |
| 消抖延时 | 50ms | 10-100ms | 按键消抖延时时间 |
| 检测电平 | 低电平 | 高/低 | 按键按下时的有效电平 |

---

## API接口

### 初始化函数

```c
/**
 * @brief 初始化按键GPIO引脚
 * 
 * 配置KEY1和KEY2为输入模式，并启用内部上拉电阻
 */
void key_init(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
在使用按键检测功能前必须先调用此函数进行初始化。初始化会配置GPIO为输入模式并启用上拉电阻。

---

### 按键检测任务

```c
/**
 * @brief 按键检测和处理任务
 * 
 * 检测按键状态，当检测到按键按下时执行相应的操作
 * 实现了按键消抖功能，防止重复触发
 */
void key_task(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
此函数应在循环中周期性调用，或在FreeRTOS任务中持续运行。函数内部包含50ms延时用于消抖。示例代码中包含LED控制逻辑，实际使用时需根据需求修改。

---

## 使用示例

### 基本使用示例

```c
#include "key.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "KEY_EXAMPLE";

void key_example_task(void *pvParameters)
{
    // 1. 初始化按键
    key_init();
    ESP_LOGI(TAG, "按键初始化完成");
    
    // 2. 循环检测按键
    while (1) {
        key_task();  // 调用按键检测任务
        // key_task内部已包含延时，无需额外延时
    }
}

void app_main(void)
{
    // 创建按键检测任务
    xTaskCreate(key_example_task, "key_task", 2048, NULL, 5, NULL);
}
```

### 自定义按键处理示例

```c
#include "key.h"
#include "esp_log.h"

static const char *TAG = "KEY_CUSTOM";
static bool key1_pressed = false;
static bool key2_pressed = false;

void custom_key_task(void)
{
    // 检测KEY1
    if (gpio_get_level(KEY1_GPIO) == 0) {
        if (!key1_pressed) {
            key1_pressed = true;
            ESP_LOGI(TAG, "KEY1 按下");
            // 在此添加KEY1按下时的处理逻辑
        }
    } else {
        key1_pressed = false;
    }
    
    // 检测KEY2
    if (gpio_get_level(KEY2_GPIO) == 0) {
        if (!key2_pressed) {
            key2_pressed = true;
            ESP_LOGI(TAG, "KEY2 按下");
            // 在此添加KEY2按下时的处理逻辑
        }
    } else {
        key2_pressed = false;
    }
    
    // 消抖延时
    vTaskDelay(pdMS_TO_TICKS(50));
}
```

### 中断方式检测示例

```c
#include "key.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "KEY_ISR";

// 按键中断处理函数
static void IRAM_ATTR key_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    // 在中断中只做简单标记，复杂处理放在任务中
    ESP_EARLY_LOGI(TAG, "按键中断触发: GPIO%d", gpio_num);
}

void key_init_with_interrupt(void)
{
    // 配置GPIO为输入模式
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << KEY1_GPIO) | (1ULL << KEY2_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE  // 下降沿触发
    };
    gpio_config(&io_conf);
    
    // 安装GPIO中断服务
    gpio_install_isr_service(0);
    
    // 添加中断处理函数
    gpio_isr_handler_add(KEY1_GPIO, key_isr_handler, (void*) KEY1_GPIO);
    gpio_isr_handler_add(KEY2_GPIO, key_isr_handler, (void*) KEY2_GPIO);
}
```

---

## 注意事项

### 硬件限制

- ⚠️ **按键类型**：适用于常开型按键（按下时闭合）
- ⚠️ **电平逻辑**：按键按下时为低电平（0），释放时为高电平（1）
- ⚠️ **引脚限制**：GPIO38/37为普通GPIO，不支持ADC功能

### 软件限制

- ⚠️ **轮询方式**：采用轮询检测，不使用中断，占用一定CPU时间
- ⚠️ **消抖延时**：每次检测包含50ms延时，影响响应速度
- ⚠️ **示例代码**：key_task()中的LED控制逻辑仅为示例，需根据实际需求修改

### 线程安全

- 该模块使用静态变量记录按键状态，不是线程安全的
- 如果多个任务需要检测按键，建议只在一个任务中调用key_task()
- 或者为每个任务维护独立的按键状态变量

### 性能考虑

- 轮询方式会持续占用CPU时间，建议使用中断方式提高效率
- 消抖延时50ms可能影响快速按键的检测，可根据需要调整
- 如需高精度时序，建议使用硬件定时器配合中断

---

## 故障排除

### 常见问题

#### 问题1：按键无响应

**现象**：按下按键后没有任何反应

**原因**：硬件连接错误或初始化未执行

**解决方案**：
1. 检查按键是否正确连接到GPIO38/37和GND
2. 确认已调用key_init()进行初始化
3. 使用万用表测量按键按下时GPIO电平是否为0
4. 检查按键是否损坏（用导线短接GPIO和GND测试）

#### 问题2：按键重复触发

**现象**：按一次按键触发多次操作

**原因**：按键抖动或消抖延时不足

**解决方案**：
1. 增加消抖延时时间（从50ms增加到100ms）
2. 检查按键状态标志是否正确使用
3. 确认按键释放后状态标志被正确重置
4. 考虑添加硬件消抖电路（RC滤波）

#### 问题3：按键检测延迟

**现象**：按键响应速度慢

**原因**：消抖延时过长或轮询频率低

**解决方案**：
1. 减少消抖延时时间（但不要低于10ms）
2. 提高key_task()的调用频率
3. 考虑使用中断方式替代轮询
4. 优化任务调度，提高按键任务优先级

#### 问题4：按键状态混乱

**现象**：按键状态不稳定，时而有效时而无效

**原因**：电源噪声或GPIO配置错误

**解决方案**：
1. 检查电源供电是否稳定
2. 确认上拉电阻已启用
3. 添加硬件滤波电容（0.1uF）
4. 检查GPIO是否被其他模块占用

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [GPIO引脚分配文档](../../docs/GPIO_PIN_ALLOCATION.md)
- [LED组件文档](../led/README.md) - 示例代码中使用的LED控制

### 数据手册

- ESP32-S3技术参考手册 - GPIO章节
- 按键开关规格书

### 代码示例

- 示例代码：`components/key/key.c` - 包含完整的按键检测和LED控制示例

### 相关组件

- [led](../led/README.md) - LED控制模块，示例代码中使用

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，支持2个按键检测 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/key/`  
**文档类型**: 组件使用说明
