# ESP32巡线小车 GPIO引脚分配表

## 文档说明

本文档详细记录了ESP32巡线小车项目的所有GPIO引脚分配情况，包括固定引脚和可自由分配的传感器引脚。本文档是硬件连接的权威参考，所有组件的引脚使用必须与此文档保持一致。

**更新日期**: 2024-12-20  
**硬件版本**: ESP32-S3  
**项目**: STM32到ESP32巡线小车移植  
**文档版本**: 1.2

### 文档用途

- 硬件工程师：查阅引脚分配，正确连接外设
- 软件工程师：了解GPIO使用情况，避免引脚冲突
- 系统集成：验证GPIO资源分配，规划扩展功能
- 故障排查：定位硬件连接问题，检查引脚配置

---

## 引脚分配总览

### 引脚分类说明

本项目的GPIO引脚分为三类：

1. **固定功能引脚**：核心硬件功能，硬件设计中已固定，不可更改
2. **可配置引脚**：传感器扩展接口，可根据实际需求分配不同传感器
3. **系统保留引脚**：ESP32系统使用，不可用于应用程序

### 固定功能引脚（不可更改）

这些引脚用于核心硬件功能，已在硬件设计中固定，不能用于其他用途。

| GPIO | 功能 | 模块 | 说明 | 特性 |
|------|------|------|------|------|
| **串口通信（5个GPIO）** |
| GPIO43 | UART0_TX | 串口0 | 日志打印和RS485通信发送 | 固定，与舵机/摄像头共用 |
| GPIO44 | UART0_RX | 串口0 | 日志打印和RS485通信接收 | 固定，与舵机/摄像头共用 |
| GPIO19 | 485DIR | RS485 | RS485方向控制（发送/接收切换） | 固定，控制RS485收发器 |
| GPIO35 | UART1_TX | 串口1 | 外接串口模块发送（语音等） | 固定，仅输入GPIO |
| GPIO36 | UART1_RX | 串口1 | 外接串口模块接收（语音等） | 固定，仅输入GPIO |
| **I2C总线（2个GPIO）** |
| GPIO21 | I2C_SCL | PCF8574 | I2C时钟线（扩展芯片） | 固定，需外部上拉 |
| GPIO20 | I2C_SDA | PCF8574 | I2C数据线（扩展芯片） | 固定，需外部上拉，⚠️被灰度传感器占用 |
| **电机PWM控制（8个GPIO）** |
| GPIO2 | PWM1 | 电机1 | 电机1正向（左前轮） | 固定，LEDC_CH0 |
| GPIO3 | PWM2 | 电机1 | 电机1反向（左前轮） | 固定，LEDC_CH1 |
| GPIO4 | PWM3 | 电机2 | 电机2正向（右前轮） | 固定，LEDC_CH2 |
| GPIO5 | PWM4 | 电机2 | 电机2反向（右前轮） | 固定，LEDC_CH3 |
| GPIO6 | PWM5 | 电机3 | 电机3正向（左后轮） | 固定，LEDC_CH4 |
| GPIO7 | PWM6 | 电机3 | 电机3反向（左后轮） | 固定，LEDC_CH5 |
| GPIO8 | PWM7 | 电机4 | 电机4正向（右后轮） | 固定，LEDC_CH6 |
| GPIO9 | PWM8 | 电机4 | 电机4反向（右后轮） | 固定，LEDC_CH7 |
| **编码器PCNT（8个GPIO）** |
| GPIO41 | A1 | 编码器1 | 编码器1 A相（左前轮） | 固定，PCNT单元0 |
| GPIO42 | B1 | 编码器1 | 编码器1 B相（左前轮） | 固定，PCNT单元0 |
| GPIO45 | A2 | 编码器2 | 编码器2 A相（右前轮） | 固定，PCNT单元1 |
| GPIO46 | B2 | 编码器2 | 编码器2 B相（右前轮） | 固定，PCNT单元1 |
| GPIO14 | A3 | 编码器3 | 编码器3 A相（左后轮） | 固定，PCNT单元2 |
| GPIO15 | B3 | 编码器3 | 编码器3 B相（左后轮） | 固定，PCNT单元2 |
| GPIO16 | A4 | 编码器4 | 编码器4 A相（右后轮） | 固定，PCNT单元3 |
| GPIO17 | B4 | 编码器4 | 编码器4 B相（右后轮） | 固定，PCNT单元3 |

### 可自由分配的传感器引脚

以下引脚可根据实际传感器连接情况自由分配，但需注意引脚特性限制。

