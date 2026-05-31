# MPU6050六轴传感器 (MPU6050 6-Axis IMU)

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: 未使用

---

## 组件简介

MPU6050六轴传感器组件用于测量三轴加速度和三轴角速度，是惯性测量单元(IMU)的核心模块。MPU6050集成了3轴MEMS陀螺仪和3轴MEMS加速度计，通过I2C接口与主控通信。

本组件支持MPU6050和MPU6500两种型号，提供完整的初始化、配置和数据读取功能，包括加速度、角速度和内置温度传感器的读取。

### 主要特性

- 3轴陀螺仪：可配置量程±250/±500/±1000/±2000 dps
- 3轴加速度计：可配置量程±2g/±4g/±8g/±16g
- 内置温度传感器：测量范围-40°C至+85°C
- I2C通信接口：支持标准模式(100kHz)和快速模式(400kHz)
- 可配置采样率：4Hz - 1000Hz
- 数字低通滤波器：可配置截止频率
- 16位ADC：高精度数据输出
- 低功耗设计：支持多种省电模式

### 适用场景

- 姿态检测和控制
- 平衡车、无人机、机器人
- 运动追踪和计步器
- 游戏手柄和VR设备
- 振动监测和倾角测量

---

## 硬件连接

### 引脚分配

| 功能 | GPIO引脚 | I2C信号 | 说明 |
|------|---------|---------|------|
| I2C时钟 | GPIO9 | SCL | I2C时钟线 |
| I2C数据 | GPIO8 | SDA | I2C数据线 |

### 接线说明

1. 将MPU6050的VCC连接到3.3V电源
2. 将MPU6050的GND与ESP32共地
3. 将MPU6050的SCL连接到GPIO9
4. 将MPU6050的SDA连接到GPIO8
5. 在SCL和SDA线上分别连接4.7kΩ上拉电阻到3.3V（部分模块已内置）

### 接线图

```
MPU6050模块       ESP32-S3
  VCC  ----------- 3.3V
  GND  ----------- GND
  SCL  ----------- GPIO9
  SDA  ----------- GPIO8
  
  上拉电阻（4.7kΩ）
  SCL ----/\/\/\---- 3.3V
  SDA ----/\/\/\---- 3.3V
```

### 注意事项

- ⚠️ **电源电压**：MPU6050使用3.3V电源，不要连接5V
- ⚠️ **I2C上拉电阻**：必须在SCL和SDA线上连接上拉电阻
- ⚠️ **I2C地址**：MPU6050的I2C地址为0x68（AD0接地）或0x69（AD0接VCC）
- ⚠️ **传感器方向**：安装时注意传感器的坐标轴方向

---

## 功能说明

### 核心功能

#### 功能1：三轴加速度测量

加速度计测量三个轴向的线性加速度，可用于检测倾角、振动和运动状态。支持四种量程：
- ±2g：高精度，适合静态倾角测量
- ±4g：中等精度，适合一般运动检测
- ±8g：低精度，适合剧烈运动
- ±16g：最低精度，适合冲击检测

#### 功能2：三轴陀螺仪测量

陀螺仪测量三个轴向的角速度，可用于检测旋转和姿态变化。支持四种量程：
- ±250 dps：高精度，适合慢速旋转
- ±500 dps：中等精度，适合一般旋转
- ±1000 dps：低精度，适合快速旋转
- ±2000 dps：最低精度，适合高速旋转

#### 功能3：温度测量

内置温度传感器可测量芯片温度，用于温度补偿或环境监测。测量范围-40°C至+85°C。

#### 功能4：数字低通滤波

可配置的数字低通滤波器用于减少高频噪声，提高数据稳定性。支持多种截止频率：188Hz、98Hz、42Hz、20Hz、10Hz、5Hz。

#### 功能5：采样率配置

可配置采样率从4Hz到1000Hz，平衡数据更新速度和功耗。

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| 陀螺仪量程 | ±2000 dps | ±250/±500/±1000/±2000 dps | 角速度测量范围 |
| 加速度计量程 | ±2g | ±2g/±4g/±8g/±16g | 加速度测量范围 |
| 采样率 | 50Hz | 4-1000Hz | 数据更新频率 |
| 低通滤波器 | 25Hz | 5-188Hz | 数字滤波器截止频率 |
| I2C地址 | 0x68 | 0x68/0x69 | I2C从机地址 |

---

