# GPIO引脚分配总结

## 📋 文档概览

本项目包含以下GPIO相关文档：

1. **GPIO_PIN_ALLOCATION.md** - 完整的引脚分配文档（详细版）
2. **GPIO_QUICK_REFERENCE.md** - 快速参考表（速查版）
3. 本文档 - 总结和关键信息

---

## 🎯 关键信息

### 总计使用GPIO数量：33个

| 类别 | 数量 | GPIO列表 |
|------|------|---------|
| 电机PWM | 8 | 2,3,4,5,6,7,8,9 |
| 编码器 | 8 | 14,15,16,17,41,42,45,46 |
| 串口通信 | 5 | 19,35,36,43,44 |
| I2C总线 | 2 | 20,21 |
| 传感器 | 6 | 33,34,37,38,39,40 |
| PCF扩展 | 2 | P5,P6（通过I2C控制） |

---

## ⚠️ 重要变更

### 与设计文档的差异

设计文档中的GPIO分配与实际硬件有以下差异：

| 设计文档 | 实际硬件 | 原因 |
|---------|---------|------|
| GPIO19 - 左灰度传感器 | GPIO19 - RS485方向控制 | 硬件已固定 |
| GPIO20 - 右灰度传感器 | GPIO20 - I2C_SDA | 硬件已固定 |
| GPIO10 - TM1637_CLK | GPIO34 - TM1637_CLK | 实际连接 |
| GPIO11 - TM1637_DIO | GPIO33 - TM1637_DIO | 实际连接 |
| GPIO22 - I2C_SCL | GPIO21 - I2C_SCL | 实际连接 |
| GPIO21 - I2C_SDA | GPIO20 - I2C_SDA | 实际连接 |

### 推荐的灰度传感器引脚

由于GPIO19/20已被占用，推荐使用：

- **左灰度传感器**: GPIO47 (ADC1_CH6) - 对应硬件接口SSA4
- **右灰度传感器**: GPIO48 (ADC1_CH7) - 对应硬件接口SSD4

**优点**：
- 使用ADC1通道，与WiFi不冲突
- 引脚可用且未被占用
- 符合硬件接口标识

---

## 🔧 需要修改的代码

### 1. 灰度传感器模块

**文件**: `components/gray_sensor/gray_sensor.c`

```c
// 修改前（设计文档）
#define GRAY_SENSOR_LEFT_GPIO   GPIO_NUM_19  // ADC2_CH9
#define GRAY_SENSOR_RIGHT_GPIO  GPIO_NUM_20  // ADC2_CH7

// 修改后（实际硬件）
#define GRAY_SENSOR_LEFT_GPIO   GPIO_NUM_47  // ADC1_CH6
#define GRAY_SENSOR_RIGHT_GPIO  GPIO_NUM_48  // ADC1_CH7
```

### 2. GPIO管理器集成

**文件**: `components/gpio_manager/gpio_manager_integration.c`

已更新为实际硬件配置。

### 3. 定时器系统

**文件**: `components/timer_system/timer_system.c`

确保使用正确的灰度传感器GPIO。

### 4. PD控制器

**文件**: `components/pd_controller/pd_controller.c`

确保读取正确的ADC通道。

---

## 📊 引脚使用统计

### 按功能模块

```
运动控制系统: 16个GPIO (48.5%)
├─ 电机PWM:    8个
└─ 编码器:     8个

通信接口:      7个GPIO (21.2%)
├─ UART0:      2个
├─ UART1:      2个
├─ RS485:      1个
└─ I2C:        2个

传感器系统:    6个GPIO (18.2%)
├─ 灰度传感器: 2个
├─ 数码管:     2个
├─ 温湿度:     1个
└─ 红外避障:   1个

交通灯控制:    2个GPIO (6.1%)
└─ 信号输出:   2个

扩展IO:        2个PCF (6.1%)
└─ PCF8574:    P5, P6
```

### 按GPIO类型

| 类型 | 数量 | 说明 |
|------|------|------|
| 数字输出 | 11 | PWM×8 + RS485DIR×1 + 交通灯×2 |
| 数字输入 | 9 | 编码器×8 + 红外避障×1 |
| ADC输入 | 2 | 灰度传感器×2 |
| UART | 4 | TX×2 + RX×2 |
| I2C | 2 | SCL×1 + SDA×1 |
| 软件协议 | 3 | TM1637×2 + DHT11×1 |

