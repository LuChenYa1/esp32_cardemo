# HC-SR04超声波测距传感器 (HC-SR04 Ultrasonic Distance Sensor)

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: 未使用

---

## 组件简介

HC-SR04超声波测距传感器组件用于非接触式距离测量。HC-SR04通过发射超声波并接收回波来测量距离，具有测量精度高、响应速度快、成本低廉等优点。

传感器工作原理：主控发送触发信号后，HC-SR04发射8个40kHz的超声波脉冲，遇到障碍物后反射回来。传感器接收到回波后，ECHO引脚输出高电平，高电平持续时间与距离成正比。

### 主要特性

- 测量范围：2cm - 400cm
- 测量精度：±3mm
- 测量角度：15度锥形范围
- 工作电压：5V（兼容3.3V逻辑）
- 触发信号：10us高电平脉冲
- 回波信号：高电平持续时间与距离成正比
- 超时保护：100ms超时自动返回

### 适用场景

- 智能小车避障
- 机器人距离测量
- 液位检测
- 停车辅助系统
- 自动门控制

---

## 硬件连接

### 引脚分配

| 功能 | GPIO引脚 | 接口标识 | 说明 |
|------|---------|---------|------|
| TRIG触发引脚 | GPIO48 | SSD4 | 输出，发送触发信号 |
| ECHO回响引脚 | GPIO47 | SSA4 | 输入，接收回波信号 |

### 接线说明

1. 将HC-SR04的VCC连接到5V电源
2. 将HC-SR04的GND与ESP32共地
3. 将HC-SR04的TRIG引脚连接到GPIO48
4. 将HC-SR04的ECHO引脚连接到GPIO47

### 接线图

```
HC-SR04模块        ESP32-S3
  VCC  ----------- 5V
  TRIG ----------- GPIO48 (SSD4)
  ECHO ----------- GPIO47 (SSA4)
  GND  ----------- GND
```

### 注意事项

- ⚠️ **电源电压**：HC-SR04需要5V电源，使用3.3V可能导致测量距离缩短或无法工作
- ⚠️ **逻辑电平**：ECHO输出为5V逻辑，但ESP32的GPIO可承受5V输入，无需电平转换
- ⚠️ **测量角度**：超声波为15度锥形范围，障碍物应在正前方
- ⚠️ **表面材质**：软质、吸音材质（如海绵、布料）可能测量不准确

---

## 功能说明

### 核心功能

#### 功能1：超声波测距

HC-SR04通过超声波的发射和接收来测量距离。测量过程：
1. 主控向TRIG引脚发送至少10us的高电平脉冲
2. HC-SR04发射8个40kHz的超声波脉冲
3. 超声波遇到障碍物后反射回来
4. HC-SR04接收到回波后，ECHO引脚输出高电平
5. 高电平持续时间 = 超声波往返时间
6. 距离 = 时间 × 声速 / 2

#### 功能2：距离计算

组件根据ECHO引脚高电平持续时间计算距离：
- 声速约为340m/s = 0.034cm/μs
- 距离(cm) = 时间(μs) × 0.034 / 2
- 除以2是因为超声波往返两次距离

#### 功能3：超时保护

组件实现了100ms超时保护机制，防止程序无限等待：
- 等待ECHO变高超时：100ms
- 等待ECHO变低超时：100ms
- 超时返回-1表示测量失败

#### 功能4：测量范围限制

组件限制测量范围在2-400cm：
- 小于2cm：盲区，无法测量
- 大于400cm：超出范围，返回-1

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| TRIG引脚 | GPIO48 | GPIO0-48 | 触发信号输出引脚 |
| ECHO引脚 | GPIO47 | GPIO0-48 | 回波信号输入引脚 |
| 超时时间 | 100ms | 50-200ms | 等待回波的最大时间 |
| 测量范围 | 2-400cm | 2-400cm | 有效测量距离范围 |
| 声速 | 0.034cm/μs | - | 用于距离计算的声速常数 |

---

## API接口

### 测距函数

```c
/**
 * @brief HC-SR04初始化及测量任务
 * @return 返回测量的距离值（单位：cm），测量失败返回-1
 */
float hc_sr04_task(void);
```

**参数说明**：
- 无

