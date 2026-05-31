# Board Config 板级配置模块

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
### 主要特性

- GPIO分配表管理，维护所有GPIO引脚的分配信息
- 自动冲突检测，在注册时检测GPIO重复分配
- 统一引脚定义，所有引脚在`pin_definitions.h`中集中管理
- 一键初始化，自动初始化GPIO管理器
- 配置查询接口，提供UART、I2C、RS485等配置查询
- 向后兼容，保留`gpio_manager.h`接口
## 硬件连接

### 引脚分配概览

本模块不直接使用GPIO引脚，而是管理整个系统的GPIO分配。详细的引脚分配请参考`include/pin_definitions.h`。

| 功能类别 | GPIO数量 | 说明 |
|---------|---------|------|
| 电机PWM控制 | 8个 | GPIO2-9，LEDC通道0-7 |
| 编码器PCNT | 8个 | GPIO14-17, 41-42, 45-46 |
| UART通信 | 4个 | UART0(GPIO43-44), UART1(GPIO35-36) |
| I2C总线 | 2个 | GPIO20-21 |
| 灰度传感器 | 2个 | GPIO18, 20（ADC2通道） |
| 数码管显示 | 2个 | GPIO34, 37（TM1637） |
## 功能说明

### 核心功能

#### 功能1：GPIO资源分配和冲突检测（GPIO Manager）

GPIO管理器维护一个全局的GPIO分配表，记录每个GPIO的使用情况。当模块尝试注册GPIO时，管理器会检查该GPIO是否已被占用，如果已占用则报告冲突并拒绝注册。

**工作原理**：
1. 系统启动时，调用`board_config_init()`初始化GPIO管理器
2. 各模块在初始化时调用`gpio_manager_register()`注册使用的GPIO
3. GPIO管理器检查该GPIO是否已被占用
4. 如果已占用，记录详细的冲突信息并返回错误
5. 如果未占用，记录该GPIO的分配信息（功能类型、模块名称、描述）

**冲突检测示例**：
```
E (1234) GPIO_MANAGER: GPIO冲突检测到！
E (1234) GPIO_MANAGER:   GPIO20 已被占用:
E (1234) GPIO_MANAGER:     现有模块: gray_sensor
E (1234) GPIO_MANAGER:     现有功能: ADC输入
E (1234) GPIO_MANAGER:     现有描述: 右灰度传感器
E (1234) GPIO_MANAGER:   尝试分配:
E (1234) GPIO_MANAGER:     新模块: i2c_master
E (1234) GPIO_MANAGER:     新功能: I2C数据
E (1234) GPIO_MANAGER:     新描述: I2C主总线SDA
```

#### 功能2：引脚定义集中管理（Pin Definitions）

所有硬件接口的GPIO引脚在`pin_definitions.h`中集中定义，便于统一管理和修改。引脚定义包括：
- 电机PWM控制引脚（8个）
- 编码器PCNT引脚（8个）
- UART通信引脚（4个）
- I2C总线引脚（2个）
- 各类传感器引脚（多个）

**命名规范**：
- 功能模块_信号类型_GPIO：如`MOTOR1_FWD_GPIO`
- 接口标识注释：如`[SSA1]`、`[SSD3]`表示传感器扩展接口
- 配置参数：如`UART0_BAUD_RATE`、`I2C_MASTER_FREQ_HZ`

**引脚分配表格式**：
```c
// 电机PWM控制（8个GPIO）
#define MOTOR1_FWD_GPIO          GPIO_NUM_2   // 电机1正向 LEDC_CH0
#define MOTOR1_REV_GPIO          GPIO_NUM_3   // 电机1反向 LEDC_CH1
// ...

// UART0（日志输出 / RS485通信）
#define UART0_PORT               UART_NUM_0
#define UART0_TX_GPIO            GPIO_NUM_43  // UART0发送引脚
#define UART0_RX_GPIO            GPIO_NUM_44  // UART0接收引脚
#define UART0_BAUD_RATE          115200       // 默认波特率
```

#### 功能3：板级配置查询（Board Config）

提供便捷的配置查询接口，各模块可以通过API查询UART、I2C、RS485等配置，无需直接访问引脚定义。

**支持的配置查询**：
- UART配置：TX/RX引脚、波特率
- I2C配置：SCL/SDA引脚、频率
- RS485配置：方向控制引脚、电平
- 板级信息：板子名称、版本、GPIO统计

