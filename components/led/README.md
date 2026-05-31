# LED 指示灯模块

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: 未使用

---

## 概述

LED模块提供简单的LED指示灯控制功能，通过GPIO控制LED的亮灭和闪烁。适用于系统状态指示、调试提示、运行状态显示等场景。

## 硬件连接

### 引脚定义

| 功能 | GPIO编号 | 说明 |
|------|---------|------|
| LED控制 | GPIO0 | 低电平亮，高电平灭 |

### LED连接方式

本模块采用低电平点亮方式（共阳极）：
- GPIO输出低电平（0） → LED点亮
- GPIO输出高电平（1） → LED熄灭

注意：如果使用共阴极LED，需要修改代码逻辑。

## 功能说明

### 工作原理

通过GPIO控制LED的亮灭状态：
- `led_on()`: 设置GPIO为低电平，LED点亮
- `led_off()`: 设置GPIO为高电平，LED熄灭
- `led_toggle()`: 翻转GPIO电平，LED状态切换

### GPIO管理

模块使用`gpio_manager`进行引脚注册，自动检测GPIO冲突，确保引脚不被重复使用。

## API 接口

### `led_init()`
初始化LED，配置GPIO为输出模式。

**参数：**
- 无参数

**返回值：**
- 无返回值

**说明：**
- 自动注册GPIO到gpio_manager
- 必须在使用其他函数前调用

### `led_on()`
点亮LED。

**参数：**
- 无参数

**返回值：**
- 无返回值

### `led_off()`
熄灭LED。

**参数：**
- 无参数

**返回值：**
- 无返回值

### `led_toggle()`
翻转LED状态（亮→灭，灭→亮）。

**参数：**
- 无参数

**返回值：**
- 无返回值

## 使用示例

### 示例1：基本控制

```c
#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    // 初始化LED
    led_init();
    
    // 点亮LED
    led_on();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 熄灭LED
    led_off();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 翻转LED状态
    led_toggle();
}
```

### 示例2：LED闪烁

```c
#include "led.h"

void led_blink_task(void *pvParameters) {
    led_init();
    
    while (1) {
        led_toggle();
        vTaskDelay(pdMS_TO_TICKS(500));  // 每500ms翻转一次
    }
}

void app_main(void) {
    xTaskCreate(led_blink_task, "led_blink", 2048, NULL, 5, NULL);
}
```

### 示例3：系统状态指示

```c
#include "led.h"

typedef enum {
    SYS_IDLE,       // 空闲：LED熄灭
    SYS_RUNNING,    // 运行：LED常亮
    SYS_ERROR       // 错误：LED快速闪烁
} system_state_t;

void led_indicate_state(system_state_t state) {
    led_init();
    
    switch (state) {
        case SYS_IDLE:
            led_off();
            break;
            
        case SYS_RUNNING:
            led_on();
            break;
            
        case SYS_ERROR:
            // 快速闪烁
            for (int i = 0; i < 10; i++) {
                led_toggle();
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            break;
    }
}
```

### 示例4：心跳指示

```c
#include "led.h"

void heartbeat_task(void *pvParameters) {
    led_init();
    
    while (1) {
        // 模拟心跳：快闪两次
        led_on();
        vTaskDelay(pdMS_TO_TICKS(100));
        led_off();
        vTaskDelay(pdMS_TO_TICKS(100));
        
        led_on();
        vTaskDelay(pdMS_TO_TICKS(100));
        led_off();
        vTaskDelay(pdMS_TO_TICKS(700));  // 等待下一次心跳
    }
}
```

### 示例5：不同频率闪烁

```c
#include "led.h"

// 慢闪：1Hz (每秒1次)
void led_blink_slow(void) {
    led_toggle();
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// 中速闪烁：2Hz (每秒2次)
void led_blink_medium(void) {
    led_toggle();
    vTaskDelay(pdMS_TO_TICKS(500));
}

// 快闪：5Hz (每秒5次)
void led_blink_fast(void) {
    led_toggle();
    vTaskDelay(pdMS_TO_TICKS(200));
}

void app_main(void) {
    led_init();
    
    while (1) {
        // 根据不同条件选择闪烁频率
        if (error_detected) {
            led_blink_fast();
        } else if (warning_detected) {
            led_blink_medium();
        } else {
            led_blink_slow();
        }
    }
}
```

### 示例6：LED与按键联动

```c
#include "led.h"
#include "key.h"

void led_key_task(void *pvParameters) {
    led_init();
    key_init();
    
    while (1) {
        if (key_is_pressed()) {
            led_on();   // 按键按下时点亮LED
        } else {
            led_off();  // 按键释放时熄灭LED
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

## 依赖

- ESP-IDF driver 组件（GPIO驱动）
- FreeRTOS（任务延时）
- board_config 组件（引脚定义、GPIO管理）

## 注意事项

1. **电平逻辑**: 本模块使用低电平点亮（共阳极），如需共阴极请修改代码
2. **GPIO冲突**: GPIO0已固定用于LED，不能用于其他功能
3. **启动状态**: GPIO0在ESP32-S3启动时有特殊用途，上电时LED可能闪烁
4. **电流限制**: 确保LED串联限流电阻，避免GPIO过流损坏
5. **闪烁任务**: 建议创建独立任务控制LED闪烁，避免阻塞主任务
6. **日志输出**: LED操作会输出日志，高频闪烁时可能影响性能

## 常见问题

### Q: LED不亮？
A: 检查：
   - LED极性是否正确
   - 是否串联了限流电阻
   - GPIO0连接是否正确
   - 是否调用了`led_init()`
   - 电平逻辑是否匹配（共阳/共阴）

### Q: 如何改变LED亮度？
A: 本模块不支持亮度调节，如需调光请使用PWM：
   ```c
   // 使用LEDC实现LED调光
   ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
   ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
   ```

### Q: 如何实现呼吸灯效果？
A: 使用PWM逐渐改变占空比：
   ```c
   for (int duty = 0; duty <= 1023; duty += 10) {
       ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
       ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
       vTaskDelay(pdMS_TO_TICKS(20));
   }
   ```

### Q: LED闪烁频率不稳定？
A: 可能原因：
   - 任务优先级太低，被其他任务抢占
   - 使用了阻塞函数导致延时不准确
   - 建议使用硬件定时器或提高任务优先级

### Q: 如何控制多个LED？
A: 修改代码支持多个GPIO：
   ```c
   #define LED1_GPIO GPIO_NUM_0
   #define LED2_GPIO GPIO_NUM_1
   
   void led_init_multi(void) {
       // 初始化多个LED
   }
   ```

## 相关文档

- [引脚定义](../board_config/include/pin_definitions.h)
- [GPIO管理器](../board_config/README.md)
- [按键模块](../key/README.md) - LED与按键联动
- [ESP-IDF GPIO文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/gpio.html)