| GPIO | 标识 | 当前分配 | 可选功能 | 特性限制 |
|------|------|---------|---------|---------|
| GPIO18 | - | 灰度传感器左（临时飞线） | 数字传感器/ADC输入 | ADC2_CH7，与WiFi冲突 |
| GPIO20 | - | 灰度传感器右（临时飞线） | 数字传感器/ADC输入 | ADC2_CH9，与WiFi冲突，⚠️占用I2C_SDA |
| GPIO33 | SSD3 | DHT11温湿度传感器 | 数字传感器/单总线 | 可重新分配 |
| GPIO34 | SSA3 | TM1637数码管CLK | 数字输入/ADC输入 | 仅输入，ADC1_CH6 |
| GPIO37 | SSA2 | TM1637数码管DIO | 数字输入/ADC输入 | 仅输入，ADC1_CH1 |
| GPIO38 | SSA1 | 红外避障传感器 | 数字输入/ADC输入 | 仅输入，ADC1_CH2 |
| GPIO39 | SSD6 | 交通灯信号1 | 数字输入/ADC输入 | 仅输入，ADC1_CH3 |
| GPIO40 | SSA6 | 交通灯信号2 | 数字传感器 | 可重新分配 |
| GPIO47 | SSA4 | 未分配（推荐灰度左） | 数字传感器/ADC输入 | ADC1_CH6，与WiFi不冲突 |
| GPIO48 | SSD4 | 未分配（推荐灰度右） | 数字传感器/ADC输入 | ADC1_CH7，与WiFi不冲突 |
| - | SSA5 | PCF P6扩展IO | 通过PCF8574扩展 | I2C扩展引脚 |
| - | SSD5 | PCF P5扩展IO | 通过PCF8574扩展 | I2C扩展引脚 |

**接口标识说明**：
- **SSA1-SSA6**: 传感器扩展接口A组（Sensor Socket A）
- **SSD3-SSD6**: 传感器扩展接口D组（Sensor Socket D）
- **SSA5/SSD5**: 通过PCF8574 I2C扩展芯片提供（P6/P5引脚）

---

## 当前项目使用情况

### 已使用的GPIO（共32个）

#### 1. 电机控制系统（8个GPIO）
- GPIO2-9: 4个电机的PWM控制（每个电机2个PWM通道）

#### 2. 编码器系统（8个GPIO）
- GPIO41, 42: 编码器1 (A1, B1)
- GPIO45, 46: 编码器2 (A2, B2)
- GPIO14, 15: 编码器3 (A3, B3)
- GPIO16, 17: 编码器4 (A4, B4)

#### 3. 串口通信（5个GPIO）
- GPIO43, 44: UART0 (TX, RX) - 日志和RS485
- GPIO19: RS485方向控制
- GPIO35, 36: UART1 (TX, RX) - 外接模块

#### 4. I2C总线（2个GPIO）
- GPIO20: I2C_SDA (PCF8574扩展芯片)
- GPIO21: I2C_SCL (PCF8574扩展芯片)

#### 5. 传感器和显示（6个GPIO）
- GPIO33: DHT11温湿度传感器 (SSD3)
- GPIO34: TM1637数码管CLK (SSA3)
- GPIO37: TM1637数码管DIO (SSA2)
- GPIO38: 红外避障传感器 (SSA1)
- GPIO39: 交通灯信号1 (SSD6)
- GPIO40: 交通灯信号2 (SSA6)

#### 6. 预留引脚（3个GPIO）
- GPIO47: SSA4 - 未分配
- GPIO48: SSD4 - 未分配
- PCF P5, P6: 通过PCF8574扩展的IO

### 未使用的GPIO

以下GPIO在当前项目中未使用，可用于扩展功能：

| GPIO | 特性 | 推荐用途 |
|------|------|---------|
| GPIO0 | 启动模式选择 | 保留（系统用） |
| GPIO1 | UART0_TX备用 | 可用于其他功能 |
| GPIO10 | SPI Flash | 保留（系统用） |
| GPIO11 | SPI Flash | 保留（系统用） |
| GPIO12 | SPI Flash | 保留（系统用） |
| GPIO13 | SPI Flash | 保留（系统用） |
| GPIO18 | 通用IO | 可自由使用 |

---

## 功能模块GPIO分组

### 巡线控制核心模块

#### 灰度传感器（需要ADC）
**当前未分配，建议使用：**
- 左灰度传感器: GPIO47 (SSA4) - ADC1_CH6
- 右灰度传感器: GPIO48 (SSD4) - ADC1_CH7

**注意**: 
- ADC2与WiFi冲突，如需使用WiFi，必须使用ADC1通道
- GPIO34-39只能作为输入，支持ADC

#### 数码管显示（TM1637）
- CLK: GPIO34 (SSA3)
- DIO: GPIO37 (SSA2)

#### 温湿度传感器（DHT11）
- DATA: GPIO33 (SSD3)

#### 红外避障传感器
- INPUT: GPIO38 (SSA1)

#### 交通灯控制
- 信号1: GPIO39 (SSD6)
- 信号2: GPIO40 (SSA6)

---

## GPIO特性说明

### 输入专用GPIO（只能输入，不能输出）
- GPIO34, GPIO35, GPIO36, GPIO37, GPIO38, GPIO39

