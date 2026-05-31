# GPIO引脚分配快速参考

## 一图看懂所有引脚

```
┌─────────────────────────────────────────────────────────────┐
│                    ESP32-S3 巡线小车引脚分配                  │
└─────────────────────────────────────────────────────────────┘

【电机PWM控制 - 8个GPIO】
  GPIO2  ─→ 电机1正向    GPIO3  ─→ 电机1反向
  GPIO4  ─→ 电机2正向    GPIO5  ─→ 电机2反向
  GPIO6  ─→ 电机3正向    GPIO7  ─→ 电机3反向
  GPIO8  ─→ 电机4正向    GPIO9  ─→ 电机4反向

【编码器PCNT - 8个GPIO】
  GPIO41 ─→ 编码器1-A    GPIO42 ─→ 编码器1-B
  GPIO45 ─→ 编码器2-A    GPIO46 ─→ 编码器2-B
  GPIO14 ─→ 编码器3-A    GPIO15 ─→ 编码器3-B
  GPIO16 ─→ 编码器4-A    GPIO17 ─→ 编码器4-B

【串口通信 - 5个GPIO】
  GPIO43 ─→ UART0_TX (日志/RS485)
  GPIO44 ─→ UART0_RX (日志/RS485)
  GPIO19 ─→ RS485方向控制
  GPIO35 ─→ UART1_TX (语音模块)
  GPIO36 ─→ UART1_RX (语音模块)

【I2C总线 - 2个GPIO】
  GPIO21 ─→ I2C_SCL (PCF8574扩展芯片)
  GPIO20 ─→ I2C_SDA (PCF8574扩展芯片)

【传感器接口 - 10个GPIO】
  GPIO47 ─→ 左灰度传感器 (ADC1_CH6) [SSA4]
  GPIO48 ─→ 右灰度传感器 (ADC1_CH7) [SSD4]
  GPIO34 ─→ TM1637数码管CLK [SSA3]
  GPIO37 ─→ TM1637数码管DIO [SSA2]
  GPIO33 ─→ DHT11温湿度 [SSD3]
  GPIO38 ─→ 红外避障传感器 [SSA1]
  GPIO39 ─→ 交通灯信号1 [SSD6]
  GPIO40 ─→ 交通灯信号2 [SSA6]
  PCF_P5 ─→ 扩展IO [SSD5]
  PCF_P6 ─→ 扩展IO [SSA5]

【系统保留 - 不可使用】
  GPIO0, GPIO6-13 (SPI Flash)
```

---

## 按功能分类

### 🚗 运动控制（16个GPIO）

| 功能 | GPIO | 说明 |
|------|------|------|
| **电机PWM** | 2,3,4,5,6,7,8,9 | 4个电机双PWM控制 |
| **编码器** | 14,15,16,17,41,42,45,46 | 4个编码器AB相 |

### 📡 通信接口（7个GPIO）

| 功能 | GPIO | 说明 |
|------|------|------|
| **UART0** | 43(TX), 44(RX) | 日志和RS485 |
| **UART1** | 35(TX), 36(RX) | 外接模块 |
| **RS485** | 19 | 方向控制 |
| **I2C** | 20(SDA), 21(SCL) | PCF8574扩展 |

### 🔍 传感器（8个GPIO + 2个PCF）

| 功能 | GPIO | 接口标识 | 说明 |
|------|------|---------|------|
| **灰度传感器** | 47, 48 | SSA4, SSD4 | ADC1通道 |
| **数码管** | 34, 37 | SSA3, SSA2 | TM1637 |
| **温湿度** | 33 | SSD3 | DHT11 |
| **红外避障** | 38 | SSA1 | 数字输入 |
| **交通灯** | 39, 40 | SSD6, SSA6 | 数字输出 |
| **扩展IO** | PCF_P5, P6 | SSD5, SSA5 | PCF8574 |

---

## 代码配置模板

### 方式1：直接定义

```c
// 灰度传感器
#define GRAY_LEFT    47
#define GRAY_RIGHT   48

// 数码管
#define TM1637_CLK   34
#define TM1637_DIO   33

// 测距
#define VL53_SCL     40
#define VL53_SDA     39

// 温湿度
#define DHT11_PIN    38

// 触摸
#define TOUCH_PIN    37

// 电机PWM
#define MOTOR1_FWD   2
#define MOTOR1_REV   3
// ... 其他电机

// 编码器
#define ENC1_A       41
#define ENC1_B       42
// ... 其他编码器

// 串口
#define UART0_TX     43
#define UART0_RX     44
#define RS485_DIR    19

// I2C
#define I2C_SCL      21
#define I2C_SDA      20
```

### 方式2：使用GPIO_NUM枚举

```c
#define GRAY_LEFT_GPIO    GPIO_NUM_47
#define GRAY_RIGHT_GPIO   GPIO_NUM_48
// ... 其他类似
```

---

## 重要提醒

### ⚠️ 必须注意

1. **GPIO34-39只能输入**，不能输出
2. **ADC2与WiFi冲突**，使用WiFi时只能用ADC1
3. **GPIO6-11连接Flash**，不能使用
4. **GPIO19已用于RS485**，不是灰度传感器

### ✅ 推荐配置

- **灰度传感器**: GPIO47, 48 (ADC1通道)
- **I2C总线**: GPIO20, 21 (PCF8574)
- **编码器**: 使用PCNT硬件计数
- **电机**: 使用LEDC硬件PWM

### 📌 引脚特性

| GPIO范围 | 特性 | 用途 |
|---------|------|------|
| 34-39 | 只能输入 | ADC、按键、传感器 |
| 0 | 启动模式 | 避免使用 |
| 6-11 | Flash | 禁止使用 |
| 其他 | 通用IO | 可自由配置 |

---

## 快速检查清单

在开始编码前，确认：

- [ ] 灰度传感器使用GPIO47/48（ADC1）
- [ ] RS485方向控制使用GPIO19
- [ ] I2C使用GPIO20/21（PCF8574）
- [ ] 电机PWM使用GPIO2-9
- [ ] 编码器使用GPIO14-17, 41-42, 45-46
- [ ] UART0使用GPIO43/44
- [ ] 没有使用GPIO6-11（Flash专用）
- [ ] 只读GPIO（34-39）没有配置为输出

---

## 常见问题

**Q: 为什么灰度传感器不用GPIO19/20？**  
A: GPIO19用于RS485方向控制，GPIO20用于I2C（PCF8574）。

**Q: 可以使用ADC2吗？**  
A: 不推荐。ADC2与WiFi冲突，如果需要WiFi功能，必须使用ADC1。

**Q: PCF8574扩展IO怎么用？**  
A: 通过I2C（GPIO20/21）控制，适合低速数字IO，不适合PWM和ADC。

**Q: 编码器为什么用这些引脚？**  
A: 这些引脚连接到PCNT硬件单元，可以实现硬件计数，不占用CPU。

---

## 相关文档

- 详细文档: `docs/GPIO_PIN_ALLOCATION.md`
- GPIO管理器: `components/gpio_manager/README.md`
- 设计文档: `.kiro/specs/stm32-to-esp32-migration/design.md`
