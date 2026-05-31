# PD控制器 (PD Controller)

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: main.c使用中

---

## 组件简介

PD控制器是一个用于巡线机器人的比例-微分（Proportional-Derivative）控制算法实现。该模块在Timer 1中断（10ms周期）中执行，通过读取灰度传感器数据计算误差，并使用PD算法调整左右轮速度，实现平滑的巡线控制。

PD控制器的核心思想是：
- **比例项（P）**：根据当前误差调整输出，误差越大调整越大
- **微分项（D）**：根据误差变化率调整输出，抑制震荡和超调

### 主要特性

- **实时控制**：在10ms周期的Timer 1中断中执行，响应迅速
- **自动归一化**：使用校准参数自动归一化ADC值，适应不同环境
- **误差死区**：小误差时不调整，避免不必要的抖动
- **输出限制**：自动限制输出范围，防止速度过大或过小
- **转弯暂停**：转弯期间自动暂停PD控制，由状态机接管
- **参数可调**：支持运行时调整速度、Kp、Kd参数
- **使能控制**：支持启用/禁用PD控制器

### 适用场景

- 巡线机器人的直线巡线控制
- 需要平滑跟踪的自动导航系统
- 基于传感器反馈的闭环控制应用

---

## 硬件连接

PD控制器依赖灰度传感器组件（gray_sensor）和PWM组件（pwm），不需要额外的硬件连接。

### 依赖的传感器

| 传感器 | GPIO引脚 | 接口标识 | 说明 |
|--------|---------|---------|------|
| 左灰度传感器 | GPIO1 (ADC1_CH0) | SSA1 | 检测左侧黑线位置 |
| 右灰度传感器 | GPIO2 (ADC1_CH1) | SSA2 | 检测右侧黑线位置 |

### 依赖的执行器

| 执行器 | GPIO引脚 | 接口标识 | 说明 |
|--------|---------|---------|------|
| 电机1（左前） | GPIO42 | SSD1 | 左侧前轮 |
| 电机2（左后） | GPIO41 | SSD2 | 左侧后轮 |
| 电机3（右前） | GPIO40 | SSD3 | 右侧前轮 |
| 电机4（右后） | GPIO39 | SSD4 | 右侧后轮 |

### 控制原理

PD控制器通过以下步骤实现巡线控制：

1. **读取传感器**：读取左右灰度传感器的ADC值
2. **归一化处理**：将ADC值归一化到[0, 1]范围（0=黑线，1=白色）
3. **计算误差**：`error = 510 * (right_norm - left_norm)`
4. **PD计算**：`output = Kp * error + Kd * (error - last_error)`
5. **速度调整**：`left_speed = speed - output`, `right_speed = speed + output`
6. **设置电机**：通过PWM接口设置四个电机速度

### 注意事项

- ⚠️ 必须先初始化灰度传感器组件（gray_sensor）
- ⚠️ 必须先初始化PWM组件（pwm）
- ⚠️ 校准参数基于特定环境，不同环境可能需要重新校准
- ⚠️ 确保Timer 1中断正常运行（10ms周期）

---

## 功能说明

### 核心功能

#### 功能1：ADC值归一化

将原始ADC值（0-4095）归一化到[0, 1]范围，其中0表示黑线，1表示白色。

**归一化公式**：
```
norm = (raw - black) / (white - black)
```

**校准参数**：
- 左传感器：白色=4095，黑线=1269
- 右传感器：白色=4095，黑线=1354

#### 功能2：误差计算

根据左右传感器的归一化值计算误差，误差反映了机器人偏离黑线的程度和方向。

**误差公式**：
```
error = 510 * (right_norm - left_norm)
```

**误差含义**：
- `error > 0`：右传感器更白，机器人偏右，需要向右转
- `error < 0`：左传感器更白，机器人偏左，需要向左转
- `error = 0`：两个传感器相同，机器人在黑线中央

#### 功能3：误差死区