这些GPIO只能配置为输入模式，适合用于：
- ADC采样（灰度传感器、光照传感器等）
- 数字输入（按键、触摸传感器等）
- 不能用于PWM输出或数字输出

### ADC通道分配

#### ADC1（推荐使用，与WiFi不冲突）
| GPIO | ADC通道 | 当前用途 |
|------|---------|---------|
| GPIO36 | ADC1_CH0 | UART1_RX（冲突） |
| GPIO37 | ADC1_CH1 | 触摸传感器 |
| GPIO38 | ADC1_CH2 | DHT11（数字信号） |
| GPIO34 | ADC1_CH6 | TM1637_SCL（数字信号） |
| GPIO47 | ADC1_CH6 | **可用于灰度传感器** |
| GPIO48 | ADC1_CH7 | **可用于灰度传感器** |

#### ADC2（与WiFi冲突，不推荐）
| GPIO | ADC通道 | 说明 |
|------|---------|------|
| GPIO0-10 | ADC2_CH0-9 | 启用WiFi时不可用 |

---

## GPIO冲突检测机制

项目使用GPIO管理器（gpio_manager）进行引脚分配和冲突检测，确保同一GPIO不会被多个模块同时使用。

### GPIO管理器工作原理

1. **注册机制**：每个组件在初始化时调用`gpio_manager_register()`注册使用的GPIO
2. **冲突检测**：如果GPIO已被其他组件注册，返回错误并阻止初始化
3. **分配表**：维护全局GPIO分配表，记录每个GPIO的使用者和功能
4. **调试支持**：提供`gpio_manager_print_allocation_table()`打印当前分配情况

### 使用GPIO管理器示例

```c
#include "gpio_manager.h"

// 在组件初始化时注册GPIO
esp_err_t my_component_init(void) {
    // 注册GPIO18用于灰度传感器
    esp_err_t ret = gpio_manager_register(
        GPIO_NUM_18,           // GPIO编号
        GPIO_FUNC_ADC,         // 功能类型
        "gray_sensor",         // 组件名称
        "左灰度传感器"         // 描述
    );
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO18已被其他组件占用");
        return ESP_FAIL;
    }
    
    // 继续初始化...
    return ESP_OK;
}
```

### 查看GPIO分配表

在main.c中调用以下函数可打印当前GPIO分配情况：

```c
gpio_manager_print_allocation_table();
```

输出示例：
```
GPIO分配表：
GPIO  2: [PWM      ] motor - 电机1正向
GPIO  3: [PWM      ] motor - 电机1反向
GPIO 18: [ADC      ] gray_sensor - 左灰度传感器
GPIO 19: [GPIO_OUT ] rs485 - RS485方向控制
...
```

---

## PCF8574扩展IO说明

PCF8574是I2C接口的8位IO扩展芯片，通过GPIO20/21（I2C）控制。

### PCF8574引脚分配

| PCF引脚 | 标识 | 功能 | 说明 |
|---------|------|------|------|
| P0 | - | 未分配 | 可自由使用 |
| P1 | - | 未分配 | 可自由使用 |
| P2 | - | 未分配 | 可自由使用 |
| P3 | - | 未分配 | 可自由使用 |
| P4 | - | 未分配 | 可自由使用 |
| P5 | SSD5 | 传感器扩展 | 可自由分配 |
| P6 | SSA5 | 传感器扩展 | 可自由分配 |
| P7 | - | 未分配 | 可自由使用 |

### PCF8574使用建议

适合用于：
- 数字输出（LED、继电器等）
- 数字输入（按键、开关等）
- 低速信号控制

不适合用于：
- PWM输出（频率受限）
- 高速信号采集
- ADC采样

---

## 引脚分配建议

### 巡线小车核心功能

根据设计文档和实际硬件，推荐以下配置：

