# ESP32模块集成项目

## 项目简介

本项目是一个基于ESP32-S3的嵌入式系统，从STM32平台迁移而来。项目实现了完整的智能小车控制系统，包括灰度传感器巡线、PD控制、转弯检测、语音控制、摄像头识别等功能。

### 硬件平台

- **主控芯片**: ESP32-S3 (双核Xtensa LX7，240MHz)
- **开发框架**: ESP-IDF v5.x
- **RTOS**: FreeRTOS
- **外设接口**: GPIO、ADC、PWM、UART、I2C、SPI

### 主要功能

- ✅ 实时灰度传感器巡线（硬件定时器，3ms周期）
- ✅ PD控制算法（硬件定时器，10ms周期）
- ✅ 转弯检测与状态机控制
- ✅ 语音模块控制（UART1）
- ✅ 摄像头协议通信（UART0 RS485）
- ✅ 数码管显示（TM1637）
- ✅ 舵机控制（RS485总线）
- ✅ 红外避障传感器
- ✅ 交通灯控制
- ✅ GPIO冲突检测与管理

---

## 目录结构

```
esp32-module-integration/
├── README.md                    # 项目说明文档（本文件）
├── CMakeLists.txt              # 项目构建配置
├── sdkconfig                   # ESP-IDF配置文件
├── components/                 # 组件目录（38个硬件驱动组件）
│   ├── timer_system/          # 定时器系统
│   ├── board_config/          # 板级配置
│   ├── gpio_manager/          # GPIO管理器
│   ├── gray_sensor/           # 灰度传感器
│   ├── pd_controller/         # PD控制器
│   ├── turn_detector/         # 转弯检测
│   ├── turn_statemachine/     # 转弯状态机
│   ├── pwm/                   # PWM控制
│   ├── servo_task/            # 舵机任务
│   ├── tm1637/                # TM1637数码管
│   ├── display_task/          # 显示任务
│   ├── voice_module/          # 语音模块
│   ├── camera_protocol/       # 摄像头协议
│   ├── ir_obstacle/           # 红外避障
│   ├── traffic_light/         # 交通灯控制
│   └── ...                    # 其他组件（共38个）
├── main/                       # 主程序和测试程序
│   ├── main.c                 # 主程序（完整集成版本）
│   ├── TESTS_INDEX.md         # 测试程序索引
│   └── ...                    # 其他测试程序
├── docs/                       # 项目文档
│   ├── README.md              # 文档导航索引
│   ├── templates/             # 文档模板
│   ├── GPIO_PIN_ALLOCATION.md # GPIO引脚分配表
│   ├── CONFIGURATION_GUIDE.md # 配置参数指南
│   ├── IMPORTANT_NOTES.md     # 项目注意事项
│   └── ARCHIVE_SUMMARY.md     # 归档总结
└── .kiro/specs/               # 项目规格文档
    ├── stm32-to-esp32-migration/  # STM32迁移文档
    └── project-documentation-archival/  # 文档归档规格
```

---

## 快速开始

### 开发环境要求

- **ESP-IDF**: v5.0 或更高版本
- **工具链**: Xtensa ESP32-S3 工具链
- **Python**: 3.8 或更高版本
- **操作系统**: Windows / Linux / macOS

### 安装ESP-IDF

请参考ESP-IDF官方文档安装开发环境：
https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s3/get-started/

### 编译项目

```bash
# 1. 设置ESP-IDF环境变量
. $HOME/esp/esp-idf/export.sh  # Linux/macOS
# 或
%userprofile%\esp\esp-idf\export.bat  # Windows

# 2. 配置项目（可选）
idf.py menuconfig

# 3. 编译项目
idf.py build
```

### 烧录程序

```bash
# 烧录到ESP32-S3开发板
idf.py -p PORT flash

# 查看串口输出
idf.py -p PORT monitor

# 或者一次性完成编译、烧录和监控
idf.py -p PORT flash monitor
```

其中`PORT`是串口设备名称，例如：
- Linux: `/dev/ttyUSB0`
- Windows: `COM3`
- macOS: `/dev/cu.usbserial-*`

---

## 组件列表

本项目包含38个硬件驱动组件，以下列出main.c中实际使用的18个核心组件：

### 系统类组件

| 组件名称 | 功能说明 | 文档链接 |
|---------|---------|---------|
| timer_system | 硬件定时器系统，提供3ms和10ms周期的实时任务调度 | [README](components/timer_system/README.md) |
| board_config | 板级配置，负责系统初始化和GPIO管理器初始化 | [README](components/board_config/README.md) |
| gpio_manager | GPIO管理器，提供引脚分配和冲突检测功能 | [README](components/board_config/README.md) |
| pin_definitions | 引脚定义，集中管理所有GPIO引脚编号 | [README](components/board_config/README.md) |

