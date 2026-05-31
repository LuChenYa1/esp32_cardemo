# 红外避障传感器 (IR Obstacle Sensor)

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: main.c使用中

---

## 组件简介

红外避障传感器组件用于检测前方是否有障碍物，是智能小车避障功能的核心模块。本组件通过GPIO数字输入读取红外传感器的状态，提供简单可靠的障碍物检测功能。

传感器工作原理：发射红外光，当前方有障碍物时，红外光被反射回来，传感器输出高电平（1）；无障碍物时输出低电平（0）。

### 主要特性

- 数字信号输出，无需ADC转换
- GPIO输入模式，启用上拉电阻
- 简单的API接口，易于使用
- 与GPIO管理器集成，自动检测引脚冲突
- 实时状态读取，无延迟
- 调试打印功能

### 适用场景

- 智能小车避障
- 机器人前方障碍物检测
- 自动停车系统
- 接近检测应用

---

## 硬件连接

### 引脚分配

| 功能 | GPIO引脚 | 接口标识 | 说明 |
|------|---------|---------|------|
| 红外避障传感器 | GPIO38 | SSA1 | 数字输入，1=有障碍物，0=无障碍物 |

### 接线说明

1. 将红外避障传感器的信号输出（OUT）连接到GPIO38
2. 将传感器的VCC连接到3.3V或5V电源（根据传感器规格）
3. 将传感器的GND与ESP32共地

### 注意事项

- ⚠️ **电压兼容性**：确认传感器输出电平与ESP32兼容（3.3V逻辑），如果传感器输出5V需要加电平转换电路
- ⚠️ **检测距离**：红外避障传感器的检测距离通常为2-30cm，可通过传感器上的电位器调节
- ⚠️ **环境光影响**：强烈的环境光（如阳光直射）可能影响检测精度
- ⚠️ **反射面材质**：黑色或吸光材质的障碍物可能检测不到，白色或反光材质检测效果最好

---

## 功能说明

### 核心功能

#### 功能1：障碍物检测

传感器通过发射红外光并接收反射信号来检测障碍物。当检测到障碍物时，传感器输出高电平（1），组件读取GPIO状态并返回检测结果。

#### 功能2：GPIO管理器集成

组件初始化时会向GPIO管理器注册引脚，自动检测是否与其他模块冲突。如果引脚已被占用，初始化会失败并记录错误日志。

#### 功能3：实时状态读取

提供非阻塞的状态读取函数，可随时查询传感器当前状态，适用于实时控制系统。

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| GPIO引脚 | GPIO38 | GPIO0-48 | 传感器信号输入引脚，定义在pin_definitions.h |
| 上拉电阻 | 启用 | 启用/禁用 | 内部上拉电阻，确保悬空时为高电平 |
| 检测距离 | 可调 | 2-30cm | 通过传感器上的电位器调节 |

---

## API接口

### 初始化函数

```c
/**
 * @brief 初始化红外避障传感器
 * 
 * 配置GPIO38（SSA1）作为输入引脚，启用上拉电阻
 * 传感器输出：1=需要避障，0=无障碍
 */
void ir_obstacle_init(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
在使用传感器前必须先调用此函数进行初始化。初始化会向GPIO管理器注册引脚，如果引脚冲突会记录错误日志。

---

### 状态读取函数

```c
/**
 * @brief 读取红外避障传感器状态
 * 
 * @return ir_obstacle_state_t 传感器状态
 *         - IR_OBSTACLE_CLEAR (0): 无障碍物
 *         - IR_OBSTACLE_DETECTED (1): 检测到障碍物
 */
ir_obstacle_state_t ir_obstacle_read(void);
```

**参数说明**：
- 无

**返回值**：
- `IR_OBSTACLE_CLEAR (0)`: 无障碍物，可以通行
- `IR_OBSTACLE_DETECTED (1)`: 检测到障碍物，需要避障

**使用说明**：
此函数直接读取GPIO电平，非阻塞，可在任何时候调用。

---

```c
/**
 * @brief 检测是否有障碍物
 * 
 * @return true 检测到障碍物（需要避障）
 *         false 无障碍物（可以通行）
 */
bool ir_obstacle_is_detected(void);
```

**参数说明**：
- 无

**返回值**：
- `true`: 检测到障碍物，需要避障
- `false`: 无障碍物，可以通行

**使用说明**：
这是`ir_obstacle_read()`的便捷封装，返回布尔值更直观。

---

### 调试函数

```c
/**
 * @brief 打印传感器状态（调试用）
 */
