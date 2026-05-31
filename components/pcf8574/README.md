# PCF8574 I2C GPIO扩展模块

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: ⏸️ 未使用

---

## 组件简介

PCF8574是一款通过I2C总线控制的8位GPIO扩展芯片，可以为ESP32提供额外的8个可编程I/O引脚。该驱动使用ESP-IDF 5.3+的新I2C master驱动，支持多设备共享I2C总线，并提供了引用计数管理机制。

### 主要特性

- 通过I2C总线扩展8个GPIO引脚
- 支持输入和输出模式
- I2C地址可配置（PCF8574: 0x20-0x27, PCF8574A: 0x38-0x3F）
- 支持400kHz I2C通信速率
- 多设备共享I2C总线，自动管理总线引用计数
- 兼容ESP-IDF 5.3+的新I2C master驱动
- 提供端口读写和单引脚操作接口

### 适用场景

适用于GPIO引脚不足的场景，如需要控制大量LED、按键、继电器等外设。可以通过一条I2C总线连接多个PCF8574芯片，最多扩展64个GPIO（8个芯片×8引脚）。

---

## 硬件连接

### 引脚分配

| 功能 | ESP32引脚 | PCF8574引脚 | 说明 |
|------|----------|------------|------|
| I2C SDA | 可配置 | SDA | I2C数据线 |
| I2C SCL | 可配置 | SCL | I2C时钟线 |
| VCC | 3.3V | VCC | 电源正极 |
| GND | GND | GND | 电源地 |

### PCF8574引脚功能

| 引脚名称 | 功能 | 说明 |
|---------|------|------|
| P0-P7 | GPIO | 8个可编程I/O引脚 |
| A0-A2 | 地址选择 | 配置I2C地址 |
| INT | 中断输出 | 输入状态变化时触发（可选） |

### I2C地址配置

**PCF8574地址范围：0x20-0x27**

| A2 | A1 | A0 | I2C地址 |
|----|----|----|---------|
| 0  | 0  | 0  | 0x20    |
| 0  | 0  | 1  | 0x21    |
| 0  | 1  | 0  | 0x22    |
| 0  | 1  | 1  | 0x23    |
| 1  | 0  | 0  | 0x24    |
| 1  | 0  | 1  | 0x25    |
| 1  | 1  | 0  | 0x26    |
| 1  | 1  | 1  | 0x27    |

**PCF8574A地址范围：0x38-0x3F**（地址配置方式相同）

### 接线说明

1. 将PCF8574的SDA引脚连接到ESP32的I2C SDA引脚
2. 将PCF8574的SCL引脚连接到ESP32的I2C SCL引脚
3. 将PCF8574的VCC连接到3.3V电源
4. 将PCF8574的GND连接到地线
5. 根据需要配置A0-A2引脚设置I2C地址（接GND为0，接VCC为1）
6. I2C总线需要上拉电阻（通常4.7kΩ），驱动已启用内部上拉

### 注意事项

- ⚠️ **电源电压**：PCF8574支持2.5V-6V，使用3.3V与ESP32兼容
- ⚠️ **上拉电阻**：驱动已启用内部上拉，通常无需外接上拉电阻
- ⚠️ **I2C地址**：确保同一总线上的设备地址不冲突
- ⚠️ **电流限制**：每个引脚最大输出电流25mA，总电流不超过100mA

---

## 功能说明

### 核心功能

#### 功能1：设备初始化

初始化PCF8574设备描述符，配置I2C总线和设备参数。支持多设备共享同一I2C总线，自动管理总线引用计数。

#### 功能2：端口读写

提供8位端口整体读写功能，可以一次性读取或设置所有8个GPIO引脚的状态。

#### 功能3：单引脚操作

提供单个引脚的读写功能，可以独立控制某个GPIO引脚，无需操作整个端口。

#### 功能4：I2C总线管理

自动管理I2C总线的创建和销毁，支持多个设备共享同一总线，使用引用计数避免重复初始化。

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| I2C频率 | 400kHz | 100kHz-400kHz | I2C通信速率 |
| I2C地址 | 可配置 | 0x20-0x27 (PCF8574) | 通过A0-A2引脚配置 |
| I2C端口 | 可配置 | I2C_NUM_0/1 | ESP32的I2C端口号 |
| SDA引脚 | 可配置 | GPIO0-48 | I2C数据线引脚 |
| SCL引脚 | 可配置 | GPIO0-48 | I2C时钟线引脚 |

---

## API接口

### 初始化函数

```c
/**
 * @brief 初始化PCF8574设备描述符
 * 
 * @param dev 指向I2C设备描述符的指针
 * @param addr I2C地址 (PCF8574: 0x20-0x27, PCF8574A: 0x38-0x3F)
 * @param port I2C端口号
 * @param sda_gpio SDA引脚
 * @param scl_gpio SCL引脚
 * @return ESP_OK 成功，其他值失败
 */
esp_err_t pcf8574_init_desc(i2c_dev_t *dev, uint8_t addr, i2c_port_t port, 
                            gpio_num_t sda_gpio, gpio_num_t scl_gpio);
```