**优势**：
- 解耦硬件配置和业务逻辑
- 便于硬件配置变更
- 统一的配置管理
- 提高代码可维护性

#### 功能4：GPIO分配表打印和验证

提供调试接口，可以打印当前的GPIO分配表，验证GPIO分配的正确性。

**打印示例**：
```
I (1234) BOARD_CONFIG: ========================================
I (1234) BOARD_CONFIG: 板级配置摘要
I (1234) BOARD_CONFIG: ========================================
I (1234) BOARD_CONFIG: 板子名称: ESP32-S3 巡线小车
I (1234) BOARD_CONFIG: 硬件版本: v1.0 (飞线修改版)
I (1234) BOARD_CONFIG: GPIO总数: 40
I (1234) BOARD_CONFIG: 已分配GPIO: 24
I (1234) BOARD_CONFIG: 未使用GPIO: 16

| 其他传感器 | 多个 | 详见pin_definitions.h |

###功能类型

GPIO管理器支持以下功能类型：

| 功能类型 | 枚举值 | 说明 |
|---------|--------|------|
| 未使用 | GPIO_FUNC_UNUSED | 初始状态 |
| ADC输入 | GPIO_FUNC_ADC | 模拟输入 |
| PWM输出 | GPIO_FUNC_PWM | 脉宽调制输出 |
| UART发送 | GPIO_FUNC_UART_TX | 串口发送 |
| UART接收 | GPIO_FUNC_UART_RX | 串口接收 |
| I2C数据 | GPIO_FUNC_I2C_SDA | I2C数据线 |
| I2C时钟 | GPIO_FUNC_I2C_SCL | I2C时钟线 |
| GPIO输出 | GPIO_FUNC_GPIO_OUT | 通用输出 |
| GPIO输入 | GPIO_FUNC_GPIO_IN | 通用输入 |
| 触摸传感器 | GPIO_FUNC_TOUCH | 触摸输入 |
| DHT11 | GPIO_FUNC_DHT11 | 温湿度传感器 |
| TM1637时钟 | 数码管时钟 |
| TM1637数据 | GPIO_FUNC_TM1637_DIO | 数码管数据 |
| RS485方向 | GPIO_FUNC_RS485_DIR | RS485方向控制 |
| 编码器 | GPIO_FUNC_ENCODER | 编码器输入 |

### 注意事项

- ⚠️ GPIO34-39只能作为输入，不能配置为输出
- ⚠️ GPIO6-11连接到SPI Flash，禁止使用
- ⚠️ ADC2（GPIO0-20）与WiFi冲突，启用WiFi时ADC2不可用
- ⚠️ 当前GPIO20被灰度传感器占用，导致PCF8574扩展芯片不可用

---

## 功能说明

### 核心功能

#### 功能1：GPIO资源分配和冲突检测

GPIO管理器维护一个全局的GPIO分配表，记录每个GPIO的使用情况。当模块尝试注册GPIO时，管理器会检查该GPIO是否已被占用，如果已占用则报告冲突并拒绝注册。

**工作原理**：
1. 系统启动时，调用`board_config_init()`初始化GPIO管理器
2. 各模块在初始化时调用的GPIO
3. GPIO管理器检查该GPIO是否已被占用
4. 如果已占用，记录详细的冲突信息并返回错误
5. 如果未占用，记录该GPIO的分配信息

**冲突检测示例**：
```
E (1234) GPIO_MANAGER: GPIO冲突检测到！
E (1234) GPIO_MANAGER:   GPIO20 已被占用:
E (1234) GPIO_MANAGER:     现有模块: gray_sensor
E (1234) GPIO_MANAGER:     现有功能: ADC输入
E (1234) GPIO_MANAGER:     现有描述: 右灰度传感器
E (1234) GPIO_MANAGER:   尝试分配:
E (1234) GPIO_MANAGER:     新模块: i2c_master
E (1234) GPIO_MANAGER:     新功能: I2C数据
E (1234) GPIO_MANAGER:     新描述: I2C主总线SDA
```

#### 功能2：引脚定义集中管理

所有硬件接口的GPI引脚定义包括：
- 电机PWM控制引脚（8个）
- 编码器PCNT引脚（8个）
- UART通信引脚（4个）
- I2C总线引脚（2个）
- 各类传感器引脚（多个）

**命名规范**：
- 功能模块_信号类型_GPIO：如`MOTOR1_FWD_GPIO`
- 接口标识注释：如`[SSA1]`、`[SSD3]`表示传感器扩展接口

**引脚分配表格式**：
```c
// 电机PWM控制（8个GPIO）
#define MOTOR1_FWD_GPIO          GPIO_NUM_2   // 电机1正向 LEDC_CH0
#define MOTOR1_REV_GPIO          GPIO_NUM_3   // 电机1反向 LEDC_CH1
// ...

