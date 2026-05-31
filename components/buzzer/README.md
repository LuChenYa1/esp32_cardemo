# Buzzer 蜂鸣器模块

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: 未使用

---

## 组件简介

蜂鸣器模块提供简单的声音提示功能，通过GPIO控制有源蜂鸣器的开关。适用于系统状态提示、报警、按键反馈等场景。

### 主要特性

- 支持有源蜂鸣器控制
- 简单的开关控制接口
- 阻塞式短响功能
- GPIO管理器集成，自动冲突检测
- 适合系统提示音和报警音

### 适用场景

适用于需要简单声音提示的应用场景，如系统启动提示、按键反馈、报警提示等。

---

## 概述

蜂鸣器模块提供简单的声音提示功能，通过GPIO控制有源蜂鸣器的开关。适用于系统状态提示、报警、按键反馈等场景。

## 硬件连接

### 引脚定义

| 功能 | GPIO编号 | 说明 |
|------|---------|------|
| 蜂鸣器控制 | GPIO26 | 高电平响，低电平停 |

### 蜂鸣器类型

支持有源蜂鸣器（内置振荡电路）：
- 工作电压：3.3V / 5V
- 控制方式：高电平触发
- 频率：固定（由蜂鸣器内部决定）

注意：本模块不支持无源蜂鸣器（需要PWM驱动）。

## 功能说明

### 工作原理

有源蜂鸣器内置振荡电路，只需提供直流电压即可发声：
- GPIO输出高电平 → 蜂鸣器响
- GPIO输出低电平 → 蜂鸣器停

### GPIO管理

模块使用`gpio_manager`进行引脚注册，自动检测GPIO冲突，确保引脚不被重复使用。

## API 接口

### `buzzer_init()`
初始化蜂鸣器，配置GPIO为输出模式。

**参数：**
- 无参数

**返回值：**
- 无返回值

**说明：**
- 自动注册GPIO到gpio_manager
- 初始状态为关闭
- 必须在使用其他函数前调用

### `buzzer_on()`
打开蜂鸣器，持续发声。

**参数：**
- 无参数

**返回值：**
- 无返回值

**说明：**
- 蜂鸣器会一直响，直到调用`buzzer_off()`

### `buzzer_off()`
关闭蜂鸣器，停止发声。

**参数：**
- 无参数

**返回值：**
- 无返回值

### `buzzer_beep()`
蜂鸣器短响一次（阻塞）。

**参数：**
- `duration_ms`: 响声持续时间（毫秒）

**返回值：**
- 无返回值

**说明：**
- 函数会阻塞指定时间
- 适合简单的提示音
- 不适合在中断中调用

## 使用示例

### 示例1：基本初始化和控制

```c
#include "buzzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    // 初始化蜂鸣器
    buzzer_init();
    
    // 响1秒
    buzzer_on();
    vTaskDelay(pdMS_TO_TICKS(1000));
    buzzer_off();
    
    // 等待1秒
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 短响100ms
    buzzer_beep(100);
}
```

### 示例2：系统启动提示音

```c
#include "buzzer.h"

void system_startup_sound(void) {
    buzzer_init();
    
    // 三声短响，表示系统启动成功
    for (int i = 0; i < 3; i++) {
        buzzer_beep(100);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

### 示例3：报警音（连续鸣叫）

```c
#include "buzzer.h"

void alarm_sound(void) {
    buzzer_init();
    
    // 连续鸣叫10次
    for (int i = 0; i < 10; i++) {
        buzzer_on();
        vTaskDelay(pdMS_TO_TICKS(200));  // 响200ms
        buzzer_off();
        vTaskDelay(pdMS_TO_TICKS(200));  // 停200ms
    }
}
```

### 示例4：按键反馈音

```c
#include "buzzer.h"
#include "key.h"

