# 转弯状态机 (Turn State Machine)

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: main.c使用中

---

## 组件简介

转弯状态机是一个六状态有限状态机，用于管理巡线机器人的转弯过程。该模块在Timer 1中断（10ms周期）中执行，通过状态转换控制机器人完成停车、后退、转弯、微调等一系列动作，实现精确的转弯控制。

状态机采用tick计数器机制替代延时函数，确保在中断环境中安全运行，并使用原子操作保护状态变量，支持多线程访问。

### 主要特性

- **六状态设计**：IDLE、STOP、BACK、PHASE1、PHASE2、ADJUST，覆盖完整转弯流程
- **实时控制**：在10ms周期的Timer 1中断中执行，响应迅速
- **Tick计数器**：使用tick计数器替代延时函数，中断安全
- **原子操作**：使用原子变量保护状态，支持多线程访问
- **传感器反馈**：基于灰度传感器实时反馈调整转弯过程
- **自动清除标志**：转弯完成后自动清除转弯标志，恢复巡线控制
- **可配置参数**：支持运行时调整后退时间等参数

### 适用场景

- 巡线机器人的十字路口转弯
- 需要精确转弯控制的自动导航系统
- 基于状态机的复杂运动控制应用

---

## 硬件连接

转弯状态机依赖灰度传感器组件（gray_sensor）、转弯检测器组件（turn_detector）和PWM组件（pwm），不需要额外的硬件连接。

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

### 注意事项

- ⚠️ 必须先初始化灰度传感器组件（gray_sensor）
- ⚠️ 必须先初始化转弯检测器组件（turn_detector）
- ⚠️ 必须先初始化PWM组件（pwm）
- ⚠️ 确保Timer 1中断正常运行（10ms周期）

---

## 功能说明

### 状态机设计

转弯状态机包含六个状态，每个状态负责转弯过程中的特定阶段：


#### 状态1：TURN_IDLE（空闲状态）

**功能**：等待转弯请求

**行为**：
- 检查转弯检测器的转弯类型标志
- 如果检测到十字路口（TURN_TYPE_CROSS），设置转弯方向为右转，进入TURN_STOP状态
- 可选支持左转（TURN_TYPE_LEFT）和右转（TURN_TYPE_RIGHT）检测

**退出条件**：检测到转弯请求

#### 状态2：TURN_STOP（停车状态）

**功能**：停车准备转弯

**行为**：
- 停止所有电机
- 持续100ms（10 ticks）

**退出条件**：tick计数器达到10

#### 状态3：TURN_BACK（后退状态）

**功能**：后退为转弯腾出空间

**行为**：
- 所有电机反转，小车后退
- 默认持续130ms（13 ticks），可配置

**退出条件**：tick计数器达到配置的后退时间

#### 状态4：TURN_PHASE1（转弯第一阶段）

**功能**：转弯直到传感器离开黑线

**行为**：
- 右转：左轮正转，右轮反转，等待左传感器离开黑线（最少250ms）
- 左转：左轮反转，右轮正转，等待右传感器离开黑线（最少150ms）

**退出条件**：
- 传感器ADC值 >= 阈值（离开黑线）
- 且tick计数器达到最小持续时间

#### 状态5：TURN_PHASE2（转弯第二阶段）

**功能**：继续转弯直到传感器找到新黑线

**行为**：
- 右转：继续右转，等待右传感器找到新黑线
- 左转：继续左转，等待左传感器找到新黑线

**退出条件**：传感器ADC值 < 阈值（找到新黑线）

#### 状态6：TURN_ADJUST（微调状态）

**功能**：停车微调，准备恢复巡线

**行为**：
- 停止所有电机
- 持续100ms（10 ticks）
- 清除转弯标志

**退出条件**：tick计数器达到10，回到TURN_IDLE状态

### 状态转换图

```
TURN_IDLE ──[检测到转弯]──> TURN_STOP ──[100ms]──> TURN_BACK ──[130ms]──>
TURN_PHASE1 ──[传感器离开黑线]──> TURN_PHASE2 ──[传感器找到新黑线]──>
TURN_ADJUST ──[100ms]──> TURN_IDLE
```

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| TURN_STOP_TICKS | 10 | 1-255 | 停车时间（10ms × 10 = 100ms） |
| TURN_BACK_TICKS_DEFAULT | 13 | 1-255 | 后退时间（10ms × 13 = 130ms） |
| TURN_ADJUST_TICKS | 10 | 1-255 | 微调停车时间（10ms × 10 = 100ms） |
| TURN_PHASE1_MIN_TICKS_RIGHT | 50 | 1-255 | 右转第一阶段最小时间（10ms × 50 = 500ms） |
| TURN_PHASE1_MIN_TICKS_LEFT | 15 | 1-255 | 左转第一阶段最小时间（10ms × 15 = 150ms） |
| TURN_SPEED | 500 | 0-1023 | 转弯速度（PWM占空比） |
| TURN_BACK_SPEED | 600 | 0-1023 | 后退速度（PWM占空比） |