// UART0（日志输出 / RS485通信）
#define UART0_PORT               UART_NUM_0
#define UART0_TX_GPIO            GPIO_NUM_43  // UART0发送引脚
#define UART UART0接收引脚
#define UART0_BAUD_RATE          115200       // 默认波特率
```

#### 功能3：板级配置查询

提供便捷的配置查询接口，各模块可以通过API查询UART、I2C、RS485等配置，无需直接访问引脚定义。

**支持的配置查询**：
- UART配置：TX/RX引脚、波特率
- I2C配置：SCL/SDA引脚、频率
- RS485配置：方向控制引脚、电平

**优势**：
- 解耦硬件配置和业务逻辑
- 便于硬件配置变更
- 统一的配置管理
pio_manager_verify(void);
```

**参数说明**：
- 无

**返回值**：
- `ESP_OK`: 验证通过
- `ESP_FAIL`: 发现冲突或错误

**使用说明**：
用于验证GPIO分配表的完整性，检查是否有模块名称或描述为空的情况。

---
location);
```

**参数说明**：
- `gpio_num`: GPIO编号（0-39）
- `allocation`: 输出参数，存储分配信息（可为NULL）

**返回值**：
- `true`: GPIO已被占用
- `false`: GPIO未被占用

**使用说明**：
用于查询GPIO是否已被占用，以及占用的详细信息。

#### 打印GPIO分配表

```c
/**
 * @brief 打印GPIO分配表
 * 
 * 打印所有已分配的GPIO信息，用于调试和验证
 */