void button_feedback_task(void *pvParameters) {
    buzzer_init();
    key_init();
    
    while (1) {
        if (key_is_pressed()) {
            buzzer_beep(50);  // 按键按下时短响50ms
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

### 示例5：不同状态的提示音

```c
#include "buzzer.h"

// 成功提示音：一声长响
void buzzer_success(void) {
    buzzer_beep(500);
}

// 错误提示音：三声短响
void buzzer_error(void) {
    for (int i = 0; i < 3; i++) {
        buzzer_beep(100);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// 警告提示音：两声中响
void buzzer_warning(void) {
    for (int i = 0; i < 2; i++) {
        buzzer_beep(200);
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

void app_main(void) {
    buzzer_init();
    
    // 根据不同情况播放不同提示音
    buzzer_success();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    buzzer_warning();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    buzzer_error();
}
```

### 示例6：非阻塞蜂鸣器任务

```c
#include "buzzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

typedef enum {
    BEEP_SHORT,   // 短响
    BEEP_LONG,    // 长响
    BEEP_ALARM    // 报警
} beep_type_t;

static QueueHandle_t beep_queue;

void buzzer_task(void *pvParameters) {
    buzzer_init();
    beep_type_t beep_type;
    
    while (1) {
        if (xQueueReceive(beep_queue, &beep_type, portMAX_DELAY)) {
            switch (beep_type) {
                case BEEP_SHORT:
                    buzzer_beep(100);
                    break;
                    
                case BEEP_LONG:
                    buzzer_beep(500);
                    break;
                    
                case BEEP_ALARM:
                    for (int i = 0; i < 5; i++) {
                        buzzer_on();
                        vTaskDelay(pdMS_TO_TICKS(200));
                        buzzer_off();
                        vTaskDelay(pdMS_TO_TICKS(200));
                    }
                    break;
            }
        }
    }
}

void app_main(void) {
    // 创建队列
    beep_queue = xQueueCreate(10, sizeof(beep_type_t));
    
    // 创建蜂鸣器任务
    xTaskCreate(buzzer_task, "buzzer_task", 2048, NULL, 5, NULL);
    
    // 其他任务可以通过队列触发蜂鸣器
    beep_type_t beep = BEEP_SHORT;
    xQueueSend(beep_queue, &beep, 0);
}
```

## 依赖

- ESP-IDF driver 组件（GPIO驱动）
- FreeRTOS（任务延时）
- board_config 组件（引脚定义、GPIO管理）

## 注意事项

1. **蜂鸣器类型**: 仅支持有源蜂鸣器，无源蜂鸣器需要PWM驱动
2. **阻塞函数**: `buzzer_beep()`会阻塞当前任务，不适合在中断中调用
3. **GPIO冲突**: GPIO26已固定用于蜂鸣器，不能用于其他功能
4. **音量控制**: 有源蜂鸣器音量固定，无法通过软件调节
5. **频率固定**: 有源蜂鸣器频率由硬件决定，无法改变音调
6. **电流限制**: 确保GPIO输出电流足够驱动蜂鸣器（可能需要三极管驱动）
7. **非阻塞需求**: 如需非阻塞控制，建议创建独立任务（参考示例6）

## 常见问题

### Q: 蜂鸣器不响？
A: 检查：
   - 蜂鸣器是否为有源蜂鸣器
   - 供电是否正常（3.3V或5V）
   - GPIO26连接是否正确
   - 是否调用了`buzzer_init()`
   - 是否需要三极管驱动电路

### Q: 如何改变蜂鸣器音调？
A: 有源蜂鸣器音调固定，无法改变。如需不同音调，请使用：
   - 无源蜂鸣器 + PWM驱动
   - 参考`pcf_buzzer`模块（支持音乐播放）

### Q: 如何实现非阻塞蜂鸣？
A: 两种方案：
   1. 创建独立的蜂鸣器任务（推荐，参考示例6）
   2. 使用定时器回调函数控制

### Q: 蜂鸣器声音太小？
A: 可能原因：
   - GPIO驱动能力不足，需要三极管放大电路
   - 蜂鸣器额定电压高于3.3V，建议使用5V供电
   - 蜂鸣器本身音量较小，更换大功率蜂鸣器

### Q: 如何播放音乐？
A: 本模块不支持音乐播放，请使用：
   - `pcf_buzzer`模块（支持音符和音乐数据）
   - 无源蜂鸣器 + PWM + 音符频率表

## 相关文档

- [引脚定义](../board_config/include/pin_definitions.h)
- [GPIO管理器](../board_config/README.md)
- [PCF蜂鸣器模块](../pcf_buzzer/README.md) - 支持音乐播放
- [ESP-IDF GPIO文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/gpio.html)



---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 添加元信息和版本历史 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/buzzer/`  
**文档类型**: 组件使用说明