void ir_obstacle_print_status(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
通过串口打印当前传感器状态，用于调试和验证传感器工作是否正常。

---

## 使用示例

### 基本使用示例

```c
#include "ir_obstacle.h"
#include "esp_log.h"

static const char *TAG = "EXAMPLE";

void example_basic_usage(void)
{
    // 1. 初始化红外避障传感器
    ir_obstacle_init();
    
    // 2. 读取传感器状态（方法1：使用枚举）
    ir_obstacle_state_t state = ir_obstacle_read();
    if (state == IR_OBSTACLE_DETECTED) {
        ESP_LOGI(TAG, "检测到障碍物，需要避障");
    } else {
        ESP_LOGI(TAG, "无障碍物，可以通行");
    }
    
    // 3. 读取传感器状态（方法2：使用布尔值）
    if (ir_obstacle_is_detected()) {
        ESP_LOGI(TAG, "有障碍物");
    } else {
        ESP_LOGI(TAG, "无障碍物");
    }
    
    // 4. 打印状态（调试用）
    ir_obstacle_print_status();
}
```

### 避障控制示例

```c
#include "ir_obstacle.h"
#include "pwm.h"  // 假设有PWM控制电机
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "OBSTACLE_AVOIDANCE";

void obstacle_avoidance_task(void *pvParameters)
{
    // 初始化传感器
    ir_obstacle_init();
    
    while (1) {
        // 检测障碍物
        if (ir_obstacle_is_detected()) {
            // 检测到障碍物，停止前进
            ESP_LOGW(TAG, "检测到障碍物，停止");
            // pwm_set_duty(MOTOR_LEFT, 0);
            // pwm_set_duty(MOTOR_RIGHT, 0);
            
            // 可以添加后退或转向逻辑
            vTaskDelay(pdMS_TO_TICKS(500));
        } else {
            // 无障碍物，继续前进
            ESP_LOGI(TAG, "无障碍物，前进");
            // pwm_set_duty(MOTOR_LEFT, 50);
            // pwm_set_duty(MOTOR_RIGHT, 50);
        }
        
        // 100ms检测一次
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void example_obstacle_avoidance(void)
{
    xTaskCreate(obstacle_avoidance_task, "obstacle_avoid", 2048, NULL, 5, NULL);
}
```

### 多传感器融合示例

```c
#include "ir_obstacle.h"
#include "gray_sensor.h"
#include "esp_log.h"

static const char *TAG = "SENSOR_FUSION";

void example_sensor_fusion(void)
{
    // 初始化传感器
    ir_obstacle_init();
    gray_sensor_init_simple();
    
    // 读取多个传感器
    bool has_obstacle = ir_obstacle_is_detected();
    
    uint16_t left_gray, right_gray;
    gray_sensor_read_both_raw_direct(&left_gray, &right_gray);
    
    // 综合判断
    if (has_obstacle) {
        ESP_LOGW(TAG, "前方有障碍物，停止");
    } else if (left_gray < 1500 || right_gray < 1500) {
        ESP_LOGI(TAG, "检测到黑线，循迹模式");
    } else {
        ESP_LOGI(TAG, "正常行驶");
    }
}
```

---

## 注意事项

### 硬件限制

- ⚠️ **GPIO38限制**：GPIO38是仅输入引脚，不能配置为输出模式
- ⚠️ **电平兼容性**：ESP32的GPIO输入电平为3.3V，如果传感器输出5V需要电平转换
- ⚠️ **检测距离限制**：红外传感器的检测距离有限（通常2-30cm），超出范围无法检测

### 软件限制

- ⚠️ **无中断支持**：当前实现不支持中断模式，需要轮询读取状态
- ⚠️ **无滤波处理**：直接读取GPIO电平，可能受到电磁干扰影响，建议在应用层添加软件滤波

### 线程安全

- ✅ `ir_obstacle_read()`和`ir_obstacle_is_detected()`是线程安全的，可在多个任务中调用
- ✅ `ir_obstacle_init()`应在初始化阶段单线程调用

### 性能考虑

- GPIO读取速度非常快（微秒级），可以高频轮询
- 建议轮询间隔为10-100ms，平衡响应速度和CPU占用

---

## 故障排除

### 常见问题

#### 问题1：传感器始终输出高电平（始终检测到障碍物）

**现象**：调用`ir_obstacle_is_detected()`始终返回true，即使前方无障碍物

**原因**：
1. 传感器未正确连接或电源未接通
2. 传感器灵敏度设置过高
3. 传感器损坏

**解决方案**：
1. 检查传感器的VCC、GND和信号线连接
2. 调整传感器上的灵敏度电位器（顺时针降低灵敏度）
3. 使用万用表测量传感器输出电压，应在0-3.3V范围内
4. 更换传感器测试

#### 问题2：传感器始终输出低电平（始终无障碍物）

**现象**：调用`ir_obstacle_is_detected()`始终返回false，即使前方有障碍物

**原因**：
1. 传感器灵敏度设置过低
2. 障碍物材质吸光（黑色）
3. 检测距离超出传感器范围
4. 传感器发射管或接收管损坏

**解决方案**：
1. 调整传感器上的灵敏度电位器（逆时针提高灵敏度）
2. 使用白色或反光材质的障碍物测试
3. 将障碍物移近传感器（5-10cm）
4. 检查传感器LED是否发光，不发光则传感器损坏

#### 问题3：GPIO注册失败

**现象**：初始化时日志显示"GPIO38注册失败，可能与其他模块冲突"

**原因**：GPIO38已被其他组件占用

**解决方案**：
1. 检查`pin_definitions.h`中的引脚分配
2. 查看GPIO管理器日志，确认哪个组件占用了GPIO38
3. 修改引脚定义，使用其他可用的GPIO
4. 或者禁用冲突的组件

#### 问题4：检测不稳定，频繁跳变

**现象**：传感器状态在有障碍物和无障碍物之间频繁切换

**原因**：
1. 传感器灵敏度设置在临界点
2. 障碍物距离在检测范围边缘
3. 电磁干扰或电源不稳定

**解决方案**：
1. 调整传感器灵敏度，避开临界点
2. 在应用层添加软件滤波（如连续检测3次才判定状态改变）
3. 检查电源质量，添加滤波电容
4. 远离强电磁干扰源

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [GPIO引脚分配文档](../../docs/GPIO_PIN_ALLOCATION.md)
- [板级配置组件](../board_config/README.md)
- [GPIO管理器](../board_config/README.md#gpio管理器)

### 数据手册

- ESP32-S3技术参考手册 - GPIO章节
- 红外避障传感器模块数据手册（具体型号根据实际硬件）

### 代码示例

- 测试程序：`main/main.c` - 包含红外避障传感器的使用示例

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，完整的API文档和使用说明 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/ir_obstacle/`  
**文档类型**: 组件使用说明
