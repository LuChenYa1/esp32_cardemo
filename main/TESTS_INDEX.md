# 测试程序索引

## 概述

本文档列出了 `main/` 目录下的所有测试程序，帮助开发者快速找到所需的测试代码。测试程序按功能分类，标注了维护状态和依赖关系。

## 测试程序分类

### 主程序

| 文件名 | 测试目的 | 维护状态 | 说明文档 |
|--------|---------|---------|---------|
| `main.c` | 完整集成主程序 | ✅ 活跃 | [main.c说明](#mainc-完整集成主程序) |
| `main_integrated.c` | 集成测试程序（旧版） | ⏸️ 已弃用 | 已被main.c替代 |
| `main_integrated_test.c` | 系统集成测试 | ✅ 活跃 | 用于系统级功能验证 |

### 系统类测试

| 文件名 | 测试目的 | 维护状态 | 依赖组件 |
|--------|---------|---------|---------|
| `main_timer_test.c` | 定时器系统测试 | ✅ 活跃 | timer_system |
| `main_timer_watchdog_example.c` | 定时器看门狗示例 | ✅ 活跃 | timer_system |
| `main_multitask.c` | 多任务调度测试 | ✅ 活跃 | FreeRTOS |

### 传感器类测试

| 文件名 | 测试目的 | 维护状态 | 依赖组件 | 说明文档 |
|--------|---------|---------|---------|---------|
| `gray_calibration_test.c` | 灰度传感器校准 | ✅ 活跃 | gray_sensor | [GRAY_CALIBRATION_README.md](GRAY_CALIBRATION_README.md) |
| `main_adc_sampling_test.c` | ADC采样测试 | ✅ 活跃 | gray_sensor, ADC |  |
| `gpio_drive_test.c` | GPIO驱动能力测试 | ✅ 活跃 | board_config |  |
| `sensor_test_multitask.c` | 传感器多任务测试 | ✅ 活跃 | gray_sensor, ir_obstacle |  |

### 控制类测试

| 文件名 | 测试目的 | 维护状态 | 依赖组件 |
|--------|---------|---------|---------|
| `main_pd_test.c` | PD控制器测试 | ✅ 活跃 | pd_controller |
| `main_statemachine_test.c` | 状态机测试 | ✅ 活跃 | turn_statemachine |
| `main_turn_detector_test.c` | 转向检测器测试 | ✅ 活跃 | turn_detector |
| `line_following.c` | 巡线算法实现 | ✅ 活跃 | gray_sensor, pd_controller |
| `turn_detection.c` | 转向检测算法实现 | ✅ 活跃 | gray_sensor, turn_detector |

### 执行器类测试

| 文件名 | 测试目的 | 维护状态 | 依赖组件 |
|--------|---------|---------|---------|
| `main_servo_test.c` | 舵机控制测试 | ✅ 活跃 | servo_task, pwm |
| `main_cartest.c` | 小车运动测试 | ✅ 活跃 | pwm, servo_task |
| `main_car_demo.c` | 小车演示程序 | ✅ 活跃 | pwm, servo_task |

### 显示类测试

| 文件名 | 测试目的 | 维护状态 | 依赖组件 | 说明文档 |
|--------|---------|---------|---------|---------|
| `main_display_test.c` | TM1637数码管测试 | ✅ 活跃 | tm1637, display_task |  |
| `main_tm1640_test.c` | TM1640点阵屏测试 | ✅ 活跃 | tm1640 | [TM1640_TEST_README.md](TM1640_TEST_README.md) |

### 通信类测试

| 文件名 | 测试目的 | 维护状态 | 依赖组件 |
|--------|---------|---------|---------|
| `main_voice_test.c` | 语音模块测试 | ✅ 活跃 | voice_module, uart |
| `main_wireless_test.c` | 无线通信测试 | ✅ 活跃 | wireless, uart |

### 其他组件测试

| 文件名 | 测试目的 | 维护状态 | 依赖组件 | 说明文档 |
|--------|---------|---------|---------|---------|
| `main_pcf8574_test.c` | PCF8574 I/O扩展测试 | ✅ 活跃 | pcf8574 | [PCF8574_TEST_README.md](PCF8574_TEST_README.md) |

## 测试程序组织结构

```
main/
├── main.c                          # 主程序（完整集成）
├── TESTS_INDEX.md                  # 本文档
│
├── 系统类测试/
│   ├── main_timer_test.c
│   ├── main_timer_watchdog_example.c
│   └── main_multitask.c
│
├── 传感器类测试/
│   ├── gray_calibration_test.c
│   ├── main_adc_sampling_test.c
│   ├── gpio_drive_test.c
│   └── sensor_test_multitask.c
│
├── 控制类测试/
│   ├── main_pd_test.c
│   ├── main_statemachine_test.c
│   ├── main_turn_detector_test.c
│   ├── line_following.c/.h
│   └── turn_detection.c/.h
│
├── 执行器类测试/
│   ├── main_servo_test.c
│   ├── main_cartest.c
│   └── main_car_demo.c
│
├── 显示类测试/
│   ├── main_display_test.c
│   └── main_tm1640_test.c
│
├── 通信类测试/
│   ├── main_voice_test.c
│   └── main_wireless_test.c
│
├── 其他组件测试/
│   └── main_pcf8574_test.c
│
├── 说明文档/
│   ├── GRAY_CALIBRATION_README.md
│   ├── TM1640_TEST_README.md
│   ├── PCF8574_TEST_README.md
│   └── FIVE_WAY_GRAY_TEST_README.md
│
└── 备份文件/
    └── backup/                     # 旧版本备份
```

## 测试程序依赖关系

### 核心依赖链

```
main.c
  ├── timer_system (定时器系统)
  ├── board_config (板级配置)
  ├── gpio_manager (GPIO管理)
  ├── gray_sensor (灰度传感器)
  ├── ir_obstacle (红外避障)
  ├── turn_detector (转向检测)
  ├── pd_controller (PD控制)
  ├── turn_statemachine (状态机)
  ├── pwm (PWM控制)
  ├── servo_task (舵机任务)
  ├── tm1637 (数码管)
  ├── display_task (显示任务)
  ├── voice_module (语音模块)
  ├── camera_protocol (摄像头协议)
  └── traffic_light (交通灯)
```

### 独立测试程序

以下测试程序可以独立运行，不依赖其他测试程序：

- `gray_calibration_test.c` - 灰度传感器校准
- `main_timer_test.c` - 定时器测试
- `main_display_test.c` - 数码管测试
- `main_tm1640_test.c` - 点阵屏测试
- `main_pcf8574_test.c` - PCF8574测试
- `main_voice_test.c` - 语音模块测试
- `main_wireless_test.c` - 无线通信测试

### 组合测试程序

以下测试程序需要多个组件协同工作：

- `main.c` - 需要所有18个使用中的组件
- `main_integrated_test.c` - 需要传感器、控制、执行器组件
- `sensor_test_multitask.c` - 需要多个传感器组件
- `main_cartest.c` - 需要PWM和舵机组件

## 如何使用测试程序

### 方法1：修改CMakeLists.txt（推荐）

编辑 `main/CMakeLists.txt`，将主文件改为测试文件：

```cmake
idf_component_register(
    SRCS "main_xxx_test.c"  # 替换为你要测试的文件
    INCLUDE_DIRS "."
)
```

### 方法2：使用备份CMakeLists.txt

某些测试程序提供了专用的CMakeLists.txt：

```bash
# 例如：灰度传感器校准
cp main/CMakeLists_calibration.txt main/CMakeLists.txt
idf.py build flash monitor
```

### 方法3：临时重命名

```bash
# 备份原main.c
mv main/main.c main/main.c.bak

# 将测试文件重命名为main.c
cp main/main_xxx_test.c main/main.c

# 编译烧录
idf.py build flash monitor

# 恢复
mv main/main.c.bak main/main.c
```

## 测试程序编译步骤

### 通用步骤

1. **选择测试程序**：从上表中选择要运行的测试程序
2. **修改CMakeLists.txt**：将SRCS改为测试文件名
3. **编译项目**：
   ```bash
   idf.py build
   ```
4. **烧录到设备**：
   ```bash
   idf.py flash
   ```
5. **查看输出**：
   ```bash
   idf.py monitor
   ```
6. **退出监视器**：按 `Ctrl+]`

### 完整命令

```bash
# 一键编译、烧录、监视
idf.py build flash monitor
```

## 测试程序维护状态说明

| 状态 | 图标 | 说明 |
|------|-----|------|
| 活跃 | ✅ | 正在维护，推荐使用 |
| 已弃用 | ⏸️ | 已被新版本替代，不推荐使用 |
| 实验性 | 🧪 | 功能尚未稳定，谨慎使用 |

## 常见问题

### Q1：如何选择合适的测试程序？

**A：** 根据你要测试的功能选择：
- 测试单个组件 → 选择对应的组件测试程序
- 测试系统集成 → 选择 `main_integrated_test.c`
- 校准传感器 → 选择 `gray_calibration_test.c`
- 运行完整功能 → 选择 `main.c`

### Q2：测试程序编译失败怎么办？

**A：** 检查以下几点：
1. 确认依赖组件已正确配置
2. 检查CMakeLists.txt语法
3. 运行 `idf.py fullclean` 清理构建
4. 查看编译错误信息，确认缺少的头文件或库

### Q3：如何查看测试结果？

**A：** 测试结果通过串口输出：
1. 使用 `idf.py monitor` 查看实时日志
2. 观察LED、数码管等硬件输出
3. 检查串口日志中的 `✓` 或 `✗` 标记

### Q4：测试程序之间有冲突吗？

**A：** 某些测试程序可能使用相同的GPIO引脚：
- TM1637和TM1640共用GPIO34/37
- 不同传感器可能共用ADC通道
- 建议一次只运行一个测试程序

### Q5：如何添加新的测试程序？

**A：** 按以下步骤添加：
1. 在 `main/` 目录创建新的 `.c` 文件
2. 实现 `app_main()` 函数
3. 在CMakeLists.txt中添加文件名
4. 更新本索引文档

## 相关文档

- [项目README](../README.md) - 项目整体说明
- [组件文档](../components/) - 各组件详细文档
- [GPIO引脚分配](../docs/GPIO_PIN_ALLOCATION.md) - 硬件连接说明
- [配置参数指南](../docs/CONFIGURATION_GUIDE.md) - 参数调整说明
- [项目注意事项](../docs/IMPORTANT_NOTES.md) - 重要提示

---

## 测试程序详细说明

### main.c - 完整集成主程序

**测试目的：** 运行完整的巡线小车功能，集成所有使用中的组件

**使用的组件（18个）：**
- 系统类：timer_system, board_config, gpio_manager, pin_definitions
- 传感器类：gray_sensor, ir_obstacle
- 控制类：turn_detector, pd_controller, turn_statemachine
- 执行器类：pwm, servo_task
- 显示类：tm1637, display_task
- 通信类：voice_module, camera_protocol
- 其他：traffic_light

**主要功能：**
1. 硬件外设初始化（GPIO、ADC、PWM、UART、I2C）
2. 软件模块初始化（传感器、控制器、显示、通信）
3. FreeRTOS任务创建（灰度扫描、PD控制、显示更新、交通灯）
4. 系统自检（GPIO冲突检测、传感器校验）
5. 巡线控制循环

**硬件连接：** 参考 [GPIO引脚分配文档](../docs/GPIO_PIN_ALLOCATION.md)

**编译运行：**
```bash
# main.c是默认主程序，直接编译即可
idf.py build flash monitor
```

**预期输出：**
```
I (1234) MAIN: ========================================
I (1234) MAIN: ESP32巡线小车系统启动
I (1234) MAIN: ========================================
I (1234) MAIN: [1/4] 初始化硬件外设...
I (1234) BOARD_CONFIG: 板级配置初始化成功
I (1234) GPIO_MANAGER: GPIO管理器初始化成功
I (1234) GRAY_SENSOR: 灰度传感器初始化成功
I (1234) PWM: PWM初始化成功
I (1234) MAIN: [2/4] 初始化软件模块...
I (1234) PD_CONTROLLER: PD控制器初始化成功
I (1234) TURN_DETECTOR: 转向检测器初始化成功
I (1234) DISPLAY_TASK: 显示任务初始化成功
I (1234) MAIN: [3/4] 创建FreeRTOS任务...
I (1234) MAIN: [4/4] 系统自检...
I (1234) MAIN: ✓ 系统自检通过
I (1234) MAIN: ========================================
I (1234) MAIN: 系统启动完成，开始运行
I (1234) MAIN: ========================================
```

**故障排除：**
- 如果初始化失败，检查硬件连接
- 如果GPIO冲突，查看GPIO管理器日志
- 如果传感器读取异常，运行校准程序

**相关文档：**
- [配置参数指南](../docs/CONFIGURATION_GUIDE.md) - 调整PD参数和传感器阈值
- [项目注意事项](../docs/IMPORTANT_NOTES.md) - 重要限制和注意事项

---

**文档版本：** 1.0  
**创建日期：** 2024-12-XX  
**维护者：** 项目团队  
**最后更新：** 2024-12-XX