**参数说明**：
- `dev`: 设备描述符指针，需要预先分配内存
- `addr`: PCF8574的I2C地址（7位地址）
- `port`: I2C端口号（I2C_NUM_0或I2C_NUM_1）
- `sda_gpio`: SDA引脚号
- `scl_gpio`: SCL引脚号

**返回值**：
- `ESP_OK`: 初始化成功
- `ESP_ERR_INVALID_ARG`: 参数无效
- `ESP_ERR_NO_MEM`: 内存不足

**使用说明**：
在使用PCF8574前必须先调用此函数初始化设备。如果多个设备共享同一I2C总线，驱动会自动复用总线句柄。

---

### 释放函数

```c
/**
 * @brief 释放PCF8574设备描述符
 * 
 * @param dev 指向I2C设备描述符的指针
 * @return ESP_OK 成功，ESP_ERR_INVALID_ARG 参数无效
 */
esp_err_t pcf8574_free_desc(i2c_dev_t *dev);
```

**参数说明**：
- `dev`: 设备描述符指针

**返回值**：
- `ESP_OK`: 释放成功
- `ESP_ERR_INVALID_ARG`: 参数无效

**使用说明**：
不再使用设备时应调用此函数释放资源。如果是最后一个使用该I2C总线的设备，总线也会被自动释放。

---

### 端口读写函数

```c
/**
 * @brief 读取GPIO端口值（8位）
 * 
 * @param dev 指向I2C设备描述符的指针
 * @param val 读取的8位GPIO端口值
 * @return ESP_OK 成功，其他值失败
 */
esp_err_t pcf8574_port_read(i2c_dev_t *dev, uint8_t *val);

/**
 * @brief 写入GPIO端口值（8位）
 * 
 * @param dev 指向I2C设备描述符的指针
 * @param value GPIO端口值
 * @return ESP_OK 成功，其他值失败
 */
esp_err_t pcf8574_port_write(i2c_dev_t *dev, uint8_t value);
```

**参数说明**：
- `dev`: 设备描述符指针
- `val`: 读取的端口值（8位，每位对应一个GPIO）
- `value`: 要写入的端口值

**返回值**：
- `ESP_OK`: 操作成功
- `ESP_ERR_INVALID_ARG`: 参数无效
- `ESP_ERR_INVALID_STATE`: 设备未初始化

**使用说明**：
端口值的每一位对应一个GPIO引脚（bit0=P0, bit1=P1, ..., bit7=P7）。

---

### 单引脚操作函数

```c
/**
 * @brief 读取单个引脚电平
 * 
 * @param dev 指向设备描述符的指针
 * @param pin 引脚编号 (0-7)
 * @param val 引脚电平 (1=高, 0=低)
 * @return ESP_OK 成功，其他值失败
 */
esp_err_t pcf8574_get_level(i2c_dev_t *dev, uint8_t pin, uint32_t *val);

/**
 * @brief 设置单个引脚电平
 * 
 * @param dev 指向设备描述符的指针
 * @param pin 引脚编号 (0-7)
 * @param val 引脚电平 (1=高, 0=低)
 * @return ESP_OK 成功，其他值失败
 */
esp_err_t pcf8574_set_level(i2c_dev_t *dev, uint8_t pin, uint32_t val);
```

**参数说明**：
- `dev`: 设备描述符指针
- `pin`: 引脚编号（0-7对应P0-P7）
- `val`: 引脚电平（0=低电平，1=高电平）

**返回值**：
- `ESP_OK`: 操作成功
- `ESP_ERR_INVALID_ARG`: 参数无效（如引脚号>7）

**使用说明**：
设置单个引脚时会先读取当前端口状态，修改指定位后再写回，因此会产生两次I2C通信。

---

## 使用示例

### 基本使用示例

```c
#include "pcf8574.h"
#include "esp_log.h"

static const char *TAG = "PCF8574_EXAMPLE";

void pcf8574_example(void)
{
    // 1. 定义设备描述符
    i2c_dev_t dev;
    
    // 2. 初始化PCF8574设备
    esp_err_t ret = pcf8574_init_desc(&dev, 0x20, I2C_NUM_0, 
                                      GPIO_NUM_21, GPIO_NUM_22);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "PCF8574初始化失败");
        return;
    }
    ESP_LOGI(TAG, "PCF8574初始化成功");
    
    // 3. 设置所有引脚为高电平（输出模式）
    ret = pcf8574_port_write(&dev, 0xFF);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "端口写入失败");
    }
    
    // 4. 读取端口状态
    uint8_t port_val;
    ret = pcf8574_port_read(&dev, &port_val);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "端口值: 0x%02X", port_val);
    }
    
    // 5. 使用完毕后释放资源
    pcf8574_free_desc(&dev);
}
```

### LED控制示例

