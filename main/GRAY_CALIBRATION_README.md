# 灰度传感器校准测试程序使用说明

## 概述

这个测试程序用于校准灰度传感器，帮助你获取准确的白色和黑线ADC值。

## 功能特性

1. **实时显示原始ADC值** - 观察传感器在不同表面上的读数
2. **交互式校准** - 引导用户完成白色和黑线的校准
3. **校准效果测试** - 显示归一化值和黑线检测结果
4. **持续监控模式** - 实时显示传感器状态

## 使用步骤

### 1. 编译和烧录

```bash
# 备份原CMakeLists.txt
cp main/CMakeLists.txt main/CMakeLists.txt.backup

# 使用校准测试的CMakeLists.txt
cp main/CMakeLists_calibration.txt main/CMakeLists.txt

# 编译并烧录
idf.py build flash monitor
```

### 2. 运行校准程序

程序启动后会自动执行以下步骤：

#### 阶段1: 显示原始ADC值（10秒）
- 程序会持续显示左右传感器的原始ADC值
- 你可以将传感器在白色和黑线上移动，观察数值变化
- 这有助于了解传感器的工作范围

#### 阶段2: 交互式校准

**步骤1 - 白色区域校准：**
1. 程序提示："请将传感器放置在白色区域上"
2. 将传感器放在白色表面（赛道的白色部分）
3. 等待5秒倒计时
4. 程序自动采集100次数据，取最大值作为白色基准
5. 显示采集结果

**步骤2 - 黑线区域校准：**
1. 程序提示："请将传感器放置在黑线上"
2. 将传感器放在黑线上（赛道的黑色线条）
3. 等待5秒倒计时
4. 程序自动采集100次数据，取最小值作为黑线基准
5. 显示采集结果

#### 阶段3: 测试校准效果（20秒）
- 程序显示归一化后的传感器值（0.0-1.0）
- 显示黑线检测结果（[黑线] 或 [白色]）
- 你可以将传感器在白色和黑线之间移动，验证检测准确性

#### 阶段4: 持续监控模式
- 程序进入持续监控模式，实时显示传感器状态
- 按 Ctrl+C 退出程序

### 3. 获取校准参数

校准完成后，程序会输出建议的校准参数：

```
建议的校准参数（可复制到main.c中）：
#define LEFT_WHITE_VALUE 4095
#define LEFT_BLACK_VALUE 1476
#define RIGHT_WHITE_VALUE 4095
#define RIGHT_BLACK_VALUE 1546
```

### 4. 应用校准参数

将获取的校准参数复制到 `main/main.c` 文件中：

```c
// ==================== 校准参数 ====================
#define LEFT_WHITE_VALUE 4095    // 替换为你的值
#define LEFT_BLACK_VALUE 1476    // 替换为你的值
#define RIGHT_WHITE_VALUE 4095   // 替换为你的值
#define RIGHT_BLACK_VALUE 1546   // 替换为你的值
```

### 5. 恢复原程序

```bash
# 恢复原CMakeLists.txt
cp main/CMakeLists.txt.backup main/CMakeLists.txt

# 重新编译主程序
idf.py build flash monitor
```

## 输出示例

### 原始ADC值显示
```
I (1234) GRAY_CALIBRATION: [  0.0秒] 左传感器: 4095 | 右传感器: 4095
I (1334) GRAY_CALIBRATION: [  0.1秒] 左传感器: 4090 | 右传感器: 4088
I (1434) GRAY_CALIBRATION: [  0.2秒] 左传感器: 1480 | 右传感器: 1550
```

### 校准数据采集
```
I (5678) GRAY_CALIBRATION: 正在采集白色区域数据（100次采样）...
I (5688) GRAY_CALIBRATION: 采样进度: 0/100 - 左: 4095, 右: 4095
I (5788) GRAY_CALIBRATION: 采样进度: 10/100 - 左: 4093, 右: 4092
I (6788) GRAY_CALIBRATION: 白色区域采集完成: 左=4095, 右=4095
```

### 校准效果测试
```
I (7890) GRAY_CALIBRATION: [  0.0秒] 左: 4095 (1.000) [白色] | 右: 4095 (1.000) [白色]
I (7990) GRAY_CALIBRATION: [  0.1秒] 左: 1476 (0.000) [黑线] | 右: 1546 (0.000) [黑线]
I (8090) GRAY_CALIBRATION: [  0.2秒] 左: 2800 (0.506) [白色] | 右: 2850 (0.512) [白色]
```

## 注意事项

1. **环境光影响**：确保校准时的光照条件与实际使用时一致
2. **表面清洁**：确保传感器和测试表面清洁，无灰尘或污渍
3. **传感器高度**：保持传感器与表面的距离一致（通常2-5mm）
4. **多次校准**：建议多次校准取平均值，提高准确性
5. **ADC2限制**：ADC2与WiFi共享硬件，使用时需禁用WiFi

## 故障排除

### 问题1：ADC值始终为4095或0
- **原因**：传感器未正确连接或GPIO配置错误
- **解决**：检查硬件连接，确认GPIO18和GPIO20正确连接到传感器

### 问题2：白色和黑线的ADC值差异很小
- **原因**：传感器距离表面太远，或表面对比度不够
- **解决**：调整传感器高度，使用对比度更高的测试表面

### 问题3：读取ADC失败
- **原因**：ADC初始化失败或WiFi占用ADC2
- **解决**：确保WiFi已禁用，检查ADC初始化代码

### 问题4：校准值不稳定
- **原因**：环境光变化或传感器抖动
- **解决**：在稳定光照下校准，固定传感器位置

## 技术参数

- **ADC分辨率**：12位（0-4095）
- **采样周期**：10ms（校准时）
- **校准采样次数**：100次
- **黑线检测阈值**：0.5（归一化值）
- **GPIO配置**：
  - 左传感器：GPIO18 (ADC2_CH7)
  - 右传感器：GPIO20 (ADC2_CH9)

## 相关文件

- `main/gray_calibration_test.c` - 校准测试程序源码
- `main/CMakeLists_calibration.txt` - 测试程序编译配置
- `components/gray_sensor/` - 灰度传感器组件
- `main/main.c` - 主程序（应用校准参数）

## 参考资料

- [灰度传感器组件文档](../components/gray_sensor/README.md)
- [ESP32 ADC驱动文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc_oneshot.html)