void gpio_manager_print_allocation_table(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
用于调试，打印详细的GPIO分配表。

#### 验证GPIO分配表

```c
/**
 * @brief 验证GPIO分配表的完整性
 * 
 * 检查是否有冲突或无效的分配
 * 
 * @return 
 *     - ESP_OK: 验证通过
 *     - ESP_FAIL: 发现冲突或错误
 */
esp_err_t g `function`: 功能类型（GPIO_FUNC_xxx）
- `module_name`: 模块名称（字符串常量）
- `description`: 描述信息（字符串常量）

**返回值**：
- `ESP_OK`: 注册成功
- `ESP_ERR_INVALID_ARG`: 无效的GPIO编号
- `ESP_ERR_INVALID_STATE`: GPIO已被占用（冲突）

**使用说明**：
各模块在初始化时调用，注册使用的GPIO。如果检测到冲突，会记录详细的错误日志。

#### 检查GPIO是否已占用

```c
/**
 * @brief 检查GPIO是否已被占用
 * 
 * @param gpio_num GPIO编号
 * @param[out] allocation 如果已占用，返回分配信息（可为NULL）
 * 
 * @return 
 *     - true: GPIO已被占用
 *     - false: GPIO未被占用
 */
bool gpio_manager_is_allocated(gpio_num_t gpio_num, gpio_allocation_t *alf 注册GPIO使用
 * 
 * @param gpio_num GPIO编号
 * @param function 功能类型
 * @param module_name 模块名称
 * @param description 描述信息
 * 
 * @return 
 *     - ESP_OK: 注册成功
 *     - ESP_ERR_INVALID_ARG: 无效的GPIO编号
 *     - ESP_ERR_INVALID_STATE: GPIO已被其他模块占用（冲突）
 */
esp_err_t gpio_manager_register(gpio_num_t gpio_num, 
                                gpio_function_t function,
                                const char *module_name,
                                const char *description);
```

**参数说明**：
- `gpio_num`: GPIO编号（0-39）
-;
```

**参数说明**：
- `dir_pin`: 输出参数，方向控制引脚
- `tx_level`: 输出参数，发送模式电平（0或1）
- `rx_level`: 输出参数，接收模式电平（0或1）

**返回值**：
- `ESP_OK`: 成功
- `ESP_ERR_INVALID_ARG`: 参数为NULL

**使用说明**：
用于查询RS485配置，避免硬编码引脚号。

---

### GPIO管理接口（向后兼容）

#### 初始化GPIO管理器

```c
/**
 * @brief 初始化GPIO管理器
 * 
 * @return 
 *     - ESP_OK: 成功
 *     - ESP_FAIL: 失败
 */
esp_err_t gpio_manager_init(void);
```

**参数说明**：
- 无

**返回值**：
- `ESP_OK`: 初始化成功
- `ESP_FAIL`: 初始化失败

**使用说明**：
通常不需要直接调用，`board_config_init()`会自动调用。

#### 注册GPIO使用

```c
/**
 * @briem`: I2C端口号（I2C_NUM_0）
- `scl_pin`: 输出参数，时钟引脚
- `sda_pin`: 输出参数，数据引脚
- `freq_hz`: 输出参数，频率（Hz）

**返回值**：
- `ESP_OK`: 成功
- `ESP_ERR_INVALID_ARG`: 无效的I2C端口或参数为NULL

**使用说明**：
用于查询I2C配置，避免硬编码引脚号。

#### 获取RS485配置

```c
/**
 * @brief 获取RS485配置
 * 
 * @param[out] dir_pin 方向控制引脚
 * @param[out] tx_level 发送模式电平
 * @param[out] rx_level 接收模式电平
 * 
 * @return ESP_OK: 成功
 */
esp_err_t board_config_get_rs485(gpio_num_t *dir_pin,
                                 uint8_t *tx_level,
                                 uint8_t *rx_level)_ARG`: 无效的UART端口或参数为NULL

**使用说明**：
用于查询UART配置，避免硬编码引脚号。

#### 获取I2C配置

```c
/**
 * @brief 获取I2C配置
 * 
 * @param i2c_num I2C端口号
 * @param[out] scl_pin 时钟引脚
 * @param[out] sda_pin 数据引脚
 * @param[out] freq_hz 频率
 * 
 * @return 
 *   - ESP_OK: 成功
 *   - ESP_ERR_INVALID_ARG: 无效的I2C端口
 */
esp_err_t board_config_get_i2c(i2c_port_t i2c_num,
                               gpio_num_t *scl_pin,
                               gpio_num_t *sda_pin,
                               uint32_t *freq_hz);
```

**参数说明**：
- `i2c_nu * @param[out] rx_pin 接收引脚
 * @param[out] baud_rate 波特率
 * 
 * @return 
 *   - ESP_OK: 成功
 *   - ESP_ERR_INVALID_ARG: 无效的UART端口
 */
esp_err_t board_config_get_uart(uart_port_t uart_num, 
                                gpio_num_t *tx_pin, 
                                gpio_num_t *rx_pin,
                                uint32_t *baud_rate);
```

**参数说明**：
- `uart_num`: UART端口号（UART_NUM_0或UART_NUM_1）
- `tx_pin`: 输出参数，发送引脚
- `rx_pin`: 输出参数，接收引脚
- `baud_rate`: 输出参数，波特率

**返回值**：
- `ESP_OK`: 成功
- `ESP_ERR_INVALID
 *   - ESP_ERR_INVALID_ARG: 参数无效
 */
esp_err_t board_config_get_info(board_info_t *info);
```

**参数说明**：
- `info`: 输出参数，存储板级信息

**返回值**：
- `ESP_OK`: 成功
- `ESP_ERR_INVALID_ARG`: 参数为NULL

**使用说明**：
用于查询板子名称、版本、GPIO统计信息。

#### 打印配置摘要

```c
/**
 * @brief 打印板级配置摘要
 * 
 * 打印板子信息和GPIO分配统计
 */
void board_config_print_summary(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
用于调试，打印板级配置摘要和GPIO分配表。

---

### 配置查询接口

#### 获取UART配置

```c
/**
 * @brief 获取UART配置
 * 
 * @param uart_num UART端口号
 * @param[out] tx_pin 发送引脚
BAUD_RATE | 115200 | 9600-921600 | UART1波特率 |
| I2C_MASTER_FREQ_HZ | 100000 | 10000-400000 | I2C频率（Hz） |

---

## API接口

### 板级配置接口

#### 初始化函数

```c
/**
 * @brief 初始化板级配置系统
 * 
 * 包含GPIO管理器初始化
 * 
 * @return 
 *   - ESP_OK: 初始化成功
 *   - ESP_FAIL: 初始化失败
 */
esp_err_t board_config_init(void);
```

**参数说明**：
- 无

**返回值**：
- `ESP_OK`: 初始化成功
- `ESP_FAIL`: 初始化失败

**使用说明**：
在所有硬件模块初始化之前调用，通常在`app_main()`的开头。

#### 获取板级信息

```c
/**
 * @brief 获取板级信息
 * 
 * @param[out] info 板级信息结构体指针
 * 
 * @return 
 *   - ESP_OK: 成功+------------------------
I (1234) GPIO_MANAGER: 2    | PWM输出       | pwm                | 电机1正向
I (1234) GPIO_MANAGER: 3    | PWM输出       | pwm                | 电机1反向
I (1234) GPIO_MANAGER: ...
I (1234) GPIO_MANAGER: ================================
I (1234) GPIO_MANAGER: 总计: 24个GPIO已分配
```

### 配置参数

本模块的配置参数主要在`pin_definitions.h`中定义：

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| MAX_GPIO_NUM | 40 | 固定 | ESP32-S3的GPIO总数 |
| UART0_BAUD_RATE | 115200 | 9600-921600 | UART0波特率 |
| UART1_
#### 功能4：GPIO分配表打印和验证

提供调试接口，可以打印当前的GPIO分配表，验证GPIO分配的正确性。

**打印示例**：
```
I (1234) GPIO_MANAGER: ========== GPIO分配表 ==========
I (1234) GPIO_MANAGER: GPIO | 功能类型      | 模块名称           | 描述
I (1234) GPIO_MANAGER: -----+---------------+--------------------
## API接口

### 板级配置接口

#### 初始化函数

```c
/**
 * @brief 初始化板级配置系统
 * 
 * 包含GPIO管理器初始化
 * 
 * @return 
 *   - ESP_OK: 初始化成功
 *   - ESP_FAIL: 初始化失败
 */
esp_err_t board_config_init(void);
```

**参数说明**：
- 无

**返回值**：
- `ESP_OK`: 初始化成功
- `ESP_FAIL`: 初始化失败

**使用说明**：
在所有硬件模块初始化之前调用，通常在`app_main()`的开头。

---

### 主要功能函数

#### 获取板级信息

```c
/**
 * @brief 获取板级信息
 * 
 * @param[out] info 板级信息结构体指针
 * 
 * @return 
 *   - ESP_OK: 成功
 *   - ESP_ERR_INVALID_ARG: 参数无效
 */
esp_err_t board_config_get_info(board_info_t *info);
```

**参数说明**：
- `info`: 输出参数，存储板级信息

**返回值**：
- `ESP_OK`: 成功
- `ESP_ERR_INVALID_ARG`: 参数为NULL

**使用说明**：
用于查询板子名称、版本、GPIO统计信息。

#### 打印配置摘要

```c
/**
 * @brief 打印板级配置摘要
 * 
 * 打印板子信息和GPIO分配统计
 */
void board_config_print_summary(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
用于调试，打印板级配置摘要和GPIO分配表。

#### 获取UART配置

```c
/**
 * @brief 获取UART配置
 * 
 * @param uart_num UART端口号
 * @param[out] tx_pin 发送引脚
 * @param[out] rx_pin 接收引脚
 * @param[out] baud_rate 波特率
 * 
 * @return 
 *   - ESP_OK: 成功
 *   - ESP_ERR_INVALID_ARG: 无效的UART端口
 */
esp_err_t board_config_get_uart(uart_port_t uart_num, 
                                gpio_num_t *tx_pin, 
                                gpio_num_t *rx_pin,
                                uint32_t *baud_rate);
```

**参数说明**：
- `uart_num`: UART端口号（UART_NUM_0或UART_NUM_1）
- `tx_pin`: 输出参数，发送引脚
- `rx_pin`: 输出参数，接收引脚
- `baud_rate`: 输出参数，波特率

**返回值**：
- `ESP_OK`: 成功
- `ESP_ERR_INVALID_ARG`: 无效的UART端口或参数为NULL

**使用说明**：
用于查询UART配置，避免硬编码引脚号。

#### 获取I2C配置

```c
/**
 * @brief 获取I2C配置
 * 
 * @param i2c_num I2C端口号
 * @param[out] scl_pin 时钟引脚
 * @param[out] sda_pin 数据引脚
 * @param[out] freq_hz 频率
 * 
 * @return 
 *   - ESP_OK: 成功
 *   - ESP_ERR_INVALID_ARG: 无效的I2C端口
 */
esp_err_t board_config_get_i2c(i2c_port_t i2c_num,
                               gpio_num_t *scl_pin,
                               gpio_num_t *sda_pin,
                               uint32_t *freq_hz);
```

**参数说明**：
- `i2c_num`: I2C端口号（I2C_NUM_0）
- `scl_pin`: 输出参数，时钟引脚
- `sda_pin`: 输出参数，数据引脚
- `freq_hz`: 输出参数，频率（Hz）

**返回值**：
- `ESP_OK`: 成功
- `ESP_ERR_INVALID_ARG`: 无效的I2C端口或参数为NULL

**使用说明**：
用于查询I2C配置，避免硬编码引脚号。

#### 获取RS485配置

```c
/**
 * @brief 获取RS485配置
 * 
 * @param[out] dir_pin 方向控制引脚
 * @param[out] tx_level 发送模式电平
 * @param[out] rx_level 接收模式电平
 * 
 * @return ESP_OK: 成功
 */
esp_err_t board_config_get_rs485(gpio_num_t *dir_pin,
                                 uint8_t *tx_level,
                                 uint8_t *rx_level);
```

**参数说明**：
- `dir_pin`: 输出参数，方向控制引脚
- `tx_level`: 输出参数，发送模式电平（0或1）
- `rx_level`: 输出参数，接收模式电平（0或1）

**返回值**：
- `ESP_OK`: 成功
- `ESP_ERR_INVALID_ARG`: 参数为NULL

**使用说明**：
用于查询RS485配置，避免硬编码引脚号。

---

### GPIO管理接口（向后兼容）

#### 初始化GPIO管理器

```c
/**
 * @brief 初始化GPIO管理器
 * 
 * @return 
 *     - ESP_OK: 成功
 *     - ESP_FAIL: 失败
 */
esp_err_t gpio_manager_init(void);
```

**参数说明**：
- 无

**返回值**：
- `ESP_OK`: 初始化成功
- `ESP_FAIL`: 初始化失败

**使用说明**：
通常不需要直接调用，`board_config_init()`会自动调用。

#### 注册GPIO使用

```c
/**
 * @brief 注册GPIO使用
 * 
 * @param gpio_num GPIO编号
 * @param function 功能类型
 * @param module_name 模块名称
 * @param description 描述信息
 * 
 * @return 
 *     - ESP_OK: 注册成功
 *     - ESP_ERR_INVALID_ARG: 无效的GPIO编号
 *     - ESP_ERR_INVALID_STATE: GPIO已被其他模块占用（冲突）
 */
esp_err_t gpio_manager_register(gpio_num_t gpio_num, 
                                gpio_function_t function,
                                const char *module_name,
                                const char *description);
```

**参数说明**：
- `gpio_num`: GPIO编号（0-39）
- `function`: 功能类型（GPIO_FUNC_xxx）
- `module_name`: 模块名称（字符串常量）
- `description`: 描述信息（字符串常量）

**返回值**：
- `ESP_OK`: 注册成功
- `ESP_ERR_INVALID_ARG`: 无效的GPIO编号
- `ESP_ERR_INVALID_STATE`: GPIO已被占用（冲突）

**使用说明**：
各模块在初始化时调用，注册使用的GPIO。如果检测到冲突，会记录详细的错误日志。

#### 检查GPIO是否已占用

```c
/**
 * @brief 检查GPIO是否已被占用
 * 
 * @param gpio_num GPIO编号
 * @param[out] allocation 如果已占用，返回分配信息（可为NULL）
 * 
 * @return 
 *     - true: GPIO已被占用
 *     - false: GPIO未被占用
 */
bool gpio_manager_is_allocated(gpio_num_t gpio_num, gpio_allocation_t *allocation);
```

**参数说明**：
- `gpio_num`: GPIO编号（0-39）
- `allocation`: 输出参数，存储分配信息（可为NULL）

**返回值**：
- `true`: GPIO已被占用
- `false`: GPIO未被占用

**使用说明**：
用于查询GPIO是否已被占用，以及占用的详细信息。

#### 打印GPIO分配表

```c
/**
 * @brief 打印GPIO分配表
 * 
 * 打印所有已分配的GPIO信息，用于调试和验证
 */
void gpio_manager_print_allocation_table(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
用于调试，打印详细的GPIO分配表。

#### 验证GPIO分配表

```c
/**
 * @brief 验证GPIO分配表的完整性
 * 
 * 检查是否有冲突或无效的分配
 * 
 * @return 
 *     - ESP_OK: 验证通过
 *     - ESP_FAIL: 发现冲突或错误
 */
esp_err_t gpio_manager_verify(void);
```

**参数说明**：
- 无

**返回值**：
- `ESP_OK`: 验证通过
- `ESP_FAIL`: 发现冲突或错误

**使用说明**：
用于验证GPIO分配表的完整性，检查是否有模块名称或描述为空的情况。

---

## 使用示例

### 基本使用示例

```c
#include "board_config.h"
#include "pin_definitions.h"
#include "esp_log.h"

static const char *TAG = "BOARD_EXAMPLE";

void app_main(void)
{
    // 1. 初始化板级配置系统
    esp_err_t ret = board_config_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "板级配置初始化失败，系统停止启动");
        return;
    }
    ESP_LOGI(TAG, "板级配置初始化成功");
    
    // 2. 打印配置摘要（可选，用于调试）
    board_config_print_summary();
    
    // 3. 使用引脚定义初始化外设
    // 例如：配置电机PWM引脚
    gpio_set_direction(MOTOR1_FWD_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR1_REV_GPIO, GPIO_MODE_OUTPUT);
    
    // 4. 查询UART配置
    gpio_num_t tx_pin, rx_pin;
    uint32_t baud_rate;
    ret = board_config_get_uart(UART_NUM_0, &tx_pin, &rx_pin, &baud_rate);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "UART0配置: TX=%d, RX=%d, 波特率=%lu", tx_pin, rx_pin, baud_rate);
    }
    
    // 5. 继续初始化其他模块...
}
```

### 高级使用示例 - 注册自定义GPIO

```c
#include "board_config.h"
#include "gpio_manager.h"
#include "esp_log.h"

