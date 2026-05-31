# TM1640 LED点阵屏模块

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: 未使用

---

## 概述

TM1640是一款LED点阵屏驱动芯片，支持8x16点阵显示。通过两线串行接口（CLK + DIN）控制，可显示文字、图案、动画等内容。适用于状态显示、信息提示、动画演示等场景。

## 硬件连接

### 引脚定义

| 功能 | GPIO编号 | 接口位置 | 说明 |
|------|---------|---------|------|
| CLK（时钟） | GPIO34 | SSA3 | 时钟信号线 |
| DIN（数据） | GPIO37 | SSA2 | 数据信号线 |

### 通信协议

TM1640使用类似I2C的两线串行协议：
- 起始信号：DIN从高到低，CLK保持高
- 数据传输：LSB先发送，每个时钟周期传输1位
- 停止信号：CLK从低到高，然后DIN从低到高

### 点阵规格

- 行数：8行（row: 0-7）
- 列数：16列（col: 0-15）
- 总像素：128个LED
- 亮度等级：8级（0-7，7最亮）

## 功能说明

### 工作原理

TM1640通过显示缓存控制LED点阵：
- 缓存大小：16字节（每字节对应一列的8行）
- 数据格式：每字节的bit0-bit7对应该列的第0-7行
- 刷新机制：修改缓存后需调用 `tm1640_refresh()` 才能显示

### 显示流程

1. 初始化：配置GPIO，清空显示
2. 绘制图案：通过 `tm1640_set_led()` 设置像素点
3. 刷新显示：调用 `tm1640_refresh()` 将缓存写入芯片
4. 调节亮度：使用 `tm1640_set_brightness()` 设置亮度

### GPIO管理

模块使用 `gpio_manager` 进行引脚注册，自动检测GPIO冲突，确保引脚不被重复使用。

## API 接口

### `tm1640_init()`
初始化TM1640，配置GPIO并清空显示。

**参数：**
- 无参数

**返回值：**
- 无返回值

**说明：**
- 自动注册GPIO到gpio_manager
- 设置GPIO为输出模式
- 清空显示并设置最高亮度
- 必须在使用其他函数前调用

### `tm1640_clear()`
清空显示缓存。

**参数：**
- 无参数

**返回值：**
- 无返回值

**说明：**
- 仅清空内存缓存，需调用 `tm1640_refresh()` 才能生效

### `tm1640_set_led(uint8_t row, uint8_t col, uint8_t on)`
设置单个像素点的亮灭状态。

**参数：**
- `row`: 行号（0-7）
- `col`: 列号（0-15）
- `on`: 1=点亮，0=熄灭

**返回值：**
- 无返回值

**说明：**
- 超出范围的坐标会被忽略
- 修改后需调用 `tm1640_refresh()` 才能显示

### `tm1640_refresh()`
刷新显示，将缓存数据写入TM1640芯片。

**参数：**
- 无参数

**返回值：**
- 无返回值

**说明：**
- 每次修改缓存后必须调用此函数
- 刷新时间约1-2ms

### `tm1640_set_brightness(uint8_t level)`
设置显示亮度。

**参数：**
- `level`: 亮度等级（0-7，7最亮）

**返回值：**
- 无返回值

**说明：**
- 超过7的值会被限制为7
- 立即生效，无需调用 `tm1640_refresh()`

### `tm1640_show_gstem_scroll_step(uint16_t step_delay_ms)`
显示GSTEM滚动动画（每次调用滚动一步）。

**参数：**
- `step_delay_ms`: 每步延时（毫秒）

**返回值：**
- 无返回值

**说明：**
- 需要在主循环中周期性调用
- 显示"GSTEM"文字从右向左滚动
- 滚动完成后显示笑脸图案
- 笑脸停留1秒后重新开始滚动

## 使用示例

### 示例1：基本初始化和显示

```c
#include "tm1640.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    // 初始化TM1640
    tm1640_init();
    
    // 清空显示
    tm1640_clear();
    
    // 绘制一个十字
    for (uint8_t i = 0; i < 16; i++) {
        tm1640_set_led(4, i, 1);  // 水平线
    }
    for (uint8_t i = 0; i < 8; i++) {
        tm1640_set_led(i, 8, 1);  // 垂直线
    }
    
    // 刷新显示
    tm1640_refresh();
}
```

### 示例2：绘制笑脸

```c
void draw_smile(void) {
    tm1640_init();
    tm1640_clear();
    
    // 绘制脸部轮廓
    for (uint8_t c = 3; c <= 12; c++) {
        tm1640_set_led(1, c, 1);  // 上边
        tm1640_set_led(6, c, 1);  // 下边
    }
    for (uint8_t r = 2; r <= 5; r++) {
        tm1640_set_led(r, 2, 1);   // 左边
        tm1640_set_led(r, 13, 1);  // 右边
    }
    
    // 绘制眼睛
    tm1640_set_led(3, 5, 1);   // 左眼
    tm1640_set_led(3, 10, 1);  // 右眼
    
    // 绘制嘴巴
    for (uint8_t c = 5; c <= 10; c++) {
        tm1640_set_led(5, c, 1);
    }
    
    tm1640_refresh();
}
```

### 示例3：显示数字

```c
// 显示数字"8"的字模（5x7点阵）
void draw_number_8(uint8_t start_col) {
    const uint8_t pattern[5] = {
        0b01110,  // 第1列
        0b10001,  // 第2列
        0b01110,  // 第3列
        0b10001,  // 第4列
        0b01110   // 第5列
    };
    
    for (uint8_t col = 0; col < 5; col++) {
        for (uint8_t row = 0; row < 7; row++) {
            if (pattern[col] & (1 << row)) {
                tm1640_set_led(row, start_col + col, 1);
            }
        }
    }
    tm1640_refresh();
}

void app_main(void) {
    tm1640_init();
    tm1640_clear();
    draw_number_8(5);  // 在第5列开始绘制
}
```

