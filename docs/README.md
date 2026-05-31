# ESP32项目文档导航

## 文档概述

本文档是ESP32模块集成项目的文档导航中心，提供所有项目文档的索引和快速访问入口。项目采用分层文档结构，包括项目级文档、组件文档、测试文档和规格文档。

**文档版本**: 1.0  
**更新日期**: 2024-12-20  
**维护者**: 项目团队

---

## 文档结构

```
esp32-module-integration/
├── README.md                          # 项目总览
├── docs/                              # 项目文档目录
│   ├── README.md                      # 文档导航（本文件）
│   ├── templates/                     # 文档模板
│   │   └── COMPONENT_README_TEMPLATE.md
│   ├── GPIO_PIN_ALLOCATION.md         # GPIO引脚分配
│   ├── CONFIGURATION_GUIDE.md         # 配置参数指南
│   ├── IMPORTANT_NOTES.md             # 项目注意事项
│   ├── ARCHIVE_SUMMARY.md             # 归档总结
│   └── DOCUMENTATION_CHECKLIST.md     # 文档检查清单
├── components/                        # 组件文档（38个）
│   └── [component_name]/README.md
├── main/                              # 测试程序文档
│   └── TESTS_INDEX.md
└── .kiro/specs/                       # 规格文档
    ├── stm32-to-esp32-migration/      # STM32迁移规格
    └── project-documentation-archival/ # 文档归档规格
```

---

## 快速导航

### 新手入门

如果你是第一次接触本项目，建议按以下顺序阅读文档：

1. 📄 [项目README](../README.md) - 了解项目概况和快速开始
2. 📄 [GPIO引脚分配](GPIO_PIN_ALLOCATION.md) - 了解硬件连接
3. 📄 [项目注意事项](IMPORTANT_NOTES.md) - 了解重要限制和注意事项
4. 📄 [测试程序索引](../main/TESTS_INDEX.md) - 了解如何测试各个功能
5. 📄 [配置参数指南](CONFIGURATION_GUIDE.md) - 了解如何调整系统参数

### 开发人员

如果你需要开发新功能或修改现有代码：