static const char *TAG = "CUSTOM_MODULE";

esp_err_t custom_module_init(void)
{
    // 1. 注册自定义GPIO（例如：GPIO12用于自定义输出）
    esp_err_t ret = gpio_manager_register(
        GPIO_NUM_12,
        GPIO_FUNC_GPIO_OUT,
        "custom_module",
        "自定义输出引脚"
    );
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO12注册失败，可能已被其他模块占用");
        return ret;
    }
    
    ESP_LOGI(TAG, "GPIO12注册成功");
    
    // 2. 配置GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_12),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    
    // 3. 使用GPIO
    gpio_set_level(GPIO_NUM_12, 1);
    
    return ESP_OK;
}
```

### 完整集成示例

```c
#include "board_config.h"
#include "pin_definitions.h"
#include "gray_sensor.h"
#include "pwm.h"
#include "tm1637.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    esp_err_t ret;
    
    // ========== 阶段1：初始化GPIO管理器 ==========
    ESP_LOGI(TAG, "初始化GPIO管理器...");
    ret = board_config_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO管理器初始化失败，系统停止");
        return;
    }
    
    // ========== 阶段2：初始化硬件外设 ==========
    ESP_LOGI(TAG, "初始化硬件外设...");
    
    // 初始化PWM（电机控制）
    ledc_init();
    
    // 初始化TM1637数码管
    tm1637_init();
    
    // 初始化灰度传感器
    gray_sensor_init_simple();
    
    // ========== 阶段3：打印GPIO分配表（调试用） ==========
    ESP_LOGI(TAG, "打印GPIO分配表：");
    gpio_manager_print_allocation_table();
    
    // ========== 阶段4：查询配置信息 ==========
    // 查询板级信息
    board_info_t info;
    ret = board_config_get_info(&info);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "板子名称: %s", info.board_name);
        ESP_LOGI(TAG, "硬件版本: %s", info.board_version);
        ESP_LOGI(TAG, "GPIO总数: %lu", info.total_gpio_count);
        ESP_LOGI(TAG, "已分配GPIO: %lu", info.allocated_gpio_count);
    }
    
    // 查询UART配置
    gpio_num_t tx, rx;
    uint32_t baud;
    board_config_get_uart(UART_NUM_0, &tx, &rx, &baud);
    ESP_LOGI(TAG, "UART0: TX=%d, RX=%d, 波特率=%lu", tx, rx, baud);
    
    // 查询I2C配置
    gpio_num_t scl, sda;
    uint32_t freq;
    board_config_get_i2c(I2C_NUM_0, &scl, &sda, &freq);
    ESP_LOGI(TAG, "I2C0: SCL=%d, SDA=%d, 频率=%lu Hz", scl, sda, freq);
    
    ESP_LOGI(TAG, "系统启动完成！");
    
    // 主循环
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