## API接口

### 初始化函数

```c
/**
 * @brief MPU6050初始化函数，配置相关寄存器
 * @return ESP_OK表示成功，其他值表示失败
 */
esp_err_t mpu60xx_init(void);
```

**参数说明**：
- 无

**返回值**：
- `ESP_OK`: 初始化成功
- `ESP_FAIL`: 初始化失败（设备ID不匹配或I2C通信失败）

**使用说明**：
此函数执行完整的MPU6050初始化流程，包括：
1. 初始化I2C接口
2. 复位MPU6050
3. 唤醒传感器
4. 配置陀螺仪量程为±2000 dps
5. 配置加速度计量程为±2g
6. 设置采样率为50Hz
7. 关闭中断和FIFO功能

---

### 配置函数

```c
/**
 * @brief 设置MPU6050陀螺仪传感器满量程范围
 * @param fsr 满量程范围选择：0=±250dps, 1=±500dps, 2=±1000dps, 3=±2000dps
 * @return ESP_OK表示设置成功，其他值表示设置失败
 */
esp_err_t mpu60xx_set_gyro_fsr(uint8_t fsr);
```

**参数说明**：
- `fsr`: 陀螺仪量程选择（0-3）

**返回值**：
- `ESP_OK`: 设置成功
- 其他值: I2C通信失败

---

```c
/**
 * @brief 设置MPU6050加速度传感器满量程范围
 * @param fsr 满量程范围选择：0=±2g, 1=±4g, 2=±8g, 3=±16g
 * @return ESP_OK表示设置成功，其他值表示设置失败
 */
esp_err_t mpu60xx_set_accel_fsr(uint8_t fsr);
```

**参数说明**：
- `fsr`: 加速度计量程选择（0-3）

**返回值**：
- `ESP_OK`: 设置成功
- 其他值: I2C通信失败

---

```c
/**
 * @brief 设置MPU6050的采样率(假定Fs=1KHz)
 * @param rate 采样率范围：4~1000 Hz
 * @return ESP_OK表示设置成功，其他值表示设置失败
 */
esp_err_t mpu60xx_set_rate(uint16_t rate);
```

**参数说明**：
- `rate`: 采样率（4-1000Hz）

**返回值**：
- `ESP_OK`: 设置成功
- 其他值: I2C通信失败

**使用说明**：
此函数会自动设置低通滤波器为采样率的一半，以避免混叠。

---

```c
/**
 * @brief 设置MPU6050的数字低通滤波器
 * @param lpf 数字低通滤波频率，单位Hz
 * @return ESP_OK表示设置成功，其他值表示设置失败
 */
esp_err_t mpu60xx_set_lfp(uint16_t lpf);
```

**参数说明**：
- `lpf`: 低通滤波器截止频率（Hz）

**返回值**：
- `ESP_OK`: 设置成功
- 其他值: I2C通信失败

---

### 数据读取函数

```c
/**
 * @brief 获取MPU6050内部温度值
 * @return 温度值（单位：°C）
 */
float mpu60xx_get_temperature(void);
```

**参数说明**：
- 无

**返回值**：
- 温度值（单位：°C）

**使用说明**：
读取MPU6050内置温度传感器的数据，可用于温度补偿或环境监测。

---

```c
/**
 * @brief 获取MPU6050陀螺仪三轴原始数据
 * @param gx 指向陀螺仪x轴原始数据的指针(带符号)
 * @param gy 指向陀螺仪y轴原始数据的指针(带符号)
 * @param gz 指向陀螺仪z轴原始数据的指针(带符号)
 * @return ESP_OK表示成功，其他值表示失败
 */
esp_err_t mpu60xx_get_gyroscope(short *gx, short *gy, short *gz);
```

**参数说明**：
- `gx`: 指向X轴陀螺仪原始数据的指针
- `gy`: 指向Y轴陀螺仪原始数据的指针
- `gz`: 指向Z轴陀螺仪原始数据的指针

**返回值**：
- `ESP_OK`: 读取成功
- 其他值: I2C通信失败

**使用说明**：
返回的是16位有符号整数原始值，需要根据量程转换为实际角速度：
- ±250 dps: 实际值 = 原始值 / 131.0
- ±500 dps: 实际值 = 原始值 / 65.5
- ±1000 dps: 实际值 = 原始值 / 32.8
- ±2000 dps: 实际值 = 原始值 / 16.4

