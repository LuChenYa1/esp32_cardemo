# I2C 组件

## 概述

本组件提供I2C设备驱动，包括：
- **mpu_i2c** - MPU6050陀螺仪的硬件I2C驱动
- **vl53l0_i2c** - VL53L0X激光测距传感器的软件I2C驱动

## 文件结构

```
components/i2c/
├── mpu_i2c.c              # MPU6050硬件I2C驱动实现
├── vl53l0_i2c.c           # VL53L0X软件I2C驱动实现
├── include/
│   ├── mpu_i2c.h          # MPU6050 I2C接口
│   └── vl53l0_i2c.h       # VL53L0X I2C接口
├── CMakeLists.txt
└── README.md
```

## MPU6050 硬件I2C驱动

### 使用方法

```c
#include "mpu_i2c.h"

// 初始化MPU6050 I2C
esp_err_t ret = mpu_i2c_init();
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "MPU6050初始化失败");
}

// 写入单个字节
mpu_write_byte(0x6B, 0x00);  // 唤醒MPU6050

// 读取单个字节
uint8_t who_am_i = mpu_read_byte(0x75);

// 批量读取
uint8_t data[6];
mpu_read_buf(0x3B, 6, data);  // 读取加速度数据
```

### 引脚配置

引脚定义来自 `pin_definitions.h`：
- SCL: `I2C_MASTER_SCL_GPIO` (GPIO21)
- SDA: `I2C_MASTER_SDA_GPIO` (GPIO20)
- 地址: `MPU6050_I2C_ADDR` (0x68)
- 频率: `MPU6050_SCL_SPEED` (100kHz)

## VL53L0X 软件I2C驱动

### 使用方法

```c
#include "vl53l0_i2c.h"

// 初始化VL53L0X GPIO
vl53l0_i2c_init();

// 使用软件I2C位操作函数
vl53l0_scl_high();
vl53l0_sda_low();
uint8_t level = vl53l0_sda_read();
```

### 引脚配置

引脚定义来自 `pin_definitions.h`：
- SCL: `VL53L0X_SCL_GPIO` (GPIO40)
- SDA: `VL53L0X_SDA_GPIO` (GPIO39)

### 软件I2C接口

VL53L0X使用软件模拟I2C协议，提供以下位操作函数：
- `vl53l0_scl_high()` / `vl53l0_scl_low()` - 控制时钟线
- `vl53l0_sda_high()` / `vl53l0_sda_low()` - 控制数据线
- `vl53l0_sda_read()` - 读取数据线电平
- `vl53l0_sda_set_input()` / `vl53l0_sda_set_output()` - 切换数据线方向

## 注意事项

1. **引脚冲突**：
   - MPU6050使用硬件I2C（GPIO20/21）
   - VL53L0X使用软件I2C（GPIO39/40）
   - 两者不会冲突，可以同时使用

2. **硬件限制**：
   - GPIO20当前被灰度传感器占用（临时飞线方案）
   - 如果需要使用MPU6050，需要先解决GPIO20冲突

3. **初始化顺序**：
   - 先初始化 `board_config`（注册GPIO）
   - 再初始化具体的I2C设备

## API参考

### MPU6050 API

- `mpu_i2c_init()` - 初始化MPU6050 I2C总线
- `mpu_write_byte(reg, byte)` - 写入单个字节
- `mpu_write_buf(reg, len, buf)` - 批量写入
- `mpu_read_byte(reg)` - 读取单个字节
- `mpu_read_buf(reg, len, buf)` - 批量读取

### VL53L0X API

- `vl53l0_i2c_init()` - 初始化GPIO引脚
- `vl53l0_scl_high()` / `vl53l0_scl_low()` - 控制SCL
- `vl53l0_sda_high()` / `vl53l0_sda_low()` - 控制SDA
- `vl53l0_sda_read()` - 读取SDA电平
- `vl53l0_sda_set_input()` / `vl53l0_sda_set_output()` - 切换SDA方向

## 从旧代码迁移

### 旧的 i2c.h/i2c.c

旧的 `i2c.c` 混合了多个设备的代码，已经拆分：

1. **MPU6050代码** → 保留在 `mpu_i2c.c`
2. **VL53L0代码** → 移到 `vl53l0_i2c.c`
3. **TM1637代码** → 移到 `components/tm1637/`

### 迁移示例

```c
// 旧代码
#include "i2c.h"
mpu_i2c_init();
Distance_IO_Init();
i2c_init();  // TM1637

// 新代码
#include "mpu_i2c.h"
#include "vl53l0_i2c.h"
#include "tm1637.h"

mpu_i2c_init();      // MPU6050
vl53l0_i2c_init();   // VL53L0X
tm1637_init();       // TM1637
```

## 相关文档

- [引脚定义](../board_config/include/pin_definitions.h)
- [GPIO分配文档](../../docs/GPIO_CURRENT_ALLOCATION.md)
- [重构计划](../../REFACTOR_PLAN.md)
