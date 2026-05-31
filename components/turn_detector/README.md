# 转弯检测器 (Turn Detector)

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: main.c使用中

---

## 组件简介

转弯检测器是一个基于灰度传感器的转弯标记检测模块，用于在巡线过程中自动识别十字路口和转弯标记。该模块在Timer 0中断（1ms周期）中执行检测逻辑，通过连续确认机制避免误检测，并使用原子变量保护共享数据以确保线程安全。

### 主要特性

- **实时检测**：在1ms周期的Timer 0中断中执行，响应迅速
- **连续确认机制**：需要连续3次（3ms）确认才触发转弯标志，有效避免误检测
- **线程安全**：使用原子变量保护共享数据，支持多线程访问
- **转弯暂停**：转弯进行中自动暂停检测，避免重复触发
- **可扩展设计**：预留左转/右转检测功能（默认禁用）
- **调试支持**：提供调试接口获取内部计数器状态

### 适用场景

- 巡线机器人的十字路口检测
- 自动导航系统的路径标记识别
- 需要实时转弯检测的嵌入式应用

---

## 硬件连接

转弯检测器依赖灰度传感器组件（gray_sensor），不需要额外的硬件连接。

### 依赖的传感器

| 传感器 | GPIO引脚 | 接口标识 | 说明 |
|--------|---------|---------|------|
| 左灰度传感器 | GPIO1 (ADC1_CH0) | SSA1 | 检测左侧黑线 |
| 右灰度传感器 | GPIO2 (ADC1_CH1) | SSA2 | 检测右侧黑线 |

### 检测原理

转弯检测器通过读取左右灰度传感器的ADC值，使用阈值判断传感器是否在黑线上：

- **左传感器阈值**：2785（(4095 + 1476) / 2）
- **右传感器阈值**：2820（(4095 + 1546) / 2）
- **判断逻辑**：ADC值小于阈值表示在黑线上

### 注意事项

- ⚠️ 必须先初始化灰度传感器组件（gray_sensor）
- ⚠️ 阈值基于特定环境校准，不同环境可能需要调整
- ⚠️ 确保Timer 0中断正常运行（1ms周期）

---

## 功能说明

### 核心功能

#### 功能1：十字路口检测

当左右灰度传感器同时检测到黑线，并连续确认3次（3ms）后，标记为十字路口转弯。

**检测逻辑**：
1. 每1ms读取一次灰度传感器的缓存ADC值
2. 判断左右传感器是否同时在黑线上（ADC值 < 阈值）
3. 如果同时在黑线上，计数器加1；否则重置计数器
4. 当计数器达到3次时，设置转弯类型为十字路口（TURN_TYPE_CROSS = 3）
5. 设置转弯进行中标志，暂停后续检测

#### 功能2：转弯暂停机制

在转弯进行中时，自动暂停检测逻辑，避免重复触发转弯标志。

**暂停逻辑**：
1. 检测到转弯后，设置 `g_turning_in_progress = true`
2. 在 `turn_detector_tick()` 中检查此标志
3. 如果标志为true，立即返回并重置所有计数器
4. 转弯完成后，状态机调用 `turn_detector_clear_flags()` 清除标志

#### 功能3：左转/右转检测（可选）

预留了单侧传感器检测功能，可用于识别左转或右转标记。此功能默认禁用，可通过修改代码中的 `#if 0` 为 `#if 1` 启用。

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| LEFT_THRESHOLD | 2785 | 0-4095 | 左传感器黑线阈值 |
| RIGHT_THRESHOLD | 2820 | 0-4095 | 右传感器黑线阈值 |
| CONFIRM_COUNT | 3 | 1-255 | 连续确认次数 |
| TURN_TYPE_NONE | 0 | - | 无转弯 |
| TURN_TYPE_LEFT | 1 | - | 左转（可选功能） |
| TURN_TYPE_RIGHT | 2 | - | 右转（可选功能） |
| TURN_TYPE_CROSS | 3 | - | 十字路口 |

---

## API接口

### 初始化函数

```c
/**
 * @brief 初始化转弯检测模块
 * 
 * 初始化共享变量和计数器
 */
void turn_detector_init(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
在使用转弯检测功能前必须调用此函数。建议在初始化灰度传感器和定时器系统后调用。

---

### 中断函数

```c
/**
 * @brief 转弯检测tick函数（在Timer 0中断中调用）
 * 
 * 执行转弯检测逻辑：
 * - 读取缓存的ADC值
 * - 使用阈值判断左右传感器是否在黑线上
 * - 实现连续确认机制
 * - 更新转弯类型和转弯进行中标志
 */