### 示例4：亮度调节

```c
void brightness_demo(void) {
    tm1640_init();
    
    // 绘制全屏点亮
    for (uint8_t r = 0; r < 8; r++) {
        for (uint8_t c = 0; c < 16; c++) {
            tm1640_set_led(r, c, 1);
        }
    }
    tm1640_refresh();
    
    // 从暗到亮渐变
    for (uint8_t level = 0; level <= 7; level++) {
        tm1640_set_brightness(level);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    // 从亮到暗渐变
    for (int8_t level = 7; level >= 0; level--) {
        tm1640_set_brightness(level);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

### 示例5：GSTEM滚动动画

```c
void gstem_animation_task(void *pvParameters) {
    tm1640_init();
    
    while (1) {
        // 每50ms滚动一步
        tm1640_show_gstem_scroll_step(50);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void app_main(void) {
    xTaskCreate(gstem_animation_task, "gstem_anim", 2048, NULL, 5, NULL);
}
```

### 示例6：闪烁效果

```c
void blink_pattern(void) {
    tm1640_init();
    
    // 绘制图案
    for (uint8_t c = 0; c < 16; c += 2) {
        for (uint8_t r = 0; r < 8; r++) {
            tm1640_set_led(r, c, 1);
        }
    }
    
    // 闪烁10次
    for (int i = 0; i < 10; i++) {
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
}
```

### 示例7：滚动文字

```c
void scroll_text_task(void *pvParameters) {
    tm1640_init();
    
    // 定义字母"A"的字模（5x7）
    const uint8_t letter_a[5] = {0x7E, 0x11, 0x11, 0x11, 0x7E};
    
    while (1) {
        // 从右向左滚动
        for (int offset = 16; offset >= -5; offset--) {
            tm1640_clear();
            
            // 绘制字母
            for (uint8_t col = 0; col < 5; col++) {
                int display_col = offset + col;
                if (display_col >= 0 && display_col < 16) {
                    for (uint8_t row = 0; row < 8; row++) {
                        if (letter_a[col] & (1 << row)) {
                            tm1640_set_led(row, display_col, 1);
                        }
                    }
                }
            }
            
            tm1640_refresh();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}
```

### 示例8：进度条显示

```c
void show_progress(uint8_t percent) {
    tm1640_clear();
    
    // 计算点亮的列数（0-100% 映射到 0-16列）
    uint8_t cols = (percent * 16) / 100;
    
    // 绘制进度条（使用中间3行）
    for (uint8_t c = 0; c < cols; c++) {
        tm1640_set_led(3, c, 1);
        tm1640_set_led(4, c, 1);
        tm1640_set_led(5, c, 1);
    }
    
    tm1640_refresh();
}

void progress_demo(void) {
    tm1640_init();
    
    // 从0%到100%
    for (uint8_t p = 0; p <= 100; p += 5) {
        show_progress(p);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
```

## 依赖

- ESP-IDF driver 组件（GPIO驱动）
- FreeRTOS（任务延时）
- board_config 组件（引脚定义、GPIO管理）

## 注意事项

1. **引脚复用**: GPIO34/37与TM1637数码管共用，不能同时使用两个模块
2. **刷新机制**: 修改显示缓存后必须调用 `tm1640_refresh()` 才能生效
3. **坐标范围**: row范围0-7，col范围0-15，超出范围会被忽略
4. **亮度调节**: 亮度范围0-7，建议使用5-7以获得较好的显示效果
5. **内存占用**: 显示缓存仅16字节，内存占用极小
6. **时序要求**: 使用10us延时满足TM1640时序要求
7. **初始化顺序**: 必须先调用 `tm1640_init()` 再使用其他函数
8. **GPIO冲突**: 使用gpio_manager自动检测冲突，初始化失败时检查日志

## 常见问题

### Q: 显示不亮或显示异常？
A: 检查：
   - 是否调用了 `tm1640_init()`
   - 修改缓存后是否调用了 `tm1640_refresh()`
   - GPIO连接是否正确（CLK=GPIO34, DIN=GPIO37）
   - 电源供电是否正常
   - 亮度是否设置过低

### Q: 如何显示自定义图案？
A: 使用 `tm1640_set_led()` 逐个设置像素点，或者定义字模数组批量设置

### Q: 刷新速度慢怎么办？
A: TM1640刷新时间约1-2ms，如需高速动画建议：
   - 减少刷新频率
   - 只更新变化的部分
   - 使用硬件定时器控制刷新

### Q: 可以同时使用TM1637和TM1640吗？
A: 不可以，它们共用GPIO34/37，只能选择其中一个使用

### Q: 如何实现滚动效果？
A: 参考示例7，通过循环移动缓存数据实现滚动

### Q: 亮度调节不明显？
A: 低亮度等级（0-3）差异较小，建议使用5-7级

### Q: 如何显示汉字？
A: 需要准备汉字字模（16x16点阵），然后使用 `tm1640_set_led()` 绘制

## 相关文档

- [引脚定义](../board_config/include/pin_definitions.h)
- [GPIO管理器](../board_config/README.md)
- [TM1637数码管模块](../tm1637/README.md) - 共用引脚的另一个显示模块
- [TM1640数据手册](https://www.titanmec.com/index.php/en/project/download/id/302.html)