### 传感器类组件

| 组件名称 | 功能说明 | 文档链接 |
|---------|---------|---------|
| gray_sensor | 灰度传感器，用于巡线检测（ADC2通道） | [README](components/gray_sensor/README.md) |
| ir_obstacle | 红外避障传感器，检测前方障碍物 | [README](components/ir_obstacle/README.md) |

### 控制类组件

| 组件名称 | 功能说明 | 文档链接 |
|---------|---------|---------|
| turn_detector | 转弯检测，基于灰度传感器判断转弯类型 | [README](components/turn_detector/README.md) |
| pd_controller | PD控制器，实现巡线的比例-微分控制 | [README](components/pd_controller/README.md) |
| turn_statemachine | 转弯状态机，管理转弯过程的状态转换 | [README](components/turn_statemachine/README.md) |

### 执行器类组件

| 组件名称 | 功能说明 | 文档链接 |
|---------|---------|---------|
| pwm | PWM控制，驱动4路电机 | [README](components/pwm/README.md) |
| servo_task | 舵机控制任务，通过RS485总线控制舵机 | [README](components/servo_task/README.md) |

### 显示类组件

| 组件名称 | 功能说明 | 文档链接 |
|---------|---------|---------|
| tm1637 | TM1637数码管驱动，显示系统状态 | [README](components/tm1637/README.md) |
| display_task | 显示任务，管理数码管显示内容的轮播 | [README](components/display_task/README.md) |

### 通信类组件

| 组件名称 | 功能说明 | 文档链接 |
|---------|---------|---------|
| voice_module | 语音模块，通过UART1接收语音命令 | [README](components/voice_module/README.md) |
| camera_protocol | 摄像头协议，通过UART0 RS485与摄像头通信 | [README](components/camera_protocol/README.md) |

### 其他组件

| 组件名称 | 功能说明 | 文档链接 |
|---------|---------|---------|
| traffic_light | 交通灯控制，模拟交通灯状态切换 | [README](components/traffic_light/README.md) |

> 📝 **注意**: 项目中还包含其他20个组件（如dht11、hc-sr04、mpu_6050、ble、wifi等），这些组件在当前main.c中未使用，但保留了完整的驱动代码和文档，可根据需要启用。完整组件列表请参考[文档导航](docs/README.md)。

---

## 硬件连接

### GPIO引脚分配总览

本项目使用GPIO管理器进行引脚分配和冲突检测。详细的引脚分配表请参考：

📄 [GPIO引脚分配文档](docs/GPIO_PIN_ALLOCATION.md)

### 关键引脚说明

- **灰度传感器**: GPIO1 (ADC2_CH0), GPIO2 (ADC2_CH1)
- **电机PWM**: GPIO4, GPIO5, GPIO6, GPIO7
- **TM1637数码管**: GPIO8 (CLK), GPIO9 (DIO)
- **语音模块UART1**: GPIO17 (TX), GPIO18 (RX)
- **摄像头/舵机UART0**: GPIO43 (TX), GPIO44 (RX), GPIO21 (RS485 DE)
- **红外避障**: GPIO10, GPIO11, GPIO12
- **交通灯**: GPIO39 (红灯), GPIO40 (绿灯)

### 接线注意事项

⚠️ **重要提示**：
- GPIO6-11被SPI Flash占用，不可用于外设
- ADC2通道与WiFi冲突，使用WiFi时ADC2不可用
- 所有外设必须与ESP32-S3共地
- 详细注意事项请参考[项目注意事项文档](docs/IMPORTANT_NOTES.md)

---

## 测试程序

项目包含多个测试程序，用于验证各个组件的功能。测试程序位于`main/`目录下。

### 主要测试程序

- `main.c` - 完整集成主程序（当前使用）
- `main_wireless_test.c` - 无线通信测试
- `main_tm1640_test.c` - TM1640数码管测试
- 其他测试程序...

详细的测试程序说明请参考：

📄 [测试程序索引](main/TESTS_INDEX.md)

### 切换测试程序

修改根目录的`CMakeLists.txt`文件，更改`SRCS`参数：

```cmake
idf_component_register(SRCS "main/main.c"  # 修改此处
                    INCLUDE_DIRS "")
```

---

## 文档索引

### 项目文档

- 📄 [文档导航](docs/README.md) - 所有文档的索引和导航
- 📄 [GPIO引脚分配](docs/GPIO_PIN_ALLOCATION.md) - 详细的引脚分配表
- 📄 [配置参数指南](docs/CONFIGURATION_GUIDE.md) - 系统参数配置说明
- 📄 [项目注意事项](docs/IMPORTANT_NOTES.md) - 重要的限制和注意事项
- 📄 [归档总结](docs/ARCHIVE_SUMMARY.md) - 项目归档总结