1. 📄 [组件文档](#组件文档) - 查阅相关组件的API和使用方法
2. 📄 [GPIO引脚分配](GPIO_PIN_ALLOCATION.md) - 规划GPIO使用
3. 📄 [配置参数指南](CONFIGURATION_GUIDE.md) - 调整控制参数
4. 📄 [STM32迁移设计文档](#迁移文档) - 了解系统架构

### 硬件工程师

如果你需要连接硬件或排查硬件问题：

1. 📄 [GPIO引脚分配](GPIO_PIN_ALLOCATION.md) - 详细的引脚分配表
2. 📄 [项目注意事项](IMPORTANT_NOTES.md) - 硬件限制和接线注意事项
3. 📄 [组件文档](#组件文档) - 各组件的硬件连接说明

### 项目维护者

如果你需要维护项目或进行交接：

1. 📄 [归档总结](ARCHIVE_SUMMARY.md) - 项目整体概况
2. 📄 [STM32迁移文档](#迁移文档) - 项目历史和技术决策
3. 📄 [文档检查清单](DOCUMENTATION_CHECKLIST.md) - 文档完整性验证

---

## 项目级文档

### 核心文档

| 文档名称 | 路径 | 说明 | 更新日期 |
|---------|------|------|---------|
| 项目README | [../README.md](../README.md) | 项目总览、快速开始、组件列表 | 2024-12-20 |
| 文档导航 | [README.md](README.md) | 所有文档的索引和导航（本文件） | 2024-12-20 |
| GPIO引脚分配 | [GPIO_PIN_ALLOCATION.md](GPIO_PIN_ALLOCATION.md) | 详细的GPIO引脚分配表和冲突检测 | 2024-12-20 |
| 配置参数指南 | [CONFIGURATION_GUIDE.md](CONFIGURATION_GUIDE.md) | 系统参数配置和调试指南 | 2024-12-20 |
| 项目注意事项 | [IMPORTANT_NOTES.md](IMPORTANT_NOTES.md) | 硬件限制、软件限制、已知问题 | 2024-12-20 |
| 归档总结 | [ARCHIVE_SUMMARY.md](ARCHIVE_SUMMARY.md) | 项目概况、技术决策、性能指标 | 2024-12-20 |

### 辅助文档

| 文档名称 | 路径 | 说明 | 更新日期 |
|---------|------|------|---------|
| 组件README模板 | [templates/COMPONENT_README_TEMPLATE.md](templates/COMPONENT_README_TEMPLATE.md) | 组件文档标准模板 | 2024-12-20 |
| 文档检查清单 | [DOCUMENTATION_CHECKLIST.md](DOCUMENTATION_CHECKLIST.md) | 文档完整性检查清单 | 2024-12-20 |

---

## 组件文档

项目包含38个硬件驱动组件，每个组件都有独立的README文档。以下按功能分类列出：

### 系统类组件（4个）

| 组件名称 | 文档路径 | 功能说明 | 使用状态 |
|---------|---------|---------|---------|
| timer_system | [../components/timer_system/README.md](../components/timer_system/README.md) | 硬件定时器系统，3ms和10ms周期 | ✅ 使用中 |
| board_config | [../components/board_config/README.md](../components/board_config/README.md) | 板级配置和系统初始化 | ✅ 使用中 |
| gpio_manager | [../components/board_config/README.md](../components/board_config/README.md) | GPIO管理器，引脚分配和冲突检测 | ✅ 使用中 |
| pin_definitions | [../components/board_config/README.md](../components/board_config/README.md) | 引脚定义，集中管理GPIO编号 | ✅ 使用中 |

### 传感器类组件（6个）

| 组件名称 | 文档路径 | 功能说明 | 使用状态 |
|---------|---------|---------|---------|
| gray_sensor | [../components/gray_sensor/README.md](../components/gray_sensor/README.md) | 灰度传感器，巡线检测 | ✅ 使用中 |
| ir_obstacle | [../components/ir_obstacle/README.md](../components/ir_obstacle/README.md) | 红外避障传感器 | ✅ 使用中 |
| dht11 | [../components/dht11/README.md](../components/dht11/README.md) | 温湿度传感器 | ⏸️ 未使用 |
| hc-sr04 | [../components/hc-sr04/README.md](../components/hc-sr04/README.md) | 超声波测距传感器 | ⏸️ 未使用 |

### 控制类组件（3个）

| 组件名称 | 文档路径 | 功能说明 | 使用状态 |
|---------|---------|---------|---------|
| turn_detector | [../components/turn_detector/README.md](../components/turn_detector/README.md) | 转弯检测，判断转弯类型 | ✅ 使用中 |
| pd_controller | [../components/pd_controller/README.md](../components/pd_controller/README.md) | PD控制器，巡线算法 | ✅ 使用中 |
| turn_statemachine | [../components/turn_statemachine/README.md](../components/turn_statemachine/README.md) | 转弯状态机，管理转弯流程 | ✅ 使用中 |

### 执行器类组件（5个）

| 组件名称 | 文档路径 | 功能说明 | 使用状态 |
|---------|---------|---------|---------|
| pwm | [../components/pwm/README.md](../components/pwm/README.md) | PWM控制，驱动4路电机 | ✅ 使用中 |
| servo_task | [../components/servo_task/README.md](../components/servo_task/README.md) | 舵机控制任务 | ✅ 使用中 |
| 485servo | [../components/485servo/README.md](../components/485servo/README.md) | RS485舵机驱动 | ⏸️ 未使用 |
| encoder | [../components/encoder/README.md](../components/encoder/README.md) | 编码器读取 | ⏸️ 未使用 |
| buzzer | [../components/buzzer/README.md](../components/buzzer/README.md) | 蜂鸣器控制 | ⏸️ 未使用 |

### 显示类组件（4个）

| 组件名称 | 文档路径 | 功能说明 | 使用状态 |
|---------|---------|---------|---------|
| tm1637 | [../components/tm1637/README.md](../components/tm1637/README.md) | TM1637数码管驱动 | ✅ 使用中 |
| display_task | [../components/display_task/README.md](../components/display_task/README.md) | 显示任务，轮播显示 | ✅ 使用中 |
| led | [../components/led/README.md](../components/led/README.md) | LED控制 | ⏸️ 未使用 |

### 通信类组件（8个）

| 组件名称 | 文档路径 | 功能说明 | 使用状态 |
|---------|---------|---------|---------|
| voice_module | [../components/voice_module/README.md](../components/voice_module/README.md) | 语音模块，UART1通信 | ✅ 使用中 |
| camera_protocol | [../components/camera_protocol/README.md](../components/camera_protocol/README.md) | 摄像头协议，UART0 RS485 | ✅ 使用中 |
| uart | [../components/uart/README.md](../components/uart/README.md) | UART通用驱动 | ⏸️ 未使用 |
| wireless | [../components/wireless/README.md](../components/wireless/README.md) | 无线通信模块 | ⏸️ 未使用 |
| ble | [../components/ble/README.md](../components/ble/README.md) | 蓝牙BLE功能 | ⏸️ 未使用 |
| tcp | [../components/tcp/README.md](../components/tcp/README.md) | TCP网络通信 | ⏸️ 未使用 |
| udp | [../components/udp/README.md](../components/udp/README.md) | UDP网络通信 | ⏸️ 未使用 |
| wifi | [../components/wifi/README.md](../components/wifi/README.md) | WiFi功能 | ⏸️ 未使用 |

### 其他组件（8个）

| 组件名称 | 文档路径 | 功能说明 | 使用状态 |
|---------|---------|---------|---------|
| traffic_light | [../components/traffic_light/README.md](../components/traffic_light/README.md) | 交通灯控制 | ✅ 使用中 |
| key | [../components/key/README.md](../components/key/README.md) | 按键检测 | ⏸️ 未使用 |
| pcf8574 | [../components/pcf8574/README.md](../components/pcf8574/README.md) | PCF8574 I/O扩展 | ⏸️ 未使用 |
| pcf_buzzer | [../components/pcf_buzzer/README.md](../components/pcf_buzzer/README.md) | PCF蜂鸣器 | ⏸️ 未使用 |
| spiflash | [../components/spiflash/README.md](../components/spiflash/README.md) | SPI Flash操作 | ⏸️ 未使用 |
| ssax1 | [../components/ssax1/README.md](../components/ssax1/README.md) | SSA接口扩展 | ⏸️ 未使用 |
| ssdx | [../components/ssdx/README.md](../components/ssdx/README.md) | SSD接口扩展 | ⏸️ 未使用 |
| us_delay | [../components/us_delay/README.md](../components/us_delay/README.md) | 微秒延时功能 | ⏸️ 未使用 |

> 📝 **说明**: 
> - ✅ 使用中：在main.c中实际使用的组件
> - ⏸️ 未使用：保留了驱动代码和文档，可根据需要启用

---

## 测试文档

### 测试程序索引

📄 [测试程序索引](../main/TESTS_INDEX.md) - 所有测试程序的列表和说明

### 主要测试程序

| 测试程序 | 文档路径 | 测试内容 |
|---------|---------|---------|
| main.c | - | 完整集成主程序 |
| main_wireless_test.c | [../main/WIRELESS_TEST_README.md](../main/WIRELESS_TEST_README.md) | 无线通信测试 |
| main_servo_test.c | - | 舵机控制测试 |
| main_voice_test.c | - | 语音模块测试 |

---

## 迁移文档

### STM32到ESP32迁移规格

本项目从STM32平台迁移到ESP32-S3，以下文档记录了迁移过程的需求、设计和实施：

| 文档名称 | 路径 | 说明 | 版本 |
|---------|------|------|------|
| 迁移需求文档 | [../.kiro/specs/stm32-to-esp32-migration/requirements.md](../.kiro/specs/stm32-to-esp32-migration/requirements.md) | 功能需求、正确性属性 | 1.0 |
| 迁移设计文档 | [../.kiro/specs/stm32-to-esp32-migration/design.md](../.kiro/specs/stm32-to-esp32-migration/design.md) | 系统架构、组件设计、测试策略 | 1.0 |
| 迁移任务列表 | [../.kiro/specs/stm32-to-esp32-migration/tasks.md](../.kiro/specs/stm32-to-esp32-migration/tasks.md) | 实施计划、任务进度 | 1.0 |

#### 迁移关键内容

- **架构变更**: STM32 TIM中断 → ESP32硬件定时器 + FreeRTOS
- **ADC策略**: 阻塞式ADC读取 + 共享变量缓存
- **线程安全**: 原子变量保护跨中断/任务的共享数据
- **实时性保证**: Timer 0 (3ms) 灰度扫描，Timer 1 (10ms) PD控制
- **错误处理**: ADC失败保持上次值，连续失败进入安全模式

### 文档归档规格

本项目的文档归档整理规格：

| 文档名称 | 路径 | 说明 | 版本 |
|---------|------|------|------|
| 归档需求文档 | [../.kiro/specs/project-documentation-archival/requirements.md](../.kiro/specs/project-documentation-archival/requirements.md) | 文档归档的功能需求 | 1.0 |
| 归档设计文档 | [../.kiro/specs/project-documentation-archival/design.md](../.kiro/specs/project-documentation-archival/design.md) | 文档系统架构、模板设计 | 1.0 |
| 归档任务列表 | [../.kiro/specs/project-documentation-archival/tasks.md](../.kiro/specs/project-documentation-archival/tasks.md) | 文档整理实施计划 | 1.0 |

---

## 文档交叉引用

### 硬件相关文档链接

- 项目README → [GPIO引脚分配](GPIO_PIN_ALLOCATION.md)
- 组件README → [GPIO引脚分配](GPIO_PIN_ALLOCATION.md)
- 项目注意事项 → [GPIO引脚分配](GPIO_PIN_ALLOCATION.md)
- 配置参数指南 → [组件文档](#组件文档)

### 软件相关文档链接

- 项目README → [测试程序索引](../main/TESTS_INDEX.md)
- 测试程序索引 → [组件文档](#组件文档)
- 配置参数指南 → [STM32迁移设计文档](../.kiro/specs/stm32-to-esp32-migration/design.md)

### 架构相关文档链接

- 项目README → [STM32迁移文档](#迁移文档)
- GPIO引脚分配 → [STM32迁移设计文档](../.kiro/specs/stm32-to-esp32-migration/design.md)
- 项目注意事项 → [STM32迁移设计文档](../.kiro/specs/stm32-to-esp32-migration/design.md)

---

## 文档更新流程

### 文档维护原则

1. **同步更新**: 代码变更时同步更新相关文档
2. **版本标注**: 每次更新标注版本号和更新日期
3. **交叉引用**: 保持文档间链接的有效性
4. **格式统一**: 遵循文档模板和格式规范

### 文档更新步骤

1. **修改代码**: 完成代码功能开发或修改
2. **更新组件文档**: 更新对应组件的README.md
3. **更新项目文档**: 如涉及GPIO、配置参数等，更新相应项目文档
4. **检查交叉引用**: 验证所有文档链接有效
5. **更新版本信息**: 修改文档末尾的版本号和更新日期
6. **提交变更**: 将代码和文档一起提交

### 文档审查清单

使用 [文档检查清单](DOCUMENTATION_CHECKLIST.md) 验证文档完整性：

- [ ] 所有组件都有README文档
- [ ] 所有测试程序都有说明
- [ ] GPIO分配表准确无误
- [ ] 配置参数文档完整
- [ ] 所有文档链接有效
- [ ] 文档格式统一
- [ ] 版本信息已更新

---

## 文档贡献指南

### 添加新组件文档

1. 使用 [组件README模板](templates/COMPONENT_README_TEMPLATE.md)
2. 填写所有必需章节（概述、硬件连接、API接口、使用示例等）
3. 在GPIO管理器中注册使用的引脚
4. 更新本文档的[组件文档](#组件文档)章节
5. 如果组件在main.c中使用，更新项目README的组件列表

### 更新项目文档

1. 确认修改内容的准确性
2. 保持文档格式统一（Markdown格式）
3. 更新文档末尾的版本信息
4. 检查相关文档的交叉引用
5. 提交前运行文档检查清单

### 报告文档问题

如果发现文档错误或不完整，请：

1. 记录问题描述（哪个文档、哪个章节、什么问题）
2. 提供正确信息或改进建议
3. 提交问题报告或直接修改文档

---

## 常见问题

### Q: 如何查找特定组件的文档？

A: 在本文档的[组件文档](#组件文档)章节按功能分类查找，或直接访问`components/[组件名]/README.md`。

### Q: 如何了解GPIO引脚使用情况？

A: 查阅 [GPIO引脚分配文档](GPIO_PIN_ALLOCATION.md)，包含详细的引脚分配表和冲突检测说明。

### Q: 如何调整系统参数？

A: 查阅 [配置参数指南](CONFIGURATION_GUIDE.md)，包含所有可调参数的说明和调试方法。

### Q: 如何了解项目的技术架构？

A: 查阅 [STM32迁移设计文档](../.kiro/specs/stm32-to-esp32-migration/design.md)，包含系统架构、数据流、任务调度等详细说明。

### Q: 如何测试某个功能？

A: 查阅 [测试程序索引](../main/TESTS_INDEX.md)，找到对应的测试程序和使用说明。

### Q: 文档链接失效怎么办？

A: 检查文件路径是否正确，或使用文档检查清单验证所有链接。如果问题持续存在，请报告文档问题。

---

## 文档版本历史

| 版本 | 日期 | 修改内容 | 修改人 |
|------|------|---------|--------|
| 1.0 | 2024-12-20 | 创建文档导航索引，整理所有文档链接 | 项目团队 |

---

## 联系方式

如有文档相关问题或建议，请联系：

- **项目维护者**: [维护者名称]
- **邮箱**: [联系邮箱]
- **项目地址**: [Git仓库地址]

---

**文档**: ESP32项目文档导航  
**版本**: 1.0  
**更新日期**: 2024-12-20  
**维护者**: 项目团队