---

## 注意事项

### 硬件限制

- ⚠️ **GPIO34-39只能作为输入**：这些引脚没有输出驱动能力，只能配置为输入模式
- ⚠️ **GPIO6-11禁止使用**：这些引脚连接到SPI Flash，使用会导致系统崩溃
- ⚠️ **ADC2与WiFi冲突**：GPIO0-20属于ADC2通道，启用WiFi时ADC2不可用
- ⚠️ **当前硬件状态**：GPIO20被灰度传感器占用，导致I2C主总线的SDA引脚冲突，PCF8574扩展芯片不可用

### 软件限制

- ⚠️ **初始化顺序**：必须在所有硬件模块初始化之前调用`board_config_init()`
- ⚠️ **字符串常量**：`gpio_manager_register()`的模块名称和描述参数必须是字符串常量，不能是局部变量
- ⚠️ **冲突检测**：GPIO冲突会导致注册失败，需要检查返回值并处理错误
- ⚠️ **线程安全**：GPIO管理器不是线程安全的，所有注册操作应在初始化阶段完成

### 线程安全

- GPIO管理器的初始化和注册操作不是线程安全的
- 建议在系统启动阶段（单线程环境）完成所有GPIO注册
- 查询操作（`gpio_manager_is_allocated`、`board_config_get_xxx`）是只读的，可以在多线程环境中使用
- 打印操作（`gpio_manager_print_allocation_table`、`board_config_print_summary`）不是线程安全的，仅用于调试