void IRAM_ATTR turn_detector_tick(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
- 此函数由Timer 0中断自动调用，用户无需手动调用
- 使用IRAM_ATTR属性，代码存储在IRAM中以提高执行速度
- 执行时间不超过200微秒

---

### 状态查询函数

```c
/**
 * @brief 获取转弯类型（线程安全）
 * 
 * @return 转弯类型：0=无，1=左转，2=右转，3=十字路口
 */
uint8_t turn_detector_get_type(void);
```

**参数说明**：
- 无

**返回值**：
- `0` (TURN_TYPE_NONE): 无转弯
- `1` (TURN_TYPE_LEFT): 左转（可选功能）
- `2` (TURN_TYPE_RIGHT): 右转（可选功能）
- `3` (TURN_TYPE_CROSS): 十字路口

**使用说明**：
使用原子操作读取转弯类型，线程安全，可在任意任务中调用。

---

```c
/**
 * @brief 检查是否正在转弯（线程安全）
 * 
 * @return true=正在转弯，false=未转弯
 */
bool turn_detector_is_turning(void);
```

**参数说明**：
- 无

**返回值**：
- `true`: 正在转弯
- `false`: 未转弯

**使用说明**：
使用原子操作读取转弯标志，线程安全，可在任意任务中调用。

---

### 状态控制函数

```c
/**
 * @brief 清除转弯标志（由状态机调用）
 * 
 * 在转弯完成后，状态机调用此函数清除转弯标志
 */
void turn_detector_clear_flags(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
在转弯动作完成后调用，清除转弯类型和转弯进行中标志，恢复检测功能。

---

```c
/**
 * @brief 设置转弯进行中标志（由状态机调用）
 * 
 * @param in_progress true=转弯进行中，false=转弯结束
 */
void turn_detector_set_turning(bool in_progress);
```

**参数说明**：
- `in_progress`: 转弯进行中标志

**返回值**：
- 无

**使用说明**：
由状态机在转弯开始时调用，设置标志以暂停检测。

---

### 调试函数

```c
/**
 * @brief 获取调试信息（用于测试）
 * 
 * @param both_count 双传感器计数器输出
 * @param left_count 左传感器计数器输出
 * @param right_count 右传感器计数器输出
 */
void turn_detector_get_debug_info(uint8_t *both_count, uint8_t *left_count, uint8_t *right_count);
```

**参数说明**：
- `both_count`: 输出双传感器计数器值（可为NULL）
- `left_count`: 输出左传感器计数器值（可为NULL）
- `right_count`: 输出右传感器计数器值（可为NULL）

**返回值**：
- 无

**使用说明**：
用于调试和测试，获取内部计数器状态。

---

## 使用示例

### 基本使用示例

```c
#include "turn_detector.h"
#include "gray_sensor.h"
#include "timer_system.h"
#include "esp_log.h"

static const char *TAG = "main";

void app_main(void)
{
    // 1. 初始化灰度传感器
    gray_scanner_init();
    
    // 2. 初始化转弯检测器
    turn_detector_init();
    
    // 3. 初始化并启动定时器系统（会自动调用turn_detector_tick）
    timer_system_init();
    timer_system_start();
    
    // 4. 在主循环中检查转弯标志
    while (1) {
        if (turn_detector_is_turning()) {
            uint8_t turn_type = turn_detector_get_type();
            
            switch (turn_type) {
                case TURN_TYPE_CROSS:
                    ESP_LOGI(TAG, "检测到十字路口");
                    // 执行十字路口转弯动作...
                    break;
                    
                case TURN_TYPE_LEFT:
                    ESP_LOGI(TAG, "检测到左转标记");
                    // 执行左转动作...
                    break;
                    
                case TURN_TYPE_RIGHT:
                    ESP_LOGI(TAG, "检测到右转标记");
                    // 执行右转动作...
                    break;
            }
            
            // 转弯完成后清除标志
            turn_detector_clear_flags();
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

### 与状态机集成示例

```c
#include "turn_detector.h"
#include "turn_statemachine.h"

void state_machine_task(void *pvParameters)
{
    while (1) {
        // 检查转弯标志
        if (turn_detector_is_turning()) {
            uint8_t turn_type = turn_detector_get_type();
            
            // 通知状态机开始转弯
            turn_statemachine_start_turn(turn_type);
            
            // 等待状态机完成转弯
            while (turn_statemachine_is_turning()) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            
            // 清除转弯标志
            turn_detector_clear_flags();
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

### 调试示例

```c
#include "turn_detector.h"
#include "esp_log.h"

static const char *TAG = "debug";

void debug_turn_detector(void)
{
    uint8_t both_count, left_count, right_count;
    
    // 获取调试信息
    turn_detector_get_debug_info(&both_count, &left_count, &right_count);
    
    ESP_LOGI(TAG, "计数器状态:");
    ESP_LOGI(TAG, "  双传感器: %d", both_count);
    ESP_LOGI(TAG, "  左传感器: %d", left_count);
    ESP_LOGI(TAG, "  右传感器: %d", right_count);
    
    // 获取转弯状态
    bool is_turning = turn_detector_is_turning();
    uint8_t turn_type = turn_detector_get_type();
    
    ESP_LOGI(TAG, "转弯状态: %s", is_turning ? "进行中" : "未转弯");
    ESP_LOGI(TAG, "转弯类型: %d", turn_type);
}
```

---

## 注意事项

### 硬件限制

- ⚠️ 依赖灰度传感器组件，必须先初始化gray_sensor
- ⚠️ 阈值基于特定环境校准，不同光照条件可能需要重新校准
- ⚠️ 传感器位置和间距会影响检测效果

### 软件限制

- ⚠️ 必须在Timer 0中断中调用tick函数，不能在其他地方调用
- ⚠️ 连续确认次数最小为1，建议3-15次以平衡响应速度和抗干扰能力
- ⚠️ 左转/右转检测功能默认禁用，启用前需要充分测试

### 线程安全

- ✅ 所有公共API函数都是线程安全的
- ✅ 使用原子变量保护共享数据（g_turn_type、g_turning_in_progress）
- ✅ 内部计数器仅在中断中访问，无需额外保护
- ⚠️ 调试函数读取的计数器值可能在读取过程中被中断修改

### 性能考虑

- ✅ tick函数执行时间约50微秒，远小于200微秒限制
- ✅ 使用IRAM_ATTR属性，代码存储在IRAM中以提高执行速度
- ✅ 连续确认机制延迟为 CONFIRM_COUNT × 1ms（默认3ms）
- ⚠️ 阈值判断和计数器操作都是简单的整数运算，性能开销极小

---

## 故障排除

### 常见问题

#### 问题1：无法检测到转弯

**现象**：机器人经过十字路口时，转弯检测器没有触发转弯标志

**原因**：
1. 灰度传感器未正确初始化
2. 阈值设置不正确，传感器无法识别黑线
3. Timer 0中断未正常运行
4. 转弯进行中标志未清除

**解决方案**：
1. 检查灰度传感器初始化：`gray_scanner_init()` 是否调用
2. 使用调试函数查看ADC原始值，调整阈值：
   ```c
   uint16_t left_raw, right_raw;
   gray_scanner_get_cached_values(&left_raw, &right_raw);
   ESP_LOGI(TAG, "ADC值: 左=%d, 右=%d", left_raw, right_raw);
   ```
3. 检查Timer 0中断是否运行：在tick函数中添加日志
4. 确保转弯完成后调用 `turn_detector_clear_flags()`

#### 问题2：频繁误触发转弯

**现象**：在直线巡线时，转弯检测器频繁触发转弯标志

**原因**：
1. 连续确认次数太少，抗干扰能力不足
2. 阈值设置不合理，正常巡线时也会触发
3. 传感器位置不当，容易误检测

**解决方案**：
1. 增加连续确认次数：修改 `CONFIRM_COUNT` 为5-15
2. 重新校准阈值：在白线和黑线上分别测量ADC值，取中间值
3. 调整传感器位置，确保只有在十字路口时才会同时检测到黑线

#### 问题3：转弯后无法恢复检测

**现象**：完成一次转弯后，无法再次检测到转弯

**原因**：
1. 转弯完成后未调用 `turn_detector_clear_flags()`
2. 转弯进行中标志未清除

**解决方案**：
1. 在转弯动作完成后，确保调用 `turn_detector_clear_flags()`
2. 使用调试函数检查标志状态：
   ```c
   bool is_turning = turn_detector_is_turning();
   ESP_LOGI(TAG, "转弯标志: %s", is_turning ? "true" : "false");
   ```

#### 问题4：检测延迟过大

**现象**：机器人已经驶过十字路口，转弯检测器才触发

**原因**：
1. 连续确认次数太多，导致延迟过大
2. Timer 0中断周期不正确

**解决方案**：
1. 减少连续确认次数：修改 `CONFIRM_COUNT` 为1-3
2. 检查Timer 0中断周期是否为1ms
3. 调整传感器位置，提前检测到转弯标记

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [灰度传感器组件](../gray_sensor/README.md)
- [定时器系统组件](../timer_system/README.md)
- [转弯状态机组件](../turn_statemachine/README.md)
- [配置参数指南](../../docs/CONFIGURATION_GUIDE.md)

### 技术文档

- ESP-IDF定时器API文档
- ESP32-S3 ADC使用指南
- C11原子操作标准

### 代码示例

- 测试程序：`main/main_turn_detector_test.c`
- 实现总结：`components/turn_detector/IMPLEMENTATION_SUMMARY.md`

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，实现十字路口检测功能 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/turn_detector/`  
**文档类型**: 组件使用说明