---

## 🚀 快速开始

### 步骤1：更新GPIO管理器

```bash
# GPIO管理器已自动更新为实际硬件配置
# 无需手动修改
```

### 步骤2：更新灰度传感器配置

在你的代码中查找并替换：

```bash
# 查找使用GPIO19/20作为灰度传感器的代码
grep -r "GPIO_NUM_19" components/gray_sensor/
grep -r "GPIO_NUM_20" components/gray_sensor/

# 替换为GPIO47/48
```

### 步骤3：验证配置

```c
// 在app_main()中调用
esp_err_t ret = gpio_manager_system_init();
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "GPIO冲突检测失败！");
    // 查看日志中的冲突信息
}
```

### 步骤4：测试硬件

按以下顺序测试：

1. ✅ 电机PWM输出
2. ✅ 编码器计数
3. ✅ 串口通信
4. ✅ I2C通信（PCF8574）
5. ✅ 灰度传感器ADC读取
6. ✅ 其他传感器

---

## 📝 检查清单

在部署到硬件前，确认以下事项：

### 代码配置
- [ ] 灰度传感器使用GPIO47/48
- [ ] RS485方向控制使用GPIO19
- [ ] I2C使用GPIO20/21
- [ ] 电机PWM使用GPIO2-9
- [ ] 编码器使用正确的GPIO
- [ ] UART配置正确

### GPIO管理器
- [ ] 已调用gpio_manager_system_init()
- [ ] 启动时无GPIO冲突错误
- [ ] GPIO分配表打印正确

### 硬件连接
- [ ] 所有传感器接线正确
- [ ] 电源和地线连接良好
- [ ] I2C总线有上拉电阻
- [ ] 编码器信号线屏蔽良好

### 功能测试
- [ ] 电机可以正常转动
- [ ] 编码器计数正确
- [ ] 串口通信正常
- [ ] 传感器读取正常
- [ ] 无GPIO冲突

---

## 🐛 故障排除

### 问题1：GPIO冲突错误

**现象**: 启动时报告GPIO冲突

**解决**:
1. 查看日志中的冲突信息
2. 检查是否有重复分配
3. 更新gpio_manager_integration.c

### 问题2：灰度传感器读取失败

**现象**: ADC读取返回错误或数值异常

**解决**:
1. 确认使用GPIO47/48（ADC1通道）
2. 检查ADC初始化代码
3. 确认未启用WiFi（如使用ADC2）
4. 检查传感器供电

### 问题3：编码器计数不准

**现象**: 编码器计数跳变或不计数

**解决**:
1. 检查GPIO配置（上拉电阻）
2. 检查PCNT配置
3. 检查编码器接线
4. 添加硬件滤波

### 问题4：I2C通信失败

**现象**: PCF8574无响应

**解决**:
1. 检查I2C地址
2. 检查上拉电阻（4.7kΩ）
3. 使用i2c_scan扫描设备
4. 检查供电电压

---

## 📚 相关资源

### 项目文档
- [完整引脚分配](./GPIO_PIN_ALLOCATION.md)
- [快速参考](./GPIO_QUICK_REFERENCE.md)
- [GPIO管理器](../components/gpio_manager/README.md)
- [设计文档](../.kiro/specs/stm32-to-esp32-migration/design.md)

### ESP32参考
- [ESP32-S3技术参考手册](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_cn.pdf)
- [ESP-IDF GPIO API](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s3/api-reference/peripherals/gpio.html)
- [ESP-IDF ADC API](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s3/api-reference/peripherals/adc_oneshot.html)

### 硬件资料
- PCF8574数据手册
- DHT11数据手册
- TM1637数据手册
- HC-SR04超声波传感器数据手册
- 红外避障传感器数据手册

---

## 📞 支持

如有问题，请：

1. 查看相关文档
2. 检查GPIO分配表
3. 使用GPIO管理器验证配置
4. 查看日志输出

---

**最后更新**: 2024-12-XX  
**维护者**: 项目团队