---

```c
/**
 * @brief 获取MPU6050加速度计三轴原始数据
 * @param ax 指向加速度计x轴原始数据的指针(带符号)
 * @param ay 指向加速度计y轴原始数据的指针(带符号)
 * @param az 指向加速度计z轴原始数据的指针(带符号)
 * @return ESP_OK表示成功，其他值表示失败
 */
esp_err_t mpu60xx_get_accelerometer(short *ax, short *ay, short *az);
```

**参数说明**：
- `ax`: 指向X轴加速度原始数据的指针
- `ay`: 指向Y轴加速度原始数据的指针
- `az`: 指向Z轴加速度原始数据的指针

**返回值**：
- `ESP_OK`: 读取成功
- 其他值: I2C通信失败

**使用说明**：
返回的是16位有符号整数原始值，需要根据量程转换为实际加速度：
- ±2g: 实际值 = 原始值 / 16384.0
- ±4g: 实际值 = 原始值 / 8192.0
- ±8g: 实际值 = 原始值 / 4096.0
- ±16g: 实际值 = 原始值 / 2048.0

---

### 测试函数

```c
/**
 * @brief MPU6050测试函数，读取并打印传感器数据
 */
void mpu60xx_test(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
一次性读取加速度计、陀螺仪和温度数据，并通过日志输出，用于测试和调试。

---

## 使用示例

### 基本使用示例

```c
#include "mpu60xx.h"
#include "esp_log.h"

static const char *TAG = "EXAMPLE";

void example_basic_usage(void)
{
    // 1. 初始化MPU6050
    esp_err_t ret = mpu60xx_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MPU6050初始化失败");
        return;
    }
    
    // 2. 读取加速度计数据
    short ax, ay, az;
    ret = mpu60xx_get_accelerometer(&ax, &ay, &az);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "加速度: ax=%d, ay=%d, az=%d", ax, ay, az);
    }
    
    // 3. 读取陀螺仪数据
    short gx, gy, gz;
    ret = mpu60xx_get_gyroscope(&gx, &gy, &gz);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "陀螺仪: gx=%d, gy=%d, gz=%d", gx, gy, gz);
    }
    
    // 4. 读取温度
    float temp = mpu60xx_get_temperature();
    ESP_LOGI(TAG, "温度: %.2f°C", temp);
}
```

### 数据转换示例

```c
#include "mpu60xx.h"
#include "esp_log.h"

static const char *TAG = "EXAMPLE";

void example_data_conversion(void)
{
    short ax, ay, az;
    short gx, gy, gz;
    
    // 读取原始数据
    mpu60xx_get_accelerometer(&ax, &ay, &az);
    mpu60xx_get_gyroscope(&gx, &gy, &gz);
    
    // 转换加速度数据（量程±2g）
    float accel_x = ax / 16384.0;  // 单位：g
    float accel_y = ay / 16384.0;
    float accel_z = az / 16384.0;
    
    ESP_LOGI(TAG, "加速度: X=%.3fg, Y=%.3fg, Z=%.3fg", 
             accel_x, accel_y, accel_z);
    
    // 转换陀螺仪数据（量程±2000 dps）
    float gyro_x = gx / 16.4;  // 单位：dps (度/秒)
    float gyro_y = gy / 16.4;
    float gyro_z = gz / 16.4;
    
    ESP_LOGI(TAG, "角速度: X=%.2f°/s, Y=%.2f°/s, Z=%.2f°/s", 
             gyro_x, gyro_y, gyro_z);
}
```

### 周期性读取示例

```c
#include "mpu60xx.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "IMU_TASK";