```c
// 灰度传感器（ADC采样）
#define GRAY_SENSOR_LEFT_GPIO    GPIO_NUM_47  // ADC1_CH6
#define GRAY_SENSOR_RIGHT_GPIO   GPIO_NUM_48  // ADC1_CH7

// 数码管显示（TM1637）
#define TM1637_CLK_GPIO          GPIO_NUM_34  // SSA3
#define TM1637_DIO_GPIO          GPIO_NUM_37  // SSA2

// 温湿度传感器（DHT11）
#define DHT11_DATA_GPIO          GPIO_NUM_33  // SSD3

// 红外避障传感器
#define IR_OBSTACLE_GPIO         GPIO_NUM_38  // SSA1

// 交通灯控制
#define TRAFFIC_LIGHT_SIGNAL1_GPIO   GPIO_NUM_39  // SSD6
#define TRAFFIC_LIGHT_SIGNAL2_GPIO   GPIO_NUM_40  // SSA6

// 电机PWM（固定）
#define MOTOR1_FWD_GPIO          GPIO_NUM_2
#define MOTOR1_REV_GPIO          GPIO_NUM_3
#define MOTOR2_FWD_GPIO          GPIO_NUM_4
#define MOTOR2_REV_GPIO          GPIO_NUM_5
#define MOTOR3_FWD_GPIO          GPIO_NUM_6
#define MOTOR3_REV_GPIO          GPIO_NUM_7
#define MOTOR4_FWD_GPIO          GPIO_NUM_8
#define MOTOR4_REV_GPIO          GPIO_NUM_9

// 编码器（固定）
#define ENCODER1_A_GPIO          GPIO_NUM_41
#define ENCODER1_B_GPIO          GPIO_NUM_42
#define ENCODER2_A_GPIO          GPIO_NUM_45
#define ENCODER2_B_GPIO          GPIO_NUM_46
#define ENCODER3_A_GPIO          GPIO_NUM_14
#define ENCODER3_B_GPIO          GPIO_NUM_15
#define ENCODER4_A_GPIO          GPIO_NUM_16
#define ENCODER4_B_GPIO          GPIO_NUM_17

// 串口通信（固定）
#define UART0_TX_GPIO            GPIO_NUM_43
#define UART0_RX_GPIO            GPIO_NUM_44
#define RS485_DIR_GPIO           GPIO_NUM_19
#define UART1_TX_GPIO            GPIO_NUM_35
#define UART1_RX_GPIO            GPIO_NUM_36

// I2C总线（固定）
#define I2C_SCL_GPIO             GPIO_NUM_21
#define I2C_SDA_GPIO             GPIO_NUM_20
```

---

## 更新GPIO管理器配置

根据实际硬件配置，需要更新`gpio_manager_integration.c`：

```c
esp_err_t gpio_manager_register_all_pins(void) {
    esp_err_t ret;

    // ========== 灰度传感器 (ADC1) ==========
    ret = gpio_manager_register(GPIO_NUM_47, GPIO_FUNC_ADC, 
                                "gray_sensor", "左灰度传感器 (ADC1_CH6)");
    if (ret != ESP_OK) return ret;

    ret = gpio_manager_register(GPIO_NUM_48, GPIO_FUNC_ADC, 
                                "gray_sensor", "右灰度传感器 (ADC1_CH7)");
    if (ret != ESP_OK) return ret;

    // ========== 电机PWM (LEDC) ==========
    ret = gpio_manager_register(GPIO_NUM_2, GPIO_FUNC_PWM, 
                                "motor", "电机1正向");
    if (ret != ESP_OK) return ret;
    // ... (GPIO3-9 类似)

    // ========== 编码器 (PCNT) ==========
    ret = gpio_manager_register(GPIO_NUM_41, GPIO_FUNC_GPIO_IN, 
                                "encoder", "编码器1 A相");
    if (ret != ESP_OK) return ret;
    // ... (其他编码器引脚类似)

    // ========== 串口通信 ==========
    ret = gpio_manager_register(GPIO_NUM_43, GPIO_FUNC_UART_TX, 
                                "uart0", "UART0发送");
    if (ret != ESP_OK) return ret;

    ret = gpio_manager_register(GPIO_NUM_44, GPIO_FUNC_UART_RX, 
                                "uart0", "UART0接收");
    if (ret != ESP_OK) return ret;

    ret = gpio_manager_register(GPIO_NUM_19, GPIO_FUNC_GPIO_OUT, 
                                "rs485", "RS485方向控制");
    if (ret != ESP_OK) return ret;

    // ... 其他引脚
    
    return ESP_OK;
}
```

---

## 注意事项

### 1. GPIO使用限制

- **GPIO0**: 启动时需要为高电平，不建议用于关键功能
- **GPIO34-39**: 只能输入，不能输出
- **GPIO6-11**: 连接到SPI Flash，不能使用
- **ADC2**: 与WiFi冲突，启用WiFi时不可用

### 2. 电源和地线

确保所有传感器和模块共地（GND），电源电压匹配（3.3V或5V）。

### 3. 上拉/下拉电阻

- I2C总线需要外部上拉电阻（通常4.7kΩ）
- 编码器信号建议使用内部上拉
- 按键输入建议使用内部上拉或下拉

### 4. 信号完整性

- 编码器信号线尽量短，避免干扰
- PWM信号与ADC采样线分开布线
- 高速信号（编码器、SPI）使用屏蔽线

---

## 版本历史

| 版本 | 日期 | 修改内容 | 修改人 |
|------|------|---------|--------|
| 1.0 | 2024-12-XX | 初始版本，整理所有GPIO分配 | - |

---

## GPIO冲突检测机制

项目使用GPIO管理器（gpio_manager）进行引脚分配和冲突检测，确保同一GPIO不会被多个模块同时使用。

### GPIO管理器工作原理