### 迁移文档

- 📄 [STM32迁移需求](.kiro/specs/stm32-to-esp32-migration/requirements.md)
- 📄 [STM32迁移设计](.kiro/specs/stm32-to-esp32-migration/design.md)
- 📄 [STM32迁移任务](.kiro/specs/stm32-to-esp32-migration/tasks.md)

### 组件文档

每个组件都有独立的README文档，位于`components/[组件名]/README.md`。

---

## 系统架构

### 任务调度架构

本项目采用混合调度架构，结合硬件定时器和FreeRTOS任务：

#### 硬件定时器任务（高优先级，实时性要求高）

- **Timer 0 (3ms周期)**:
  - 灰度传感器扫描
  - 转弯检测

- **Timer 1 (10ms周期)**:
  - PD控制算法
  - 转弯状态机

#### FreeRTOS任务（低优先级，实时性要求低）

- **ADC采样任务** (优先级5): 1ms周期，读取灰度传感器ADC值
- **语音模块任务** (优先级4): 接收和处理语音命令
- **数码管显示任务** (优先级3): 5秒轮播显示系统状态
- **舵机控制任务** (优先级3): 控制舵机动作
- **交通灯控制任务** (优先级2): 3秒切换交通灯状态

### 数据流架构

```
灰度传感器(ADC) → 转弯检测 → PD控制器 → PWM输出 → 电机
                      ↓
                  转弯状态机
                      ↓
                  舵机控制

语音模块(UART1) → 运行模式控制 → 启停电机
                      ↓
                  摄像头命令

摄像头(UART0) → 识别结果 → 语音模块标志位
```

---

## 项目历史

### STM32迁移背景

本项目原本基于STM32平台开发，为了利用ESP32-S3的WiFi/蓝牙功能和更强的处理能力，进行了完整的平台迁移。

### 主要变更

- **硬件平台**: STM32 → ESP32-S3
- **开发框架**: STM32 HAL → ESP-IDF
- **RTOS**: 无 → FreeRTOS
- **定时器**: STM32 TIM → ESP32 硬件定时器
- **ADC**: STM32 ADC → ESP32 ADC2
- **GPIO**: STM32 GPIO → ESP32 GPIO + GPIO管理器

### 迁移成果

✅ 所有核心功能已成功迁移并验证  
✅ 新增GPIO冲突检测机制  
✅ 新增完整的文档系统  
✅ 保持了原有的控制算法和性能

详细的迁移文档请参考：[STM32迁移规格](.kiro/specs/stm32-to-esp32-migration/)

---

## 开发指南

### 代码规范

- 使用中文注释，技术术语保留英文
- 函数命名采用`模块名_功能名`格式
- 所有公共函数必须有Doxygen格式注释
- 使用ESP-IDF的日志系统（ESP_LOGI/ESP_LOGW/ESP_LOGE）

### 添加新组件

1. 在`components/`目录下创建组件文件夹
2. 使用[组件README模板](docs/templates/COMPONENT_README_TEMPLATE.md)创建文档
3. 在GPIO管理器中注册使用的引脚
4. 在main.c中初始化和使用组件

### 调试技巧

- 使用`idf.py monitor`查看串口日志
- 使用`esp_log_level_set()`调整日志级别
- 使用GPIO管理器的`gpio_manager_print_allocation_table()`查看引脚分配
- 参考[项目注意事项](docs/IMPORTANT_NOTES.md)中的调试章节

---

## 常见问题

### Q: 编译时提示找不到ESP-IDF？

A: 请确保已正确安装ESP-IDF并执行了`export.sh`或`export.bat`脚本。

### Q: 烧录失败？

A: 检查串口连接，确保开发板处于下载模式（按住BOOT按钮后按RESET按钮）。

### Q: ADC读取值异常？

A: 确保未同时使用WiFi功能（ADC2与WiFi冲突），检查传感器接线和供电。

### Q: 电机不转？

A: 检查PWM引脚连接，确认运行模式为巡线模式（语音命令控制）。

更多问题请参考各组件的故障排除章节。

---

## 贡献指南

欢迎提交问题报告和改进建议！

### 提交问题

请在问题报告中包含：
- 问题描述
- 复现步骤
- 预期行为
- 实际行为
- 日志输出

### 提交代码

1. Fork本项目
2. 创建特性分支
3. 提交代码并添加测试
4. 更新相关文档
5. 提交Pull Request

---

## 许可证

[请根据实际情况添加许可证信息]

---

## 联系方式

- **项目维护者**: [维护者名称]
- **邮箱**: [联系邮箱]
- **项目地址**: [Git仓库地址]

---

**项目**: ESP32模块集成项目  
**版本**: 1.0.0  
**更新日期**: 2024-12-XX  
**文档类型**: 项目说明文档
