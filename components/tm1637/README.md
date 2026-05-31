# TM1637 四位数码管显示驱动

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: main.c使用中

---

## 📖 组件简介

TM1637 是一款专用的 LED 数码管驱动芯片，采用两线串行接口（类似 I2C），可以驱动 4 位共阳数码管。本组件提供了完整的驱动程序，支持数字显示、亮度控制等功能。

**主要特性**：
- 4 位 8 段数码管驱动
- 两线串行接口（CLK + DIO）
- 8 级亮度调节
- 内置键盘扫描功能（本驱动未实现）
- 自动显示刷新

**适用场景**：
- 温湿度显示
- 距离测量显示
- 时钟显示
- 计数器显示
- 状态指示

---

## 🔌 硬件连接

### 引脚定义

| 引脚名称 | GPIO | 接口标识 | 方向 | 说明 |
|---------|------|---------|------|------|
| CLK | GPIO34 | SSA3 | 输出 | 时钟线（开漏输出，需外部上拉） |
| DIO | GPIO37 | SSA2 | 双向 | 数据线（开漏输出，需外部上拉） |
| VCC | 5V/3.3V | - | 电源 | 供电电压 |
| GND | GND | - | 地 | 接地 |

### 接线说明

```
TM1637          ESP32-S3
---------       ---------
  VCC   ------>   5V 或 3.3V
  CLK   ------>   GPIO34 [SSA3]
  DIO   ------>   GPIO37 [SSA2]
  GND   ------>   GND
```

**注意事项**：
- GPIO34 只能作为输入，TM1637 使用开漏输出模式
- CLK 和 DIO 需要外部上拉电阻（通常 TM1637 模块已集成）
- 推荐上拉电阻值：4.7kΩ - 10kΩ

---

## ⚙️ 功能说明

### 工作原理

TM1637 采用类似 I2C 的两线串行通信协议：
1. **起始信号**：DIO 从高到低，CLK 保持高电平
2. **数据传输**：CLK 为低时改变 DIO，CLK 为高时采样数据
3. **停止信号**：DIO 从低到高，CLK 保持高电平
4. **应答信号**：每传输一个字节后读取应答

### 显示编码

TM1637 使用 7 段数码管编码（a-g 段 + 小数点）：

```
   a
  ---
f|   |b
  -g-
e|   |c
  ---
   d   .dp
```

编码格式：`0bDP-G-F-E-D-C-B-A`

### 命令系统

- **数据命令** (0x40)：设置数据写入模式
- **显示控制** (0x80-0x8F)：控制显示开关和亮度
- **地址命令** (0xC0-0xC5)：设置显示地址

---

## 📚 API 接口

### 初始化函数

#### tm1637_init()

**功能**：初始化 TM1637 数码管

**函数原型**：
```c
void tm1637_init(void);
```

**说明**：
- 配置 GPIO 引脚为输出模式
- 初始化通信协议
- 开启显示
- 设置为固定地址模式

**使用示例**：
```c
tm1637_init();  // 初始化数码管
```

---

### 显示控制函数

#### tm1637_switch()

**功能**：控制数码管显示开关

**函数原型**：
```c
void tm1637_switch(bool bstate);
```

**参数**：
- `bstate` - 显示状态
  - `true`：开启显示
  - `false`：关闭显示

**使用示例**：
```c
tm1637_switch(true);   // 开启显示
tm1637_switch(false);  // 关闭显示
```

---

#### tm1637_disp_num_process()

**功能**：显示 0-9999 的数字（自动处理位数）

**函数原型**：
```c
void tm1637_disp_num_process(uint16_t u16Data);
```

**参数**：
- `u16Data` - 要显示的数值（0-9999）
  - 超过 9999 自动限制为 9999
  - 自动右对齐显示
  - 前导零不显示（空白）

**显示规则**：
- 1 位数：`   1` （右对齐）
- 2 位数：`  12`
- 3 位数：` 123`
- 4 位数：`1234`

**使用示例**：
```c
tm1637_disp_num_process(25);    // 显示 "  25"
tm1637_disp_num_process(1234);  // 显示 "1234"
tm1637_disp_num_process(10000); // 显示 "9999" (超出范围)
```

---

#### tm1637_tubedisplay()

**功能**：自定义显示内容（高级功能）

**函数原型**：
```c
void tm1637_tubedisplay(uint8_t *td);
```

**参数**：
- `td` - 4 字节数组，每个元素对应一位数码管的显示内容
  - 0-9：显示数字 0-9
  - 10-15：显示字母 A-F
  - 16-25：显示带小数点的数字 0.-9.
  - 26 (TUBE_DISPLAY_NULL)：不显示（空白）

**使用示例**：
```c
uint8_t display_data[4] = {1, 2, 3, 4};  // 显示 "1234"
tm1637_tubedisplay(display_data);

uint8_t temp_humi[4] = {2, 5, 6, 0};     // 显示 "2560" (温度25°C, 湿度60%)
tm1637_tubedisplay(temp_humi);
```

---

### 底层通信函数

以下函数为底层通信函数，一般不需要直接调用：

- `tm1637_wt_cmd(uint8_t cmd)` - 发送命令
- `tm1637_wt_data(uint8_t grid, uint8_t data)` - 写入数据到指定位置
- `tm1637_start()` - 发送起始信号
- `tm1637_stop()` - 发送停止信号
- `tm1637_wt_byte(uint8_t byte)` - 发送一个字节

---

## 💡 使用示例

### 示例1：基本数字显示

```c
#include "tm1637.h"

void app_main(void)
{
    // 初始化数码管
    tm1637_init();
    
    // 显示数字
    tm1637_disp_num_process(1234);
    
    // 等待 2 秒
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 关闭显示
    tm1637_switch(false);
}
```