1. **注册机制**：每个组件在初始化时调用`gpio_manager_register()`注册使用的GPIO
2. **冲突检测**：如果GPIO已被其他组件注册，返回错误并阻止初始化
3. **分配表**：维护全局GPIO分配表，记录每个GPIO的使用者和功能
4. **调试支持**：提供`gpio_manager_print_allocation_table()`打印当前分配情况

### GPIO管理器架构

```
应用层组件 (gray_sensor, pwm, uart, etc.)
    ↓ 调用 gpio_manager_register()
GPIO管理器 (gpio_manager)
    ↓ 检查冲突
GPIO分配表 (全局数组)
    ↓ 记录使用情况
硬件GPIO (ESP32-S3)
```

### 冲突检测流程

```
组件初始化
    ↓
调用 gpio_manager_register(GPIO_NUM, FUNC, NAME, DESC)
    ↓
检查 GPIO 是否已被占用？
    ├─ 是 → 返回 ESP_FAIL，记录错误日志
    └─ 否 → 记录到分配表，返回 ESP_OK
         ↓
组件继续初始化
```

### 使用GPIO管理器示例

```c
#include "gpio_manager.h"

// 在组件初始化时注册GPIO
esp_err_t my_component_init(void) {
    // 注册GPIO18用于灰度传感器
    esp_err_t ret = gpio_manager_register(
        GPIO_NUM_18,           // GPIO编号
        GPIO_FUNC_ADC,         // 功能类型
        "gray_sensor",         // 组件名称
        "左灰度传感器"         // 描述
    );
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO18已被其他组件占用");
        return ESP_FAIL;
    }
    
    // 继续初始化...
    return ESP_OK;
}
```

### 查看GPIO分配表

在main.c中调用以下函数可打印当前GPIO分配情况：

```c
gpio_manager_print_allocation_table();
```

输出示例：
```
GPIO分配表：
GPIO  2: [PWM      ] motor - 电机1正向
GPIO  3: [PWM      ] motor - 电机1反向
GPIO 18: [ADC      ] gray_sensor - 左灰度传感器
GPIO 19: [GPIO_OUT ] rs485 - RS485方向控制
...
```

### 功能类型说明

| 功能类型 | 说明 | 示例 |
|---------|------|------|
| GPIO_FUNC_GPIO_IN | 数字输入 | 按键、红外避障 |
| GPIO_FUNC_GPIO_OUT | 数字输出 | LED、RS485方向控制 |
| GPIO_FUNC_PWM | PWM输出 | 电机控制、舵机 |
| GPIO_FUNC_ADC | ADC输入 | 灰度传感器、温度传感器 |
| GPIO_FUNC_UART_TX | UART发送 | 串口通信 |
| GPIO_FUNC_UART_RX | UART接收 | 串口通信 |
| GPIO_FUNC_I2C_SCL | I2C时钟 | I2C总线 |
| GPIO_FUNC_I2C_SDA | I2C数据 | I2C总线 |
| GPIO_FUNC_PCNT | 脉冲计数 | 编码器 |

---

## 接线建议和注意事项

### 电源和地线

#### 共地原则

⚠️ **所有模块必须共地**：ESP32、传感器、电机驱动、电源模块的GND必须连接在一起。

**原因**：
- 数字信号的高低电平是相对于GND的电压差
- 不共地会导致信号电平不稳定，产生误触发
- 可能损坏芯片（电压差过大）

**正确接线**：
```
ESP32 GND ─┬─ 传感器 GND
           ├─ 电机驱动 GND
           ├─ 电源模块 GND
           └─ 电池 GND (-)
```

#### 电源电压匹配

| 模块 | 工作电压 | 说明 |
|------|---------|------|
| ESP32-S3 | 3.3V | 核心芯片，GPIO电平3.3V |
| 灰度传感器 | 3.3V-5V | 输出电压应≤3.3V |
| 数码管(TM1637) | 3.3V-5V | 可直接连接ESP32 |
| DHT11 | 3.3V-5V | 可直接连接ESP32 |
| 红外避障 | 3.3V-5V | 输出电压应≤3.3V |
| 电机驱动 | 5V-12V | 独立供电，不能直接连接ESP32 |
| 舵机 | 5V-6V | 独立供电，信号线可连接ESP32 |

⚠️ **注意**：
- ESP32 GPIO输入电压不能超过3.3V，否则会损坏芯片
- 5V传感器需要使用电平转换电路或分压电阻
- 电机和舵机需要独立供电，不能从ESP32取电

### 上拉/下拉电阻

#### I2C总线上拉电阻

I2C总线（SCL和SDA）需要外部上拉电阻：

```
3.3V ─┬─ 4.7kΩ ─┬─ SCL (GPIO21)
      │         └─ I2C设备SCL
      └─ 4.7kΩ ─┬─ SDA (GPIO20)
                └─ I2C设备SDA
```