### 性能考虑

- GPIO管理器使用静态数组存储分配表，查询操作时间复杂度为O(1)
- 注册操作时间复杂度为O(1)，仅检查单个GPIO是否已占用
- 打印操作时间复杂度为O(n)，n为GPIO总数（40）
- 内存占用：约1.6KB（40个GPIO × 40字节/GPIO）

---

## 故障排除

### 常见问题

#### 问题1：GPIO冲突检测到

**现象**：日志显示"GPIO冲突检测到！"，某个GPIO已被占用

**原因**：
1. 两个模块尝试使用同一个GPIO
2. 引脚定义错误，多个功能使用了相同的GPIO编号
3. 硬件飞线导致引脚分配冲突

**解决方案**：
1. 检查日志中的冲突信息，确定哪两个模块冲突
2. 修改`pin_definitions.h`中的引脚定义，为其中一个模块分配不同的GPIO
3. 如果是硬件飞线导致的冲突，修改硬件连接或更新引脚定义
4. 如果某个模块不需要使用，注释掉其初始化代码

#### 问题2：板级配置初始化失败

**现象**：`board_config_init()`返回`ESP_FAIL`

**原因**：
1. GPIO管理器初始化失败（内存不足）
2. 系统资源不足

**解决方案**：
1. 检查堆内存是否充足（使用`esp_get_free_heap_size()`）
2. 增加堆内存大小（menuconfig -> Component config -> ESP32-specific -> Minimum free heap size）
3. 减少其他模块的内存占用