### 示例2：计数器

```c
#include "tm1637.h"

void counter_task(void *pvParameters)
{
    tm1637_init();
    
    uint16_t count = 0;
    
    while (1) {
        // 显示当前计数值
        tm1637_disp_num_process(count);
        
        // 计数递增
        count++;
        if (count > 9999) {
            count = 0;  // 循环计数
        }
        
        // 延时 100ms
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

### 示例3：温湿度显示

```c
#include "tm1637.h"
#include "dht11.h"

void display_temp_humi_task(void *pvParameters)
{
    tm1637_init();
    
    while (1) {
        // 读取 DHT11 温湿度
        int16_t temp, humi;
        esp_err_t ret = dht11_read_data(DHT11_DATA_GPIO, &humi, &temp);
        
        if (ret == ESP_OK) {
            // 转换为整数（去掉小数部分）
            uint8_t temp_int = temp / 10;  // 温度
            uint8_t humi_int = humi / 10;  // 湿度
            
            // 组合显示：TTHH（前2位温度，后2位湿度）
            uint16_t display_value = temp_int * 100 + humi_int;
            tm1637_disp_num_process(display_value);
            
            // 例如：温度25°C，湿度60% -> 显示 "2560"
        }
        
        // 每 2 秒更新一次
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
```

### 示例4：距离显示

```c
#include "tm1637.h"
#include "hc_sr04.h"

void display_distance_task(void *pvParameters)
{
    tm1637_init();
    
    while (1) {
        // 读取超声波距离
        float distance = hc_sr04_task();
        
        if (distance > 0) {
            // 转换为整数并显示
            uint16_t dist_cm = (uint16_t)distance;
            tm1637_disp_num_process(dist_cm);
        } else {
            // 测量失败，显示 "----"
            tm1637_disp_num_process(9999);
        }
        
        // 每 200ms 更新一次
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
```

### 示例5：自定义显示（带小数点）

```c
#include "tm1637.h"

void custom_display_example(void)
{
    tm1637_init();
    
    // 显示 "12.34"（带小数点）
    uint8_t display_data[4];
    display_data[0] = 1;      // 第1位：1
    display_data[1] = 16;     // 第2位：2.（带小数点，编码16）
    display_data[2] = 3;      // 第3位：3
    display_data[3] = 4;      // 第4位：4
    
    tm1637_tubedisplay(display_data);
}
```

---

## ⚠️ 注意事项

### 硬件限制

1. **GPIO34 限制**：
   - GPIO34 只能作为输入
   - TM1637 使用开漏输出，兼容此限制
   - 需要外部上拉电阻

2. **上拉电阻**：
   - CLK 和 DIO 都需要上拉电阻
   - 推荐值：4.7kΩ - 10kΩ
   - 大多数 TM1637 模块已集成上拉电阻

3. **供电电压**：
   - 支持 3.3V 和 5V 供电
   - 逻辑电平兼容 3.3V

### 使用注意事项

1. **显示范围**：
   - `tm1637_disp_num_process()` 只能显示 0-9999
   - 超出范围自动限制为 9999

2. **显示格式**：
   - 数字右对齐显示
   - 前导零不显示（空白）
   - 如需前导零，使用 `tm1637_tubedisplay()`

3. **刷新频率**：
   - 建议刷新间隔 > 50ms
   - 过高频率可能导致闪烁

4. **初始化顺序**：
   - 必须先调用 `tm1637_init()` 再使用其他函数
   - 初始化后默认开启显示

### 常见问题

**Q1: 数码管不显示？**
- 检查供电是否正常（3.3V 或 5V）
- 检查接线是否正确
- 确认已调用 `tm1637_init()`
- 检查是否调用了 `tm1637_switch(false)` 关闭显示

**Q2: 显示内容闪烁？**
- 降低刷新频率（增加延时）
- 检查供电是否稳定
- 检查上拉电阻是否正常

**Q3: 如何显示小数点？**
- 使用 `tm1637_tubedisplay()` 函数
- 数字编码 16-25 对应带小数点的 0.-9.
- 参考示例5

**Q4: 如何调节亮度？**
- 当前驱动使用默认亮度
- 如需调节，修改 `DISP_ON` 命令的低3位（0x88-0x8F）

---

## 📖 参考资料

### 数据手册
- [TM1637 数据手册](https://www.titanmec.com/index.php/en/project/download/id/302.html)

### 相关文档
- `pin_definitions.h` - 引脚定义
- `docs/GPIO_PIN_ALLOCATION.md` - GPIO分配文档

### 相关组件
- `dht11` - 温湿度传感器（可用于温湿度显示）
- `hc-sr04` - 超声波传感器（可用于距离显示）
- `display_task` - 显示任务（集成了轮播显示功能）

### 显示编码表

| 字符 | 编码 | 字符 | 编码 |
|------|------|------|------|
| 0 | 0x3F | 0. | 0xBF |
| 1 | 0x06 | 1. | 0x86 |
| 2 | 0x5B | 2. | 0xDB |
| 3 | 0x4F | 3. | 0xCF |
| 4 | 0x66 | 4. | 0xE6 |
| 5 | 0x6D | 5. | 0xED |
| 6 | 0x7D | 6. | 0xFD |
| 7 | 0x07 | 7. | 0x87 |
| 8 | 0x7F | 8. | 0xFF |
| 9 | 0x6F | 9. | 0xEF |
| A | 0x77 | 空白 | 0x00 |
| b | 0x7C | - | - |
| C | 0x39 | - | - |
| d | 0x5E | - | - |
| E | 0x79 | - | - |
| F | 0x71 | - | - |

---

**最后更新**: 2024-12-XX  
**维护者**: 项目团队