**推荐值**：4.7kΩ（标准值），可根据总线长度和设备数量调整（2.2kΩ-10kΩ）

#### 编码器信号上拉

编码器信号建议使用内部上拉电阻（ESP32内置）：

```c
gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << ENCODER1_A_GPIO),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,  // 启用内部上拉
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
};
gpio_config(&io_conf);
```

#### 按键输入下拉

按键输入建议使用内部下拉电阻：

```c
gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << KEY_GPIO),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,  // 启用内部下拉
};
gpio_config(&io_conf);
```

### 信号完整性

#### 编码器信号线

编码器信号频率较高，容易受干扰：

- ✅ 使用屏蔽双绞线
- ✅ 信号线尽量短（<30cm）
- ✅ 远离电机和PWM信号线
- ✅ 使用内部上拉电阻
- ❌ 避免与电源线并行走线

#### PWM信号线

PWM信号会产生电磁干扰：

- ✅ 与ADC采样线分开布线
- ✅ 使用屏蔽线或双绞线
- ✅ 在电机驱动板附近加滤波电容
- ❌ 避免与传感器信号线并行走线

#### ADC采样线

ADC采样对噪声敏感：

- ✅ 使用屏蔽线
- ✅ 远离PWM信号线和电机
- ✅ 在传感器输出端加滤波电容（0.1uF）
- ✅ 使用差分信号（如果可能）
- ❌ 避免长距离走线

### 防护措施

#### ESD防护

静电放电（ESD）可能损坏ESP32：

- ✅ 在外部接口处添加TVS二极管
- ✅ 使用金属外壳并接地
- ✅ 在GPIO输入端添加串联电阻（100Ω-1kΩ）

#### 过压保护

防止外部电压过高损坏ESP32：

- ✅ 使用稳压二极管（3.3V）
- ✅ 使用电压钳位电路
- ✅ 在输入端添加限流电阻

#### 反向保护

防止电源反接：

- ✅ 使用二极管或MOSFET反向保护电路
- ✅ 使用防反接插座

---

## 故障排除

### 常见硬件问题

#### 问题1：GPIO冲突错误

**现象**：
```
E (1234) gpio_manager: GPIO18已被占用: gray_sensor (左灰度传感器)
E (1235) my_component: GPIO18注册失败
```

**原因**：
- 多个组件尝试使用同一个GPIO
- 组件初始化顺序错误
- 引脚定义配置错误

**解决方案**：
1. 检查`pin_definitions.h`中的引脚定义，确保没有重复
2. 调用`gpio_manager_print_allocation_table()`查看当前分配情况
3. 修改组件配置，使用不同的GPIO
4. 检查组件初始化顺序，确保GPIO管理器最先初始化

#### 问题2：ADC读取值异常

**现象**：
- ADC值始终为4095（最大值）
- ADC值始终为0（最小值）
- ADC值跳变剧烈

**原因**：
- 传感器未正确连接或电源未接通
- GPIO引脚配置错误
- 传感器输出电压超出范围
- 信号线受干扰

**解决方案**：
1. 使用万用表测量传感器输出电压（应在0-3.3V范围内）
2. 检查传感器VCC、GND和信号线连接
3. 确认GPIO配置为ADC输入模式
4. 在信号线上添加滤波电容（0.1uF）
5. 远离PWM信号线和电机

#### 问题3：I2C通信失败

**现象**：
```
E (1234) i2c: i2c_master_cmd_begin(1234): I2C transaction failed
E (1235) pcf8574: PCF8574初始化失败
```

**原因**：
- I2C设备未正确连接
- 缺少上拉电阻
- I2C地址错误
- SCL/SDA引脚配置错误

**解决方案**：
1. 检查I2C设备的VCC、GND、SCL、SDA连接
2. 确认SCL和SDA有4.7kΩ上拉电阻
3. 使用I2C扫描工具确认设备地址
4. 检查`pin_definitions.h`中的I2C引脚定义
5. 降低I2C时钟频率（从400kHz降到100kHz）

#### 问题4：UART通信无数据

**现象**：
- UART发送数据，但接收端无响应
- UART接收不到数据

**原因**：
- TX/RX引脚接反
- 波特率不匹配
- RS485方向控制错误
- 电平不匹配（3.3V vs 5V）

**解决方案**：
1. 确认TX连接到对方RX，RX连接到对方TX
2. 检查双方波特率设置是否一致
3. 如果使用RS485，检查方向控制引脚（GPIO19）
4. 使用逻辑分析仪或示波器查看信号波形
5. 如果对方是5V设备，使用电平转换电路

#### 问题5：电机不转或转向错误

**现象**：
- 设置PWM后电机不转
- 电机转向与预期相反
- 电机转速不稳定

**原因**：
- 电机驱动板未供电
- PWM引脚接线错误
- PWM频率或占空比设置错误
- 电机驱动板使能信号未设置