---

## API接口

### 初始化函数

```c
/**
 * @brief 初始化转弯状态机
 * 
 * 初始化状态变量和内部计数器
 */
void turn_statemachine_init(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
在使用转弯状态机前必须调用此函数。建议在初始化灰度传感器、转弯检测器、PWM和定时器系统后调用。

---

### 中断函数

```c
/**
 * @brief 转弯状态机tick函数（在Timer 1中断中调用）
 * 
 * 执行状态机逻辑
 */
void IRAM_ATTR turn_statemachine_tick(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
- 此函数由Timer 1中断自动调用，用户无需手动调用
- 使用IRAM_ATTR属性，代码存储在IRAM中以提高执行速度
- 执行时间不超过1毫秒

---

### 状态查询函数

```c
/**
 * @brief 获取当前状态（线程安全）
 * 
 * @return 当前状态
 */
TurnState_t turn_statemachine_get_state(void);
```

**参数说明**：
- 无

**返回值**：
- `TURN_IDLE` (0): 空闲状态
- `TURN_STOP` (1): 停车状态
- `TURN_BACK` (2): 后退状态
- `TURN_PHASE1` (3): 转弯第一阶段
- `TURN_PHASE2` (4): 转弯第二阶段
- `TURN_ADJUST` (5): 微调状态

**使用说明**：
使用原子操作读取状态，线程安全，可在任意任务中调用。

---

### 参数设置函数

```c
/**
 * @brief 设置后退时间（单位：tick，1 tick = 10ms）
 * 
 * @param ticks 后退时间（建议：高速20，低速15）
 */
void turn_statemachine_set_back_ticks(uint8_t ticks);
```

**参数说明**：
- `ticks`: 后退时间，单位为tick（1 tick = 10ms）

**返回值**：
- 无

**使用说明**：
可在运行时动态调整后退时间。高速巡线时建议增加后退时间，低速时可减少。

---

```c
/**
 * @brief 获取后退时间
 * 
 * @return 当前后退时间（tick数）
 */
uint8_t turn_statemachine_get_back_ticks(void);
```

**参数说明**：
- 无

**返回值**：
- 当前后退时间（tick数）

**使用说明**：
查询当前后退时间设置。

---

### 调试函数

```c
/**
 * @brief 获取调试信息（用于测试）
 * 
 * @param tick_count 当前状态持续的tick数输出
 * @param turn_dir 转弯方向输出（1=左转，2=右转）
 */
void turn_statemachine_get_debug_info(uint16_t *tick_count, uint8_t *turn_dir);
```

**参数说明**：
- `tick_count`: 输出当前状态持续的tick数（可为NULL）
- `turn_dir`: 输出转弯方向（可为NULL）

**返回值**：
- 无

**使用说明**：
用于调试和测试，获取内部状态信息。

---

## 使用示例

### 基本使用示例

```c
#include "turn_statemachine.h"
#include "turn_detector.h"
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
    
    // 3. 初始化转弯检测器
    turn_detector_init();
    
    // 4. 初始化转弯状态机
    turn_statemachine_init();
    
    // 5. 初始化并启动定时器系统（会自动调用状态机tick）
    timer_system_init();
    timer_system_start();
    
    ESP_LOGI(TAG, "转弯状态机已启动");
    
    // 6. 主循环监控状态
    TurnState_t last_state = TURN_IDLE;
    while (1) {
        TurnState_t current_state = turn_statemachine_get_state();
        
        if (current_state != last_state) {
            ESP_LOGI(TAG, "状态变化: %d -> %d", last_state, current_state);
            last_state = current_state;
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```


### 参数调整示例

```c
#include "turn_statemachine.h"
#include "esp_log.h"

static const char *TAG = "tuning";

void tune_back_time(void)
{
    // 1. 低速巡线，使用较短的后退时间
    ESP_LOGI(TAG, "低速模式：后退时间 = 10 ticks (100ms)");
    turn_statemachine_set_back_ticks(10);
    vTaskDelay(pdMS_TO_TICKS(10000));
    
    // 2. 中速巡线，使用默认后退时间
    ESP_LOGI(TAG, "中速模式：后退时间 = 13 ticks (130ms)");
    turn_statemachine_set_back_ticks(13);
    vTaskDelay(pdMS_TO_TICKS(10000));
    
    // 3. 高速巡线，使用较长的后退时间
    ESP_LOGI(TAG, "高速模式：后退时间 = 20 ticks (200ms)");
    turn_statemachine_set_back_ticks(20);
    vTaskDelay(pdMS_TO_TICKS(10000));
    
    ESP_LOGI(TAG, "参数调整测试完成");
}
```

### 实时监控示例

```c
#include "turn_statemachine.h"
#include "turn_detector.h"
#include "esp_log.h"

static const char *TAG = "monitor";

// 状态名称字符串
const char* state_names[] = {
    "IDLE", "STOP", "BACK", "PHASE1", "PHASE2", "ADJUST"
};

void monitor_task(void *pvParameters)
{
    TurnState_t last_state = TURN_IDLE;
    
    while (1) {
        // 获取当前状态
        TurnState_t current_state = turn_statemachine_get_state();
        
        // 获取调试信息
        uint16_t tick_count;
        uint8_t turn_dir;
        turn_statemachine_get_debug_info(&tick_count, &turn_dir);
        
        // 获取转弯标志
        bool is_turning = turn_detector_is_turning();
        uint8_t turn_type = turn_detector_get_type();
        
        // 检测状态变化
        if (current_state != last_state) {
            ESP_LOGI(TAG, "========== 状态变化 ==========");
            ESP_LOGI(TAG, "%s -> %s", 
                     state_names[last_state], 
                     state_names[current_state]);
            ESP_LOGI(TAG, "Tick计数: %d", tick_count);
            ESP_LOGI(TAG, "转弯方向: %s", 
                     turn_dir == 1 ? "左转" : (turn_dir == 2 ? "右转" : "无"));
            last_state = current_state;
        }
        
        // 定期打印状态
        ESP_LOGI(TAG, "状态: %s, Tick: %d, 方向: %d, 转弯标志: %s, 类型: %d",
                 state_names[current_state],
                 tick_count,
                 turn_dir,
                 is_turning ? "是" : "否",
                 turn_type);
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

### 完整集成示例

```c
#include "turn_statemachine.h"
#include "turn_detector.h"
#include "pd_controller.h"
#include "gray_sensor.h"
#include "pwm.h"
#include "timer_system.h"
#include "esp_log.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "========== 初始化系统 ==========");
    
    // 1. 初始化硬件
    gray_scanner_init();
    pwm_init();
    
    // 2. 初始化控制模块
    turn_detector_init();
    pd_controller_init();
    turn_statemachine_init();
    
    // 3. 初始化定时器系统
    timer_system_init();
    timer_system_start();
    
    // 4. 启用PD控制器
    pd_controller_enable();
    
    ESP_LOGI(TAG, "========== 系统启动完成 ==========");
    
    // 5. 主循环
    while (1) {
        TurnState_t state = turn_statemachine_get_state();
        
        if (state == TURN_IDLE) {
            // 空闲状态，PD控制器正在巡线
            ESP_LOGI(TAG, "巡线中...");
        } else {
            // 转弯状态，状态机接管控制
            ESP_LOGI(TAG, "转弯中，状态: %d", state);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

---

## 注意事项

### 硬件限制

- ⚠️ 依赖灰度传感器组件，必须先初始化gray_sensor
- ⚠️ 依赖转弯检测器组件，必须先初始化turn_detector
- ⚠️ 依赖PWM组件，必须先初始化pwm
- ⚠️ 电机接线必须正确，否则可能导致转弯方向错误

### 软件限制

- ⚠️ 必须在Timer 1中断中调用tick函数，不能在其他地方调用
- ⚠️ 状态转换依赖传感器反馈，传感器故障可能导致状态机卡死
- ⚠️ 后退时间需要根据速度调整，速度越快后退时间越长

### 线程安全

- ✅ 状态变量使用原子操作，线程安全
- ✅ 状态查询函数使用原子操作，可在任意任务中调用
- ⚠️ 内部tick计数器和转弯方向变量仅在中断中访问，调试函数读取时可能不一致

### 性能考虑

- ✅ tick函数执行时间约0.5-1毫秒，满足10ms周期要求
- ✅ 使用IRAM_ATTR属性，代码存储在IRAM中以提高执行速度
- ✅ 使用内联函数优化电机控制性能
- ⚠️ 状态转换频繁时可能影响系统响应

---

## 故障排除

### 常见问题

#### 问题1：状态机不响应转弯请求

**现象**：检测到转弯标志，但状态机仍停留在TURN_IDLE状态

**原因**：
1. 转弯检测器未正确初始化
2. 转弯标志未正确设置
3. Timer 1中断未正常运行

**解决方案**：
1. 检查转弯检测器初始化：`turn_detector_init()`
2. 检查转弯标志：
   ```c
   uint8_t turn_type = turn_detector_get_type();
   ESP_LOGI(TAG, "转弯类型: %d", turn_type);
   ```
3. 检查Timer 1中断是否运行：在tick函数中添加日志

#### 问题2：状态机卡在TURN_PHASE1或TURN_PHASE2

**现象**：状态机进入TURN_PHASE1或TURN_PHASE2后，长时间不退出

**原因**：
1. 传感器阈值设置不正确，无法检测到黑线变化
2. 传感器故障或接线错误
3. 转弯速度过慢，无法完成转弯

**解决方案**：
1. 检查传感器ADC值：
   ```c
   uint16_t left_raw, right_raw;
   gray_scanner_get_cached_values(&left_raw, &right_raw);
   ESP_LOGI(TAG, "ADC值: 左=%d, 右=%d", left_raw, right_raw);
   ```
2. 重新校准传感器阈值
3. 增加转弯速度：修改 `TURN_SPEED` 常量
4. 检查传感器接线和工作状态

#### 问题3：转弯后无法恢复巡线

**现象**：转弯完成后，PD控制器无法恢复巡线

**原因**：
1. 转弯标志未正确清除
2. PD控制器未启用
3. 传感器位置不在黑线上

**解决方案**：
1. 检查转弯标志是否清除：
   ```c
   bool is_turning = turn_detector_is_turning();
   ESP_LOGI(TAG, "转弯标志: %s", is_turning ? "true" : "false");
   ```
2. 检查PD控制器是否启用：
   ```c
   bool is_enabled = pd_controller_is_enabled();
   ESP_LOGI(TAG, "PD控制器: %s", is_enabled ? "启用" : "禁用");
   ```
3. 调整TURN_ADJUST状态的停车时间，给传感器更多时间稳定

#### 问题4：转弯方向错误

**现象**：应该右转时左转，或应该左转时右转

**原因**：
1. 电机接线错误
2. 转弯方向判断逻辑错误
3. 转弯检测器返回错误的转弯类型

**解决方案**：
1. 检查电机接线，确保左右轮正确连接
2. 检查转弯方向：
   ```c
   uint8_t turn_dir;
   turn_statemachine_get_debug_info(NULL, &turn_dir);
   ESP_LOGI(TAG, "转弯方向: %d (1=左转, 2=右转)", turn_dir);
   ```
3. 检查转弯检测器的转弯类型判断逻辑

#### 问题5：后退距离不合适

**现象**：后退距离过长或过短，影响转弯效果

**原因**：
1. 后退时间设置不合理
2. 后退速度与巡线速度不匹配

**解决方案**：
1. 根据巡线速度调整后退时间：
   - 低速（300-500）：10-13 ticks
   - 中速（500-700）：13-17 ticks
   - 高速（700-900）：17-24 ticks
2. 调整后退速度：修改 `TURN_BACK_SPEED` 常量
3. 使用 `turn_statemachine_set_back_ticks()` 动态调整

---

## 参数调整指南

### 调整流程

1. **低速测试**：从速度400开始，使用默认参数
2. **调整后退时间**：观察转弯效果，调整后退时间
3. **调整转弯速度**：根据转弯精度调整转弯速度
4. **调整最小持续时间**：根据转弯角度调整PHASE1最小持续时间
5. **提高速度**：逐步提高巡线速度，重新调整参数

### 参数建议

| 巡线速度 | 后退时间 | 转弯速度 | 说明 |
|---------|---------|---------|------|
| 300-500 | 10-13 ticks | 400-500 | 低速，稳定优先 |
| 500-700 | 13-17 ticks | 500-600 | 中速，平衡性能 |
| 700-900 | 17-24 ticks | 600-700 | 高速，效率优先 |

### 调整技巧

- **后退时间影响转弯起点**：后退时间越长，转弯起点越靠后
- **转弯速度影响转弯精度**：速度越快，转弯越快但精度可能降低
- **最小持续时间防止误判**：确保传感器真正离开黑线，避免误判
- **微调时间影响恢复速度**：微调时间越长，恢复巡线越稳定

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [灰度传感器组件](../gray_sensor/README.md)
- [转弯检测器组件](../turn_detector/README.md)
- [PD控制器组件](../pd_controller/README.md)
- [PWM组件](../pwm/README.md)
- [定时器系统组件](../timer_system/README.md)
- [配置参数指南](../../docs/CONFIGURATION_GUIDE.md)

### 技术文档

- 有限状态机设计原理
- ESP-IDF定时器API文档
- 原子操作和线程安全

### 代码示例

- 测试程序：`main/main_statemachine_test.c`
- 实现总结：`components/turn_statemachine/IMPLEMENTATION_SUMMARY.md`

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，实现六状态转弯控制 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/turn_statemachine/`  
**文档类型**: 组件使用说明