当误差很小时（|error| < 50），认为机器人在直线上，将误差设为0，避免不必要的调整。

**死区阈值**：50（可通过修改 `ERROR_DEADZONE` 调整）

#### 功能4：PD控制算法

使用比例-微分算法计算控制输出。

**PD公式**：
```
output = Kp * error + Kd * (error - last_error)
```

**参数说明**：
- `Kp`（比例系数）：控制响应速度，越大响应越快，但过大会震荡
- `Kd`（微分系数）：控制稳定性，越大越稳定，但过大会反应迟钝
- `error`：当前误差
- `last_error`：上次误差

**默认参数**：
- Kp = 6.0
- Kd = 30.0

#### 功能5：速度调整

根据PD输出调整左右轮速度，实现转向控制。

**速度公式**：
```
left_speed = speed - output
right_speed = speed + output
```

**调整逻辑**：
- 当 `output > 0`（需要向右转）：左轮加速，右轮减速
- 当 `output < 0`（需要向左转）：左轮减速，右轮加速

**速度限制**：
- 输出限制在 `[-speed, +speed]` 范围内
- 车轮速度限制在 `[0, 1023]` 范围内

#### 功能6：转弯暂停机制

在转弯进行中时，自动暂停PD控制，由转弯状态机接管电机控制。

**暂停逻辑**：
1. 在 `pd_controller_tick()` 开始时检查 `turn_detector_is_turning()`
2. 如果正在转弯，立即返回，不执行PD控制
3. 转弯完成后，自动恢复PD控制

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| DEFAULT_SPEED | 700 | 0-1023 | 基础速度（PWM占空比） |
| DEFAULT_KP | 6.0 | 0.0-100.0 | 比例系数 |
| DEFAULT_KD | 30.0 | 0.0-100.0 | 微分系数 |
| ERROR_DEADZONE | 50.0 | 0.0-510.0 | 误差死区阈值 |
| LEFT_WHITE_VALUE | 4095 | 0-4095 | 左传感器白色校准值 |
| LEFT_BLACK_VALUE | 1269 | 0-4095 | 左传感器黑线校准值 |
| RIGHT_WHITE_VALUE | 4095 | 0-4095 | 右传感器白色校准值 |
| RIGHT_BLACK_VALUE | 1354 | 0-4095 | 右传感器黑线校准值 |

---

## API接口

### 初始化函数

```c
/**
 * @brief 初始化PD控制器
 * 
 * 初始化PD参数和状态变量
 */
void pd_controller_init(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
在使用PD控制功能前必须调用此函数。建议在初始化灰度传感器、PWM和定时器系统后调用。

---

### 中断函数

```c
/**
 * @brief PD控制器tick函数（在Timer 1中断中调用）
 * 
 * 执行PD控制算法
 */