**解决方案**：
1. 检查电机驱动板电源（通常5V-12V）
2. 确认PWM引脚连接正确（GPIO2-9）
3. 检查PWM频率（推荐1kHz-20kHz）
4. 如果转向错误，交换正反向PWM引脚
5. 检查电机驱动板使能信号（如果有）

### 调试工具和技巧

#### 使用GPIO管理器调试

```c
// 在main.c中添加
#include "gpio_manager.h"

void app_main(void) {
    // ... 初始化代码 ...
    
    // 打印GPIO分配表
    ESP_LOGI(TAG, "========== GPIO分配表 ==========");
    gpio_manager_print_allocation_table();
    
    // ... 其他代码 ...
}
```

#### 使用万用表测量

| 测量项 | 正常值 | 说明 |
|--------|--------|------|
| ESP32 3.3V电源 | 3.2V-3.4V | 电源稳定 |
| GPIO输出高电平 | 3.0V-3.3V | 数字输出 |
| GPIO输出低电平 | 0V-0.3V | 数字输出 |
| 灰度传感器输出 | 0V-3.3V | 模拟输出 |
| I2C SCL/SDA空闲 | 3.0V-3.3V | 上拉电阻正常 |

#### 使用逻辑分析仪

推荐使用逻辑分析仪调试以下信号：
- UART通信（TX/RX）
- I2C通信（SCL/SDA）
- 编码器信号（A/B相）
- PWM信号

#### 使用示波器

推荐使用示波器测量以下信号：
- PWM波形（频率、占空比）
- ADC输入信号（噪声、幅度）
- 编码器信号（上升沿、下降沿）

### 快速检查清单

#### 硬件连接检查

- [ ] 所有模块共地（GND连接）
- [ ] 电源电压正确（ESP32 3.3V，传感器3.3V-5V）
- [ ] I2C总线有上拉电阻（4.7kΩ）
- [ ] 传感器输出电压≤3.3V
- [ ] 电机驱动板独立供电
- [ ] TX/RX引脚未接反

#### 软件配置检查

- [ ] GPIO管理器已初始化（`board_config_init()`）
- [ ] 引脚定义正确（`pin_definitions.h`）
- [ ] 没有GPIO冲突（`gpio_manager_print_allocation_table()`）
- [ ] ADC通道配置正确（ADC1 vs ADC2）
- [ ] UART波特率匹配
- [ ] I2C地址正确

#### 功能测试检查

- [ ] ADC读取值在合理范围内（0-4095）
- [ ] I2C设备响应正常
- [ ] UART能收发数据
- [ ] PWM输出波形正确
- [ ] 编码器计数正常

---

## GPIO管理器使用示例

### 示例1：注册单个GPIO

```c
#include "gpio_manager.h"
#include "esp_log.h"

static const char *TAG = "MY_COMPONENT";

esp_err_t my_sensor_init(void) {
    // 注册GPIO47用于传感器
    esp_err_t ret = gpio_manager_register(
        GPIO_NUM_47,
        GPIO_FUNC_ADC,
        "my_sensor",
        "自定义传感器"
    );
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO47注册失败，可能已被占用");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "GPIO47注册成功");
    
    // 继续配置GPIO...
    return ESP_OK;
}
```

### 示例2：注册多个GPIO

```c
#include "gpio_manager.h"
#include "esp_log.h"

static const char *TAG = "MY_MODULE";

esp_err_t my_module_init(void) {
    esp_err_t ret;
    
    // 注册GPIO47（输入）
    ret = gpio_manager_register(
        GPIO_NUM_47,
        GPIO_FUNC_GPIO_IN,
        "my_module",
        "输入信号1"
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO47注册失败");
        return ESP_FAIL;
    }
    
    // 注册GPIO48（输出）
    ret = gpio_manager_register(
        GPIO_NUM_48,
        GPIO_FUNC_GPIO_OUT,
        "my_module",
        "输出信号1"
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO48注册失败");
        // 注意：如果第二个GPIO注册失败，第一个GPIO已经被占用
        // 可能需要清理资源或重启系统
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "所有GPIO注册成功");
    return ESP_OK;
}
```

### 示例3：在main.c中打印GPIO分配表

```c
#include "board_config.h"
#include "gpio_manager.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

void app_main(void) {
    // 1. 初始化GPIO管理器
    ESP_LOGI(TAG, "初始化GPIO管理器...");
    esp_err_t ret = board_config_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO管理器初始化失败");
        return;
    }
    
    // 2. 初始化所有组件（会自动注册GPIO）
    // ... 组件初始化代码 ...
    
    // 3. 打印GPIO分配表（调试用）
    ESP_LOGI(TAG, "========== GPIO分配表 ==========");
    gpio_manager_print_allocation_table();
    ESP_LOGI(TAG, "================================");
    
    // 4. 继续其他初始化...
}
```

### 示例4：错误处理

