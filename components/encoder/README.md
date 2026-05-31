# Encoder 编码器模块

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: 未使用

---

## 组件简介

编码器模块用于读取电机转速和位置信息，支持增量式正交编码器（AB相）。提供两种实现方式：
- **PCNT版本**：使用ESP32-S3硬件脉冲计数器（推荐）
- **GPIO中断版本**：使用软件中断计数（备用方案）

两种实现均支持四倍频模式，提供更高的分辨率和精度。

### 主要特性

- 支持增量式正交编码器（AB相）
- 四倍频模式，提高分辨率4倍
- 硬件PCNT实现，CPU占用极低
- 支持4个独立编码器
- 转速测量（RPM）和角度计算
- GPIO中断备用方案

### 适用场景

适用于需要电机速度和位置反馈的应用场景，如闭环控制、里程计、机器人定位等。

---

## 概述

编码器模块用于读取电机转速和位置信息，支持增量式正交编码器（AB相）。提供两种实现方式：
- **PCNT版本**：使用ESP32-S3硬件脉冲计数器（推荐）
- **GPIO中断版本**：使用软件中断计数（备用方案）

两种实现均支持四倍频模式，提供更高的分辨率和精度。

## 硬件连接

### 引脚定义

| 编码器编号 | A相引脚 | B相引脚 | PCNT单元 | 说明 |
|-----------|---------|---------|----------|------|
| 编码器1 | GPIO41 | GPIO42 | PCNT_0 | 电机1转速检测 |
| 编码器2 | GPIO45 | GPIO46 | PCNT_1 | 电机2转速检测 |
| 编码器3 | GPIO14 | GPIO15 | PCNT_2 | 电机3转速检测 |
| 编码器4 | GPIO16 | GPIO17 | PCNT_3 | 电机4转速检测 |

### 编码器类型

支持标准增量式正交编码器（AB相），常见规格：
- 11线编码器：每转44个脉冲（四倍频）
- 13线编码器：每转52个脉冲（四倍频）
- 26线编码器：每转104个脉冲（四倍频）

## 功能说明

### 四倍频原理

正交编码器输出两路相位差90°的方波信号（A相和B相）。四倍频模式同时检测A相和B相的上升沿和下降沿，每个周期产生4个计数脉冲，大幅提高分辨率。

**计数规则**：
- A相上升沿：B相为低电平时+1，高电平时-1
- A相下降沿：B相为高电平时+1，低电平时-1
- B相上升沿：A相为高电平时+1，低电平时-1
- B相下降沿：A相为低电平时+1，高电平时-1

### PCNT vs GPIO中断

| 特性 | PCNT版本 | GPIO中断版本 |
|------|----------|-------------|
| 实现方式 | 硬件脉冲计数器 | 软件中断计数 |
| CPU占用 | 极低 | 较高 |
| 最高频率 | >40MHz | ~10kHz |
| 精度 | 高 | 中等 |
| 资源占用 | 4个PCNT单元 | GPIO中断 |
| 推荐场景 | 常规使用 | PCNT资源不足时 |

## API 接口

### PCNT版本（推荐）

#### `encoder_new_unit()`
创建并初始化编码器单元。

**参数：**
- `config`: 编码器配置结构体指针
  - `phase_a_gpio_num`: A相GPIO引脚
  - `phase_b_gpio_num`: B相GPIO引脚
  - `low_limit`: PCNT下限值（默认-1000）
  - `high_limit`: PCNT上限值（默认1000）
  - `filter_val`: 滤波阈值（0-1023，默认100）
- `ret_encoder`: 返回编码器句柄

**返回值：**
- `ESP_OK`: 成功
- `ESP_ERR_INVALID_ARG`: 参数错误
- `ESP_ERR_NO_MEM`: 内存不足

#### `encoder_del_unit()`
删除编码器单元，释放资源。

#### `encoder_get_counter()`
获取编码器当前计数值。

**参数：**
- `encoder`: 编码器句柄
- `count`: 返回计数值指针

**返回值：**
- `ESP_OK`: 成功

#### `encoder_clear_count()`
清零编码器计数值。

#### `encoder_measure_speed()`
测量编码器速度（单位：脉冲/秒）。