**返回值**：
- 成功：返回距离值（单位：cm），范围2-400cm
- 失败：返回-1（超时或超出测量范围）

**使用说明**：
此函数会自动初始化GPIO引脚并执行一次测量。每次调用都会重新配置GPIO，因此可以随时调用。测量过程会阻塞约几毫秒到100ms（取决于距离和是否超时）。

---

## 使用示例

### 基本使用示例

```c
#include "hc_sr04.h"
#include "esp_log.h"

static const char *TAG = "EXAMPLE";

void example_basic_usage(void)
{
    // 执行一次测距
    float distance = hc_sr04_task();
    
    if (distance > 0) {
        ESP_LOGI(TAG, "测量距离: %.2f cm", distance);
    } else {
        ESP_LOGE(TAG, "测量失败（超时或超出范围）");
    }
}
```

### 周期性测距示例

```c
#include "hc_sr04.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "DISTANCE_MONITOR";

void distance_monitor_task(void *pvParameters)
{
    while (1) {
        // 测量距离
        float distance = hc_sr04_task();
        
        if (distance > 0) {
            ESP_LOGI(TAG, "距离: %.2f cm", distance);
            
            // 根据距离执行控制逻辑
            if (distance < 10.0) {
                ESP_LOGW(TAG, "障碍物非常近，紧急停止");
            } else if (distance < 30.0) {
                ESP_LOGW(TAG, "障碍物较近，减速");
            } else {
                ESP_LOGI(TAG, "距离安全，正常行驶");
            }
        } else {
            ESP_LOGW(TAG, "测量失败");
        }
        
        // 建议测量间隔至少60ms（HC-SR04建议周期）
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void example_periodic_measurement(void)
{
    xTaskCreate(distance_monitor_task, "distance_monitor", 2048, NULL, 5, NULL);
}
```

### 避障控制示例

```c
#include "hc_sr04.h"
#include "pwm.h"  // 假设有PWM控制电机
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "OBSTACLE_AVOIDANCE";

#define SAFE_DISTANCE_CM  30.0   // 安全距离阈值
#define STOP_DISTANCE_CM  10.0   // 停止距离阈值

void obstacle_avoidance_task(void *pvParameters)
{
    while (1) {
        float distance = hc_sr04_task();
        
        if (distance > 0) {
            if (distance < STOP_DISTANCE_CM) {
                // 距离过近，停止
                ESP_LOGW(TAG, "距离%.2fcm，紧急停止", distance);
                // pwm_set_duty(MOTOR_LEFT, 0);
                // pwm_set_duty(MOTOR_RIGHT, 0);
            } else if (distance < SAFE_DISTANCE_CM) {
                // 距离较近，减速
                ESP_LOGW(TAG, "距离%.2fcm，减速行驶", distance);
                // pwm_set_duty(MOTOR_LEFT, 30);
                // pwm_set_duty(MOTOR_RIGHT, 30);
            } else {
                // 距离安全，正常速度
                ESP_LOGI(TAG, "距离%.2fcm，正常行驶", distance);
                // pwm_set_duty(MOTOR_LEFT, 50);
                // pwm_set_duty(MOTOR_RIGHT, 50);
            }
        } else {
            // 测量失败，保守处理：停止
            ESP_LOGE(TAG, "测量失败，停止");
            // pwm_set_duty(MOTOR_LEFT, 0);
            // pwm_set_duty(MOTOR_RIGHT, 0);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void example_obstacle_avoidance(void)
{
    xTaskCreate(obstacle_avoidance_task, "obstacle_avoid", 2048, NULL, 5, NULL);
}
```

### 多次测量取平均值

```c
#include "hc_sr04.h"
#include "esp_log.h"

static const char *TAG = "EXAMPLE";

float measure_distance_average(int samples)
{
    float sum = 0;
    int valid_count = 0;
    
    for (int i = 0; i < samples; i++) {
        float distance = hc_sr04_task();
        
        if (distance > 0) {
            sum += distance;
            valid_count++;
        }
        
        // 每次测量间隔60ms
        vTaskDelay(pdMS_TO_TICKS(60));
    }
    
    if (valid_count > 0) {
        return sum / valid_count;
    } else {
        return -1;  // 所有测量都失败
    }
}

void example_average_measurement(void)
{
    // 测量5次取平均值
    float avg_distance = measure_distance_average(5);
    
    if (avg_distance > 0) {
        ESP_LOGI(TAG, "平均距离: %.2f cm", avg_distance);
    } else {
        ESP_LOGE(TAG, "测量失败");
    }
}
```