#### 问题3：查询配置返回错误

**现象**：`board_config_get_uart()`或其他查询函数返回`ESP_ERR_INVALID_ARG`

**原因**：
1. 传入的端口号无效（如UART_NUM_2，ESP32-S3只有UART0和UART1）
2. 输出参数为NULL

**解决方案**：
1. 检查端口号是否正确（UART_NUM_0、UART_NUM_1、I2C_NUM_0）
2. 确保所有输出参数不为NULL
3. 参考API文档，确认支持的端口号

#### 问题4：引脚定义与实际硬件不符

**现象**：外设无法正常工作，但代码逻辑正确

**原因**：
1. `pin_definitions.h`中的引脚定义与实际硬件连接不符
2. 硬件飞线后未更新引脚定义
3. 使用了错误的引脚宏定义

**解决方案**：
1. 对照实际硬件连接，检查`pin_definitions.h`中的引脚定义
2. 使用万用表或示波器测量引脚信号，确认硬件连接
3. 更新引脚定义后重新编译和烧录
4. 打印GPIO分配表，确认引脚分配正确

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [GPIO引脚分配文档](../../docs/GPIO_PIN_ALLOCATION.md)
- [配置参数指南](../../docs/CONFIGURATION_GUIDE.md)
- [STM32到ESP32迁移规格](../../.kiro/specs/stm32-to-esp32-migration/)

### 数据手册

- [ESP32-S3技术参考手册](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)
- [ESP-IDF GPIO API文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html)

### 代码示例

- [完整集成示例](../../main/main.c)
- [GPIO测试程序](../../components/board_config/test/test_gpio_manager.c)

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，整合GPIO管理、引脚定义和板级配置 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/board_config/`  
**文档类型**: 组件使用说明