**参数：**
- `encoder`: 编码器句柄
- `duration_ms`: 测量时间间隔（毫秒）

**返回值：**
- 脉冲数/秒

#### `encoder_get_rpm()`
获取编码器转速（RPM）。

**参数：**
- `encoder`: 编码器句柄
- `duration_ms`: 测量时间间隔（毫秒）
- `pulses_per_revolution`: 每转脉冲数（四倍频后）

**返回值：**
- RPM（转/分钟）

#### `encoder_get_angle()` / `encoder_get_angle_with_params()`
获取编码器角度（度）。

### GPIO中断版本

#### `encoder_gpio_new_unit()`
创建GPIO中断版编码器。

**参数：**
- `phase_a`: A相GPIO引脚
- `phase_b`: B相GPIO引脚
- `ret_enc`: 返回句柄

#### `encoder_gpio_get_count()`
获取当前计数值（有符号，正转为正）。

#### `encoder_gpio_clear_count()`
清零计数值。

#### `encoder_gpio_get_rpm()`
获取转速RPM。

## 使用示例

### 示例1：基本初始化和读取计数

```c
#include "encoder.h"
#include "pin_definitions.h"
#include "esp_log.h"

static const char *TAG = "ENCODER_DEMO";

void app_main(void) {
    // 配置编码器1
    encoder_config_t config = {
        .phase_a_gpio_num = ENCODER1_A_GPIO,  // GPIO41
        .phase_b_gpio_num = ENCODER1_B_GPIO,  // GPIO42
        .low_limit = -10000,
        .high_limit = 10000,
        .filter_val = 100  // 100ns滤波
    };
    
    encoder_unit_handle_t encoder1;
    esp_err_t ret = encoder_new_unit(&config, &encoder1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "编码器初始化失败");
        return;
    }
    
    // 读取计数值
    while (1) {
        int count;
        encoder_get_counter(encoder1, &count);
        ESP_LOGI(TAG, "编码器计数: %d", count);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

### 示例2：测量电机转速（RPM）

```c
#include "encoder.h"
#include "pin_definitions.h"

#define ENCODER_LINES 26          // 编码器线数
#define PULSES_PER_REV (ENCODER_LINES * 4)  // 四倍频：26*4=104

