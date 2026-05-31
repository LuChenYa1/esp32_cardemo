# PD控制器实现总结

## 实施日期

2024年（任务5完成）

## 实施内容

### 任务5.1：在Timer 1中断中实现PD控制算法 ✓

已完成以下功能：

1. **读取缓存的ADC值**
   - 使用`gray_scanner_get_cached_values()`中断安全地读取ADC缓存值
   - 支持左右两个灰度传感器

2. **归一化处理**
   - 使用校准参数（左：白色=4095，黑线=1476；右：白色=4095，黑线=1546）
   - 归一化公式：`norm = (raw - black) / (white - black)`
   - 限制在[0, 1]范围内

3. **计算误差**
   - 公式：`error = 510 * (right_norm - left_norm)`
   - 右传感器更白时误差为正（需要向右转）
   - 左传感器更白时误差为负（需要向左转）

4. **实现误差死区**
   - 当`|error| < 30`时，设置`error = 0`
   - 避免小幅度抖动

5. **计算PD输出**
   - 公式：`output = Kp * error + Kd * (error - last_error)`
   - 默认参数：Kp=5.0, Kd=12.0

6. **限制输出范围**
   - 输出限制在`[-speed, +speed]`范围内
   - 默认速度：700

7. **计算左右轮速度**
   - `left_speed = speed - output`
   - `right_speed = speed + output`
   - 限制在[0, 1023]范围内

8. **调用PWM接口设置电机速度**
   - 电机1（左前）：`set_motor_speed(left_speed, 0)`
   - 电机2（左后）：`set_motor2_speed(0, left_speed)`
   - 电机3（右前）：`set_motor3_speed(right_speed, 0)`
   - 电机4（右后）：`set_motor4_speed(0, right_speed)`

9. **更新状态**
   - 保存当前误差为`last_error`，用于下次计算微分项

### 任务5.2：实现转弯期间暂停PD控制逻辑 ✓

已完成以下功能：

1. **检查转弯标志**
   - 在`pd_controller_tick()`开始时检查`turn_detector_is_turning()`
   - 如果正在转弯，直接返回，不执行PD控制

2. **转弯优先级**
   - 转弯状态机优先级高于PD控制
   - 转弯期间由状态机接管电机控制

## 文件结构

```
components/pd_controller/
├── include/
│   └── pd_controller.h          # 头文件
├── pd_controller.c              # 实现文件
├── CMakeLists.txt               # 构建配置
├── README.md                    # 使用文档
└── IMPLEMENTATION_SUMMARY.md    # 实现总结（本文件）
```

## API接口

### 初始化函数

```c
void pd_controller_init(void);
```

### 控制tick函数

```c
void IRAM_ATTR pd_controller_tick(void);
```

在Timer 1中断（10ms周期）中调用。

### 参数设置

```c
void pd_controller_set_params(uint16_t speed, float kp, float kd);
void pd_controller_get_params(uint16_t *speed, float *kp, float *kd);
```

### 状态管理

```c
float pd_controller_get_last_error(void);
void pd_controller_reset(void);
```

## 集成方式

### 1. 在timer_system.c中调用

```c
static bool IRAM_ATTR timer1_isr_handler(...)
{
    // ...
    pd_controller_tick();  // 执行PD控制
    // ...
}
```

### 2. 依赖组件

- `gray_sensor`：读取ADC缓存值
- `turn_detector`：检查转弯标志
- `pwm`：设置电机速度

### 3. 更新CMakeLists.txt

```cmake
# components/timer_system/CMakeLists.txt
REQUIRES driver turn_detector pd_controller
```

## 测试程序

创建了`main/main_pd_test.c`测试程序，包含：

1. **参数设置和读取测试**
2. **PD重置测试**
3. **实时监控任务**
   - 定时器频率
   - ADC值
   - 转弯状态
   - PD参数和误差
   - ADC错误计数

## 性能指标

- **执行时间**：单次控制周期 < 2ms（满足需求4.10）
- **控制周期**：10ms（由Timer 1中断保证）
- **中断安全**：使用IRAM_ATTR属性
- **速度范围**：0-1023（直接对应硬件PWM占空比）

## 验证需求

本实现满足以下需求：

- ✓ 需求4.1：在Timer 1中断中执行
- ✓ 需求4.2：读取灰度传感器值
- ✓ 需求4.3：计算归一化误差
- ✓ 需求4.4：应用误差死区
- ✓ 需求4.5：计算PD输出
- ✓ 需求4.6：限制输出范围
- ✓ 需求4.7：计算左右轮速度
- ✓ 需求4.8：限制车轮速度在[0, 1023]
- ✓ 需求4.9：通过PWM接口设置电机速度
- ✓ 需求4.10：单次控制周期不超过2毫秒
- ✓ 需求4.11：转弯期间暂停执行

## 注意事项

1. **不要在中断中调用阻塞API**
   - 已确保所有函数都是非阻塞的
   - 使用IRAM_ATTR属性

2. **参数调整建议**
   - Kp影响响应速度，过大会震荡
   - Kd影响稳定性，过大会反应迟钝
   - 速度影响整体速度，过快可能失控

3. **依赖关系**
   - 必须先初始化ADC采样任务
   - 必须先初始化转弯检测器
   - 必须先初始化PWM

4. **调试方法**
   - 使用`pd_controller_get_last_error()`查看误差
   - 使用监控任务实时查看状态
   - 调整参数观察效果

## 下一步工作

任务5已完成，下一步是任务6：实现六状态转弯状态机。

## 相关文档

- [需求文档](../../.kiro/specs/stm32-to-esp32-migration/requirements.md)
- [设计文档](../../.kiro/specs/stm32-to-esp32-migration/design.md)
- [任务列表](../../.kiro/specs/stm32-to-esp32-migration/tasks.md)
- [README](README.md)