void IRAM_ATTR pd_controller_tick(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
- 此函数由Timer 1中断自动调用，用户无需手动调用
- 使用IRAM_ATTR属性，代码存储在IRAM中以提高执行速度
- 执行时间不超过2毫秒

---

### 参数设置函数

```c
/**
 * @brief 设置PD控制参数
 * 
 * @param speed 基础速度（0-1023）
 * @param kp 比例系数
 * @param kd 微分系数
 */
void pd_controller_set_params(uint16_t speed, float kp, float kd);
```

**参数说明**：
- `speed`: 基础速度，范围0-1023，对应PWM占空比
- `kp`: 比例系数，建议范围3.0-10.0
- `kd`: 微分系数，建议范围10.0-50.0

**返回值**：
- 无

**使用说明**：
可在运行时动态调整参数，立即生效。建议先在低速下测试，逐步增加速度。

---

```c
/**
 * @brief 获取当前PD参数
 * 
 * @param speed 速度输出
 * @param kp Kp输出
 * @param kd Kd输出
 */
void pd_controller_get_params(uint16_t *speed, float *kp, float *kd);
```

**参数说明**：
- `speed`: 输出当前速度（可为NULL）
- `kp`: 输出当前Kp（可为NULL）
- `kd`: 输出当前Kd（可为NULL）

**返回值**：
- 无

**使用说明**：
用于查询当前参数设置。

---

### 状态管理函数

```c
/**
 * @brief 获取上次误差值（用于调试）
 * 
 * @return 上次误差值
 */
float pd_controller_get_last_error(void);
```

**参数说明**：
- 无

**返回值**：
- 上次误差值，范围约为[-510, 510]

**使用说明**：
用于调试和监控，查看当前误差状态。

---

```c
/**
 * @brief 重置PD控制器状态
 * 
 * 重置last_error为0
 */
void pd_controller_reset(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
在切换模式或重新开始巡线时调用，清除历史误差。

---

### 使能控制函数

```c
/**
 * @brief 启用PD控制器
 * 
 * 启用后，pd_controller_tick()会正常执行PD控制
 */
void pd_controller_enable(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
启用PD控制器，开始巡线控制。

---

```c
/**
 * @brief 禁用PD控制器
 * 
 * 禁用后，pd_controller_tick()会立即返回，不执行任何控制
 */
void pd_controller_disable(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
禁用PD控制器，停止巡线控制。

---

```c
/**
 * @brief 获取PD控制器使能状态
 * 
 * @return true 已启用，false 已禁用
 */
bool pd_controller_is_enabled(void);
```

**参数说明**：
- 无

**返回值**：
- `true`: 已启用
- `false`: 已禁用

**使用说明**：
查询PD控制器当前状态。

---

## 使用示例

### 基本使用示例

```c
#include "pd_controller.h"
#include "gray_sensor.h"
#include "pwm.h"
#include "timer_system.h"
#include "esp_log.h"

static const char *TAG = "main";

void app_main(void)
{
    // 1. 初始化灰度传感器
    gray_scanner_init();
    
    // 2. 初始化PWM
    pwm_init();
    
    // 3. 初始化PD控制器
    pd_controller_init();
    
    // 4. 初始化并启动定时器系统（会自动调用pd_controller_tick）
    timer_system_init();
    timer_system_start();
    
    // 5. 启用PD控制器
    pd_controller_enable();
    
    ESP_LOGI(TAG, "PD控制器已启动，开始巡线");
    
    // 6. 主循环监控状态
    while (1) {
        float error = pd_controller_get_last_error();
        ESP_LOGI(TAG, "当前误差: %.2f", error);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

### 参数调整示例

```c
#include "pd_controller.h"
#include "esp_log.h"

static const char *TAG = "tuning";

void tune_pd_parameters(void)
{
    // 1. 低速测试（速度400）
    ESP_LOGI(TAG, "测试1: 低速，默认参数");
    pd_controller_set_params(400, 6.0f, 30.0f);
    pd_controller_enable();
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    // 2. 增加Kp，提高响应速度
    ESP_LOGI(TAG, "测试2: 增加Kp");
    pd_controller_set_params(400, 8.0f, 30.0f);
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    // 3. 增加Kd，提高稳定性
    ESP_LOGI(TAG, "测试3: 增加Kd");
    pd_controller_set_params(400, 8.0f, 40.0f);
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    // 4. 提高速度
    ESP_LOGI(TAG, "测试4: 提高速度");
    pd_controller_set_params(700, 8.0f, 40.0f);
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    // 5. 停止测试
    pd_controller_disable();
    ESP_LOGI(TAG, "参数调整测试完成");
}
```

### 实时监控示例

```c
#include "pd_controller.h"
#include "gray_sensor.h"
#include "turn_detector.h"
#include "esp_log.h"

static const char *TAG = "monitor";

void monitor_task(void *pvParameters)
{
    while (1) {
        // 获取PD参数
        uint16_t speed;
        float kp, kd;
        pd_controller_get_params(&speed, &kp, &kd);
        
        // 获取误差
        float error = pd_controller_get_last_error();
        
        // 获取ADC值
        uint16_t left_raw, right_raw;
        gray_scanner_get_cached_values(&left_raw, &right_raw);
        
        // 获取转弯状态
        bool is_turning = turn_detector_is_turning();
        bool is_enabled = pd_controller_is_enabled();
        
        // 打印监控信息
        ESP_LOGI(TAG, "========== PD控制器状态 ==========");
        ESP_LOGI(TAG, "参数: 速度=%d, Kp=%.2f, Kd=%.2f", speed, kp, kd);
        ESP_LOGI(TAG, "误差: %.2f", error);
        ESP_LOGI(TAG, "ADC: 左=%d, 右=%d", left_raw, right_raw);
        ESP_LOGI(TAG, "状态: %s, 转弯=%s", 
                 is_enabled ? "启用" : "禁用",
                 is_turning ? "是" : "否");
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

### 与语音模块集成示例

```c
#include "pd_controller.h"
#include "voice_module.h"
#include "esp_log.h"

static const char *TAG = "voice";

void voice_command_handler(uint8_t command)
{
    switch (command) {
        case 0x01:  // 开始巡线
            ESP_LOGI(TAG, "收到开始巡线命令");
            pd_controller_reset();
            pd_controller_enable();
            break;
            
        case 0x02:  // 停止巡线
            ESP_LOGI(TAG, "收到停止巡线命令");
            pd_controller_disable();
            break;
            
        case 0x03:  // 加速
            ESP_LOGI(TAG, "收到加速命令");
            uint16_t speed;
            pd_controller_get_params(&speed, NULL, NULL);
            pd_controller_set_params(speed + 100, 6.0f, 30.0f);
            break;
            
        case 0x04:  // 减速
            ESP_LOGI(TAG, "收到减速命令");
            pd_controller_get_params(&speed, NULL, NULL);
            pd_controller_set_params(speed - 100, 6.0f, 30.0f);
            break;
    }
}
```

---

## 注意事项

### 硬件限制

- ⚠️ 依赖灰度传感器组件，必须先初始化gray_sensor
- ⚠️ 依赖PWM组件，必须先初始化pwm
- ⚠️ 校准参数基于特定环境，不同光照条件可能需要重新校准
- ⚠️ 电机接线必须正确，否则可能导致反向运动

### 软件限制

- ⚠️ 必须在Timer 1中断中调用tick函数，不能在其他地方调用
- ⚠️ 参数调整需要逐步进行，避免突变导致失控
- ⚠️ 速度过快可能导致传感器响应不及时，建议从低速开始测试

### 线程安全

- ✅ 参数设置函数使用volatile变量，主循环写，中断读，安全
- ✅ 状态查询函数读取volatile变量，线程安全
- ⚠️ 不要在中断中调用参数设置函数

### 性能考虑

- ✅ tick函数执行时间约1-2毫秒，满足10ms周期要求
- ✅ 使用IRAM_ATTR属性，代码存储在IRAM中以提高执行速度
- ✅ 使用内联函数优化性能
- ⚠️ 浮点运算较多，ESP32-S3硬件浮点单元可以高效处理

---

## 故障排除

### 常见问题

#### 问题1：机器人不跟随黑线

**现象**：启用PD控制器后，机器人不跟随黑线，或者偏离黑线

**原因**：
1. 灰度传感器未正确初始化或校准
2. PD参数不合适
3. 速度过快
4. 传感器位置不当

**解决方案**：
1. 检查灰度传感器初始化和ADC值：
   ```c
   uint16_t left_raw, right_raw;
   gray_scanner_get_cached_values(&left_raw, &right_raw);
   ESP_LOGI(TAG, "ADC值: 左=%d, 右=%d", left_raw, right_raw);
   ```
2. 重新校准传感器，更新校准参数
3. 降低速度，从400开始测试
4. 调整传感器位置，确保能够检测到黑线

#### 问题2：机器人震荡或摆动

**现象**：机器人在黑线上左右摆动，无法平稳前进

**原因**：
1. Kp过大，响应过快
2. Kd过小，阻尼不足
3. 速度过快

**解决方案**：
1. 减小Kp，从3.0开始逐步增加
2. 增大Kd，从20.0开始逐步增加
3. 降低速度
4. 调整误差死区，增大死区阈值

#### 问题3：机器人反应迟钝

**现象**：机器人偏离黑线后，反应缓慢，无法及时纠正

**原因**：
1. Kp过小，响应不足
2. Kd过大，阻尼过强
3. 速度过慢

**解决方案**：
1. 增大Kp，从6.0开始逐步增加
2. 减小Kd，从30.0开始逐步减小
3. 适当提高速度
4. 减小误差死区

#### 问题4：转弯时PD控制未暂停

**现象**：转弯时PD控制仍在执行，导致转弯不准确

**原因**：
1. 转弯检测器未正确初始化
2. 转弯标志未正确设置

**解决方案**：
1. 检查转弯检测器初始化：`turn_detector_init()`
2. 检查转弯标志：
   ```c
   bool is_turning = turn_detector_is_turning();
   ESP_LOGI(TAG, "转弯标志: %s", is_turning ? "true" : "false");
   ```
3. 确保转弯状态机正确设置转弯标志

#### 问题5：参数调整无效

**现象**：调用 `pd_controller_set_params()` 后，参数没有生效

**原因**：
1. PD控制器未启用
2. 参数被其他代码覆盖

**解决方案**：
1. 检查PD控制器是否启用：
   ```c
   bool is_enabled = pd_controller_is_enabled();
   ESP_LOGI(TAG, "PD控制器: %s", is_enabled ? "启用" : "禁用");
   ```
2. 使用 `pd_controller_get_params()` 验证参数是否正确设置
3. 检查是否有其他代码修改参数

---

## 参数调整指南

### 调整流程

1. **低速测试**：从速度400开始，使用默认参数（Kp=6.0, Kd=30.0）
2. **调整Kp**：观察响应速度，如果反应慢增大Kp，如果震荡减小Kp
3. **调整Kd**：观察稳定性，如果摆动增大Kd，如果反应迟钝减小Kd
4. **提高速度**：逐步提高速度，每次增加100，重新调整Kp和Kd
5. **优化死区**：根据实际情况调整误差死区阈值

### 参数建议

| 速度范围 | Kp建议 | Kd建议 | 说明 |
|---------|--------|--------|------|
| 300-500 | 4.0-6.0 | 20.0-30.0 | 低速，稳定优先 |
| 500-700 | 6.0-8.0 | 30.0-40.0 | 中速，平衡性能 |
| 700-900 | 8.0-10.0 | 40.0-50.0 | 高速，响应优先 |

### 调整技巧

- **Kp影响响应速度**：Kp越大，响应越快，但过大会震荡
- **Kd影响稳定性**：Kd越大，越稳定，但过大会反应迟钝
- **速度影响整体性能**：速度越快，需要更大的Kp和Kd
- **死区避免抖动**：死区越大，越稳定，但可能影响精度

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [灰度传感器组件](../gray_sensor/README.md)
- [PWM组件](../pwm/README.md)
- [转弯检测器组件](../turn_detector/README.md)
- [定时器系统组件](../timer_system/README.md)
- [配置参数指南](../../docs/CONFIGURATION_GUIDE.md)

### 技术文档

- PID控制理论
- ESP-IDF定时器API文档
- ESP32-S3浮点运算性能

### 代码示例

- 测试程序：`main/main_pd_test.c`
- 实现总结：`components/pd_controller/IMPLEMENTATION_SUMMARY.md`

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，实现PD控制算法 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/pd_controller/`  
**文档类型**: 组件使用说明