void measure_motor_speed(void) {
    // 初始化编码器
    encoder_config_t config = {
        .phase_a_gpio_num = ENCODER1_A_GPIO,
        .phase_b_gpio_num = ENCODER1_B_GPIO,
        .low_limit = -10000,
        .high_limit = 10000,
        .filter_val = 100
    };
    
    encoder_unit_handle_t encoder1;
    encoder_new_unit(&config, &encoder1);
    
    // 每100ms测量一次转速
    while (1) {
        float rpm = encoder_get_rpm(encoder1, 100, PULSES_PER_REV);
        ESP_LOGI("SPEED", "电机转速: %.2f RPM", rpm);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

### 示例3：四个编码器同时使用

```c
#include "encoder.h"
#include "pin_definitions.h"

encoder_unit_handle_t encoders[4];

void init_all_encoders(void) {
    // 编码器引脚配置
    gpio_num_t encoder_pins[4][2] = {
        {ENCODER1_A_GPIO, ENCODER1_B_GPIO},  // GPIO41, GPIO42
        {ENCODER2_A_GPIO, ENCODER2_B_GPIO},  // GPIO45, GPIO46
        {ENCODER3_A_GPIO, ENCODER3_B_GPIO},  // GPIO14, GPIO15
        {ENCODER4_A_GPIO, ENCODER4_B_GPIO}   // GPIO16, GPIO17
    };
    
    // 初始化4个编码器
    for (int i = 0; i < 4; i++) {
        encoder_config_t config = {
            .phase_a_gpio_num = encoder_pins[i][0],
            .phase_b_gpio_num = encoder_pins[i][1],
            .low_limit = -10000,
            .high_limit = 10000,
            .filter_val = 100
        };
        
        esp_err_t ret = encoder_new_unit(&config, &encoders[i]);
        if (ret != ESP_OK) {
            ESP_LOGE("ENCODER", "编码器%d初始化失败", i+1);
        }
    }
}

void read_all_encoders(void) {
    int counts[4];
    
    for (int i = 0; i < 4; i++) {
        encoder_get_counter(encoders[i], &counts[i]);
    }
    
    ESP_LOGI("ENCODER", "计数值: [%d, %d, %d, %d]", 
             counts[0], counts[1], counts[2], counts[3]);
}
```

### 示例4：使用GPIO中断版本

```c
#include "encoder.h"

void gpio_encoder_demo(void) {
    encoder_gpio_handle_t enc;
    
    // 创建GPIO中断编码器
    esp_err_t ret = encoder_gpio_new_unit(GPIO_NUM_41, GPIO_NUM_42, &enc);
    if (ret != ESP_OK) {
        ESP_LOGE("ENCODER", "GPIO编码器初始化失败");
        return;
    }
    
    // 读取计数
    while (1) {
        int32_t count = encoder_gpio_get_count(enc);
        ESP_LOGI("ENCODER", "GPIO编码器计数: %d", count);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

### 示例5：PID闭环控制配合

```c
#include "encoder.h"
#include "pwm.h"

#define TARGET_RPM 100.0f
#define KP 0.5f

void pid_speed_control(void) {
    encoder_unit_handle_t encoder1;
    encoder_config_t config = {
        .phase_a_gpio_num = ENCODER1_A_GPIO,
        .phase_b_gpio_num = ENCODER1_B_GPIO,
        .low_limit = -10000,
        .high_limit = 10000,
        .filter_val = 100
    };
    encoder_new_unit(&config, &encoder1);
    
    ledc_init();  // 初始化PWM
    
    while (1) {
        // 测量当前转速
        float current_rpm = encoder_get_rpm(encoder1, 100, 104);
        
        // 简单P控制
        float error = TARGET_RPM - current_rpm;
        float output = KP * error;
        
        // 限制输出范围
        if (output > 1023) output = 1023;
        if (output < 0) output = 0;
        
        // 设置PWM
        set_motor1_speed((uint16_t)output, 0);
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

## 依赖

- ESP-IDF driver 组件（PCNT、GPIO驱动）
- FreeRTOS（任务延时、临界区保护）
- board_config 组件（引脚定义）

## 注意事项

1. **PCNT资源限制**: ESP32-S3有4个PCNT单元，最多支持4个编码器
2. **滤波设置**: `filter_val`用于过滤抖动，建议100-1000ns
3. **计数范围**: 设置合理的`low_limit`和`high_limit`，避免溢出
4. **四倍频计算**: 每转脉冲数 = 编码器线数 × 4
5. **GPIO中断版本**: 高速时可能丢脉冲，仅在PCNT不可用时使用
6. **引脚冲突**: 确保编码器引脚未被其他模块占用
7. **测速时间**: `duration_ms`越长，测速精度越高，但响应越慢
8. **方向判断**: 正转计数增加，反转计数减少

## 常见问题

### Q: 如何计算每转脉冲数？
A: 每转脉冲数 = 编码器线数 × 4（四倍频）
   例如：26线编码器 → 104脉冲/转

### Q: 编码器计数不准确？
A: 检查：
   - 编码器供电是否稳定（通常5V）
   - 信号线是否有干扰（建议使用屏蔽线）
   - 滤波值是否合适（增大filter_val）
   - 接线是否正确（A相、B相不能接反）

### Q: 如何判断电机转向？
A: 读取计数值的变化：
   - 计数增加 → 正转
   - 计数减少 → 反转
   - 如果方向相反，交换A相和B相引脚

### Q: PCNT和GPIO中断版本如何选择？
A: 优先使用PCNT版本：
   - 性能更好，CPU占用低
   - 支持更高频率
   - 精度更高
   仅在PCNT资源不足时使用GPIO中断版本

### Q: 如何提高测速精度？
A: 
   - 增加测量时间（duration_ms）
   - 使用更高线数的编码器
   - 多次测量取平均值

## 相关文档

- [引脚定义](../board_config/include/pin_definitions.h)
- [PWM电机控制](../pwm/README.md) - 配合编码器实现闭环控制
- [ESP-IDF PCNT文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/pcnt.html)



---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 添加元信息和版本历史 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/encoder/`  
**文档类型**: 组件使用说明