```c
#include "pcf8574.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void pcf8574_led_example(void)
{
    i2c_dev_t dev;
    
    // 初始化PCF8574
    pcf8574_init_desc(&dev, 0x20, I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22);
    
    // 流水灯效果
    while (1) {
        for (int i = 0; i < 8; i++) {
            // 点亮第i个LED（假设低电平点亮）
            uint8_t pattern = ~(1 << i);
            pcf8574_port_write(&dev, pattern);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
    }
}
```

### 单引脚操作示例

```c
void pcf8574_pin_example(void)
{
    i2c_dev_t dev;
    pcf8574_init_desc(&dev, 0x20, I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22);
    
    // 设置P0为高电平
    pcf8574_set_level(&dev, 0, 1);
    
    // 读取P1的电平
    uint32_t level;
    pcf8574_get_level(&dev, 1, &level);
    ESP_LOGI(TAG, "P1电平: %d", level);
    
    // 翻转P2的状态
    pcf8574_get_level(&dev, 2, &level);
    pcf8574_set_level(&dev, 2, !level);
}
```

### 多设备示例

```c
void pcf8574_multi_device_example(void)
{
    i2c_dev_t dev1, dev2;
    
    // 初始化两个PCF8574设备（共享同一I2C总线）
    pcf8574_init_desc(&dev1, 0x20, I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22);
    pcf8574_init_desc(&dev2, 0x21, I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22);
    
    // 分别控制两个设备
    pcf8574_port_write(&dev1, 0x55);  // 设备1: 01010101
    pcf8574_port_write(&dev2, 0xAA);  // 设备2: 10101010
    
    // 释放资源（总线会在最后一个设备释放时自动删除）
    pcf8574_free_desc(&dev1);
    pcf8574_free_desc(&dev2);
}
```

---

## 注意事项

### 硬件限制

- ⚠️ **电流限制**：每个引脚最大输出电流25mA，总电流不超过100mA
- ⚠️ **电压范围**：工作电压2.5V-6V，建议使用3.3V
- ⚠️ **上拉电阻**：I2C总线需要上拉电阻，驱动已启用内部上拉
- ⚠️ **地址冲突**：同一I2C总线上的设备地址不能冲突

### 软件限制

- ⚠️ **准双向I/O**：PCF8574使用准双向I/O，输出高电平时可作为输入
- ⚠️ **读取延迟**：I2C通信有一定延迟，不适合高速GPIO操作
- ⚠️ **单引脚写入**：设置单个引脚需要读-改-写操作，非原子性

### 线程安全

- 该驱动不是线程安全的，多线程访问需要添加互斥锁保护
- I2C总线管理使用静态数组，多线程初始化/释放设备需要加锁

### 性能考虑

- I2C通信速度较慢（400kHz），不适合高频GPIO操作
- 单引脚操作比端口操作慢（需要两次I2C通信）
- 建议批量操作时使用端口读写函数

---

## 故障排除

### 常见问题

#### 问题1：初始化失败

**现象**：pcf8574_init_desc()返回错误

**原因**：I2C总线配置错误或设备地址错误

**解决方案**：
1. 检查SDA/SCL引脚连接是否正确
2. 确认I2C地址配置正确（使用I2C扫描工具检测）
3. 检查PCF8574的电源供电是否正常
4. 确认I2C总线上有上拉电阻

#### 问题2：读写操作失败

**现象**：pcf8574_port_read/write()返回错误

**原因**：I2C通信失败或设备无响应

**解决方案**：
1. 使用逻辑分析仪检查I2C波形
2. 确认设备地址正确（A0-A2引脚配置）
3. 检查I2C总线是否被其他设备占用
4. 降低I2C频率尝试（修改I2C_FREQ_HZ）

#### 问题3：GPIO输出无效

**现象**：设置GPIO电平后外设无响应

**原因**：电流不足或接线错误

**解决方案**：
1. 检查负载电流是否超过25mA
2. 确认PCF8574引脚与外设连接正确
3. 使用万用表测量PCF8574引脚电压
4. 检查是否需要外部驱动电路（如继电器、大功率LED）

#### 问题4：多设备冲突

**现象**：多个PCF8574设备工作异常

**原因**：I2C地址冲突

**解决方案**：
1. 确认每个设备的A0-A2配置不同
2. 使用I2C扫描工具检测总线上的设备地址
3. 检查是否有其他I2C设备使用相同地址

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [GPIO引脚分配文档](../../docs/GPIO_PIN_ALLOCATION.md)

### 数据手册

- PCF8574/PCF8574A数据手册
- ESP32-S3技术参考手册 - I2C章节

### 代码示例

- 示例代码：`components/pcf8574/pcf8574.c` - 完整的驱动实现

### 相关组件

- [pcf_buzzer](../pcf_buzzer/README.md) - 使用PCF8574控制蜂鸣器的示例

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，支持ESP-IDF 5.3+新I2C驱动 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/pcf8574/`  
**文档类型**: 组件使用说明