---

## 注意事项

### 硬件限制

- ⚠️ **电源要求**：HC-SR04需要5V电源，3.3V可能导致测量距离缩短
- ⚠️ **测量盲区**：小于2cm的距离无法测量
- ⚠️ **测量角度**：超声波为15度锥形，障碍物应在正前方
- ⚠️ **表面材质**：软质、吸音材质测量不准确，光滑平面反射效果最好

### 软件限制

- ⚠️ **阻塞执行**：测量函数会阻塞执行，不适合在中断或高优先级任务中调用
- ⚠️ **测量周期**：建议测量间隔至少60ms，过快可能导致回波干扰
- ⚠️ **超时时间**：最大超时100ms，测量远距离时可能需要较长时间

### 线程安全

- ❌ `hc_sr04_task()`不是线程安全的，不应在多个任务中同时调用
- ❌ 每次调用都会重新配置GPIO，可能与其他使用相同GPIO的组件冲突

### 性能考虑

- 测量一次距离需要几毫秒到100ms（取决于距离）
- 建议测量间隔为100ms，平衡响应速度和系统负载

---

## 故障排除

### 常见问题

#### 问题1：始终返回-1（测量失败）

**现象**：调用`hc_sr04_task()`始终返回-1

**原因**：
1. HC-SR04未正确连接或电源未接通
2. 电源电压不足（低于4.5V）
3. TRIG或ECHO引脚接线错误
4. 前方无障碍物或障碍物超出测量范围
5. HC-SR04损坏

**解决方案**：
1. 检查HC-SR04的VCC、GND、TRIG、ECHO连接
2. 使用万用表测量VCC电压，应为5V
3. 确认TRIG连接到GPIO48，ECHO连接到GPIO47
4. 在传感器前方10-50cm处放置障碍物测试
5. 更换HC-SR04传感器测试

#### 问题2：测量值不稳定，频繁跳变

**现象**：连续测量时，距离值在较大范围内跳变

**原因**：
1. 障碍物表面不平整或材质吸音
2. 障碍物不在传感器正前方（超出15度角）
3. 环境中有其他超声波干扰
4. 电源不稳定

**解决方案**：
1. 使用平整、光滑的障碍物（如墙壁、纸板）
2. 确保障碍物在传感器正前方
3. 远离其他超声波设备
4. 在应用层添加滤波算法（如中值滤波、移动平均）
5. 在HC-SR04的VCC和GND之间添加100uF电容稳定电源

#### 问题3：测量距离明显偏小或偏大

**现象**：测量值与实际距离相差较大（超过±3mm）

**原因**：
1. 声速常数不准确（温度影响）
2. 障碍物倾斜导致超声波斜向反射
3. 传感器老化

**解决方案**：
1. 根据环境温度调整声速常数（声速 = 331.5 + 0.6×温度(°C) m/s）
2. 确保障碍物表面垂直于传感器
3. 使用已知距离的障碍物校准传感器
4. 更换新的HC-SR04传感器

#### 问题4：小于10cm的距离测量不准

**现象**：障碍物距离小于10cm时，测量值不准确或返回-1

**原因**：
1. 进入测量盲区（<2cm）
2. 超声波发射和接收时间重叠
3. 近距离反射信号过强

**解决方案**：
1. HC-SR04的有效测量范围为2-400cm，小于2cm无法测量
2. 如需测量更近的距离，建议使用红外测距或激光测距传感器
3. 在应用层设置最小距离阈值（如5cm）

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [GPIO引脚分配文档](../../docs/GPIO_PIN_ALLOCATION.md)
- [板级配置组件](../board_config/README.md)

### 数据手册

- HC-SR04超声波测距模块数据手册
- ESP32-S3技术参考手册 - GPIO章节

### 代码示例

- 本组件暂未在main.c中使用，可参考本文档的使用示例

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，完整的API文档和使用说明 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/hc-sr04/`  
**文档类型**: 组件使用说明