```c
#include "gpio_manager.h"
#include "esp_log.h"

static const char *TAG = "SENSOR";

esp_err_t sensor_init(gpio_num_t gpio_num) {
    // 尝试注册GPIO
    esp_err_t ret = gpio_manager_register(
        gpio_num,
        GPIO_FUNC_ADC,
        "sensor",
        "传感器输入"
    );
    
    if (ret != ESP_OK) {
        // 注册失败，打印详细错误信息
        ESP_LOGE(TAG, "GPIO%d注册失败", gpio_num);
        ESP_LOGE(TAG, "可能原因：");
        ESP_LOGE(TAG, "  1. GPIO已被其他组件占用");
        ESP_LOGE(TAG, "  2. GPIO编号无效");
        ESP_LOGE(TAG, "  3. GPIO管理器未初始化");
        
        // 打印当前GPIO分配表，帮助定位问题
        ESP_LOGE(TAG, "当前GPIO分配情况：");
        gpio_manager_print_allocation_table();
        
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "GPIO%d注册成功", gpio_num);
    return ESP_OK;
}
```

---

## 引脚特性详细说明

### 输入专用GPIO（只能输入，不能输出）

ESP32-S3的以下GPIO只能配置为输入模式：

| GPIO | ADC通道 | 特性 | 推荐用途 |
|------|---------|------|---------|
| GPIO34 | ADC1_CH6 | 仅输入 | ADC采样、数字输入 |
| GPIO35 | - | 仅输入 | UART1_TX（特殊） |
| GPIO36 | ADC1_CH0 | 仅输入 | UART1_RX、ADC采样 |
| GPIO37 | ADC1_CH1 | 仅输入 | ADC采样、数字输入 |
| GPIO38 | ADC1_CH2 | 仅输入 | ADC采样、数字输入 |
| GPIO39 | ADC1_CH3 | 仅输入 | ADC采样、数字输入 |

⚠️ **注意**：
- 这些GPIO不能用于PWM输出、数字输出
- 适合用于ADC采样、按键输入、传感器输入
- GPIO35虽然标记为仅输入，但可以作为UART1_TX使用（特殊情况）

### ADC通道分配详细说明

#### ADC1（推荐使用，与WiFi不冲突）

| GPIO | ADC通道 | 当前用途 | 可用性 |
|------|---------|---------|--------|
| GPIO36 | ADC1_CH0 | UART1_RX | ❌ 已占用 |
| GPIO37 | ADC1_CH1 | TM1637_DIO | ❌ 已占用 |
| GPIO38 | ADC1_CH2 | 红外避障 | ❌ 已占用 |
| GPIO39 | ADC1_CH3 | 交通灯信号1 | ❌ 已占用 |
| GPIO34 | ADC1_CH6 | TM1637_CLK | ❌ 已占用 |
| GPIO47 | ADC1_CH6 | 未分配 | ✅ 可用 |
| GPIO48 | ADC1_CH7 | 未分配 | ✅ 可用 |

**推荐配置**：
- 灰度传感器改用GPIO47/48（ADC1通道）
- 可与WiFi同时使用
- 不受WiFi启动影响

#### ADC2（与WiFi冲突，不推荐）

| GPIO | ADC通道 | 当前用途 | 可用性 |
|------|---------|---------|--------|
| GPIO0 | ADC2_CH0 | LED指示灯 | ❌ 已占用 |
| GPIO1-10 | ADC2_CH1-9 | 各种功能 | ⚠️ WiFi冲突 |
| GPIO18 | ADC2_CH7 | 灰度传感器左 | ⚠️ WiFi冲突 |
| GPIO20 | ADC2_CH9 | 灰度传感器右 | ⚠️ WiFi冲突 |

⚠️ **重要提示**：
- ADC2在WiFi启动后无法使用
- 当前灰度传感器使用ADC2，如需WiFi请改用GPIO47/48
- ADC2与WiFi共享硬件资源，无法同时工作

### 系统保留GPIO（不可用）

以下GPIO被ESP32系统占用，不能用于应用程序：

| GPIO | 功能 | 说明 |
|------|------|------|
| GPIO6-11 | SPI Flash | 连接到外部Flash，不可使用 |
| GPIO12 | MTDI | 启动时采样，影响启动模式 |
| GPIO13 | MTCK | JTAG调试接口 |
| GPIO14 | MTMS | JTAG调试接口 |
| GPIO15 | MTDO | JTAG调试接口 |

⚠️ **注意**：
- GPIO6-11绝对不能使用，会导致系统无法启动
- GPIO12-15可以使用，但会影响JTAG调试功能
- 当前项目GPIO14-17用于编码器，已占用

---

## 参考资料

1. ESP32-S3技术参考手册
2. 项目设计文档：`.kiro/specs/stm32-to-esp32-migration/design.md`
3. GPIO管理器文档：`components/gpio_manager/README.md`