void imu_monitor_task(void *pvParameters)
{
    // 初始化MPU6050
    if (mpu60xx_init() != ESP_OK) {
        ESP_LOGE(TAG, "MPU6050初始化失败");
        vTaskDelete(NULL);
        return;
    }
    
    while (1) {
        // 使用测试函数读取并打印所有数据
        mpu60xx_test();
        
        // 20ms读取一次（50Hz）
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void example_periodic_read(void)
{
    xTaskCreate(imu_monitor_task, "imu_monitor", 4096, NULL, 5, NULL);
}
```

### 姿态角计算示例

```c
#include "mpu60xx.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "ATTITUDE";

void example_attitude_calculation(void)
{
    short ax, ay, az;
    
    // 读取加速度计数据
    if (mpu60xx_get_accelerometer(&ax, &ay, &az) == ESP_OK) {
        // 转换为g单位
        float accel_x = ax / 16384.0;
        float accel_y = ay / 16384.0;
        float accel_z = az / 16384.0;
        
        // 计算俯仰角（pitch）和横滚角（roll）
        float pitch = atan2(accel_y, sqrt(accel_x*accel_x + accel_z*accel_z)) * 180.0 / M_PI;
        float roll = atan2(-accel_x, accel_z) * 180.0 / M_PI;
        
        ESP_LOGI(TAG, "俯仰角: %.2f°, 横滚角: %.2f°", pitch, roll);
    }
}
```

---

## 注意事项

### 硬件限制

- ⚠️ **电源电压**：MPU6050使用3.3V电源，连接5V会损坏芯片
- ⚠️ **I2C上拉电阻**：必须在SCL和SDA线上连接上拉电阻（4.7kΩ）
- ⚠️ **I2C总线速度**：支持100kHz和400kHz，过高速度可能导致通信失败
- ⚠️ **传感器方向**：安装时注意坐标轴方向，影响数据解释

### 软件限制

- ⚠️ **I2C初始化**：组件会自动初始化I2C，不要重复初始化
- ⚠️ **数据转换**：原始数据需要根据量程转换为实际物理量
- ⚠️ **温度漂移**：陀螺仪存在温度漂移，长时间使用需要温度补偿
- ⚠️ **零点偏移**：传感器存在零点偏移，需要校准

### 线程安全

- ❌ MPU6050读取函数不是线程安全的，不应在多个任务中同时调用
- ❌ I2C总线不支持并发访问，需要添加互斥锁保护

### 性能考虑

- 读取一次数据需要约1-2ms（取决于I2C速度）
- 建议读取间隔为10-50ms，平衡数据更新速度和CPU占用
- 如需高频率数据采集，可使用MPU6050的FIFO功能

---

## 故障排除

### 常见问题

#### 问题1：初始化失败（设备ID不匹配）

**现象**：`mpu60xx_init()`返回`ESP_FAIL`，日志显示设备ID不是0x68或0x98

**原因**：
1. MPU6050未正确连接或电源未接通
2. I2C地址错误
3. I2C通信失败
4. 芯片损坏

**解决方案**：
1. 检查MPU6050的VCC、GND、SCL、SDA连接
2. 确认I2C地址（AD0接地为0x68，接VCC为0x69）
3. 检查I2C上拉电阻是否连接
4. 使用I2C扫描工具确认设备地址
5. 更换MPU6050模块测试

#### 问题2：读取数据全为0或异常值

**现象**：读取的加速度或陀螺仪数据全为0或异常大

**原因**：
1. 传感器未正确初始化
2. I2C通信错误
3. 传感器处于睡眠模式
4. 量程设置错误

**解决方案**：
1. 确认`mpu60xx_init()`返回`ESP_OK`
2. 检查I2C通信是否正常
3. 确认电源管理寄存器配置正确
4. 重新设置量程参数

#### 问题3：数据漂移严重

**现象**：陀螺仪数据在静止时持续变化

**原因**：
1. 陀螺仪零点偏移
2. 温度漂移
3. 振动干扰

**解决方案**：
1. 在静止状态下采集多组数据，计算平均值作为零点偏移
2. 读取温度数据，进行温度补偿
3. 减少机械振动
4. 启用数字低通滤波器

#### 问题4：I2C通信超时

**现象**：读取函数返回超时错误

**原因**：
1. I2C上拉电阻缺失或阻值不合适
2. I2C总线速度过高
3. 线缆过长或干扰严重

**解决方案**：
1. 确认SCL和SDA线上有4.7kΩ上拉电阻
2. 降低I2C总线速度到100kHz
3. 缩短I2C线缆长度（建议<20cm）
4. 远离强电磁干扰源

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [GPIO引脚分配文档](../../docs/GPIO_PIN_ALLOCATION.md)
- [I2C组件](../i2c/README.md)
- [板级配置组件](../board_config/README.md)

### 数据手册

- MPU-6050产品规格书
- MPU-6050寄存器映射和描述
- ESP32-S3技术参考手册 - I2C章节

### 代码示例

- 本组件暂未在main.c中使用，可参考本文档的使用示例

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，完整的API文档和使用说明 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/mpu_6050/`  
**文档类型**: 组件使用说明
