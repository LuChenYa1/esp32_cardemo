# 灰度传感器 (Gray Sensor)

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: main.c使用中

---

## 组件简介

灰度传感器组件用于检测地面的黑白线条，是循迹小车的核心传感器模块。本组件通过ADC2读取两个灰度传感器的模拟信号，支持原始值读取、归一化处理和黑线检测功能。

组件提供两种工作模式：
1. **简化模式**：直接阻塞式读取ADC值，适用于单任务顺序执行
2. **高频采样模式**：创建后台采样任务，以3ms周期采样并缓存数据，适用于多任务实时系统

### 主要特性

- 双通道灰度传感器支持（左/右）
- ADC2硬件接口，12位分辨率（0-4095）
- 自动校准功能，支持白色区域和黑线区域校准
- 归一化处理，输出0.0-1.0范围的浮点值
- 黑线检测功能，可配置阈值
- 高频采样模式，3ms周期后台采样
- 原子变量保护，中断安全
- 错误处理机制，连续失败200次自动进入安全模式

### 适用场景

- 循迹小车的路径跟踪
- 黑白线条识别
- 地面颜色检测
- 需要高频实时采样的传感器应用

---

## 硬件连接

### 引脚分配

| 功能 | GPIO引脚 | ADC通道 | 说明 |
|------|---------|---------|------|
| 左灰度传感器 | GPIO18 | ADC2_CH7 | 模拟输入，检测左侧地面 |
| 右灰度传感器 | GPIO20 | ADC2_CH9 | 模拟输入，检测右侧地面 |

### 接线说明

1. 将左侧灰度传感器的信号输出连接到GPIO18
2. 将右侧灰度传感器的信号输出连接到GPIO20
3. 确保传感器的VCC连接到3.3V电源
4. 确保传感器的GND与ESP32共地

### 注意事项

- ⚠️ **ADC2与WiFi冲突**：ADC2在WiFi启动后无法使用，如需同时使用WiFi和灰度传感器，请改用ADC1通道
- ⚠️ **电压范围**：传感器输出电压应在0-3.3V范围内，超出范围可能损坏ESP32
- ⚠️ **传感器高度**：灰度传感器应距离地面5-10mm，过高或过低会影响检测精度
- ⚠️ **环境光影响**：强光或阴影可能影响检测结果，建议在稳定光照环境下使用

---

## 功能说明

### 核心功能

#### 功能1：原始ADC值读取

组件通过ESP-IDF的ADC OneShot API读取传感器的模拟信号，转换为12位数字值（0-4095）。支持单通道读取和双通道同时读取。

#### 功能2：传感器校准

提供交互式校准功能，引导用户将传感器放置在白色区域和黑线上，自动采集并保存校准参数。校准后可进行归一化处理。

#### 功能3：归一化处理

根据校准参数将ADC原始值映射到0.0-1.0范围：
- 0.0 = 黑线区域
- 1.0 = 白色区域
- 中间值 = 灰色区域

#### 功能4：黑线检测

基于归一化值和可配置阈值判断传感器是否在黑线上，默认阈值为0.5。

#### 功能5：高频采样模式

创建FreeRTOS后台任务，以3ms周期采样ADC值并缓存到原子变量中。其他任务可通过非阻塞方式读取缓存值，适用于实时控制系统。

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| 白色区域ADC值 | 3400 | 2000-4095 | 校准参数，传感器在白色区域的ADC值 |
| 黑线区域ADC值 | 800 | 0-2000 | 校准参数，传感器在黑线上的ADC值 |
| 黑线检测阈值 | 0.5 | 0.0-1.0 | 归一化值小于此阈值判定为黑线 |
| 采样周期 | 3ms | 1-100ms | 高频采样模式的采样间隔 |
| 错误阈值 | 200次 | 50-500次 | 连续读取失败超过此次数进入安全模式 |

---

## API接口

### 初始化函数

```c
/**
 * @brief 简化初始化灰度传感器（不启动采样任务）
 * 
 * 配置ADC2的通道7和通道9，不创建后台采样任务
 * 适用于单任务顺序执行模式
 */
void gray_sensor_init_simple(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
在单任务模式下调用此函数初始化ADC，然后使用`gray_sensor_read_both_raw_direct()`阻塞式读取数据。

---

```c
/**
 * @brief 初始化ADC采样任务（高频采样模式）
 * 
 * 初始化ADC2并创建采样任务，以3ms周期采样并缓存数据
 */
void gray_scanner_init(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
在多任务模式下调用此函数，会自动创建后台采样任务（优先级6）。初始化后使用`gray_scanner_get_cached_values()`非阻塞读取缓存数据。

---

### 数据读取函数

```c
/**
 * @brief 直接读取两个传感器的原始值（阻塞方式）
 * 
 * @param left_value 左传感器ADC值输出
 * @param right_value 右传感器ADC值输出
 * @return true 成功读取，false 读取失败
 */
bool gray_sensor_read_both_raw_direct(uint16_t *left_value, uint16_t *right_value);
```

**参数说明**：
- `left_value`: 指向uint16_t的指针，用于接收左传感器ADC值（0-4095）
- `right_value`: 指向uint16_t的指针，用于接收右传感器ADC值（0-4095）

**返回值**：
- `true`: 读取成功
- `false`: 读取失败（ADC未初始化或硬件错误）

**使用说明**：
此函数会阻塞执行直到ADC读取完成，适用于简化模式。

---

```c
/**
 * @brief 获取缓存的ADC值（中断安全）
 * 
 * 使用原子读取操作，可在中断中安全调用
 * 
 * @param left 左传感器ADC值输出
 * @param right 右传感器ADC值输出
 */
void gray_scanner_get_cached_values(uint16_t *left, uint16_t *right);
```

**参数说明**：
- `left`: 指向uint16_t的指针，用于接收左传感器缓存的ADC值
- `right`: 指向uint16_t的指针，用于接收右传感器缓存的ADC值

**返回值**：
- 无

**使用说明**：
此函数从原子变量读取缓存值，非阻塞，可在中断中安全调用。需先调用`gray_scanner_init()`启动采样任务。

---

### 校准函数

```c
/**
 * @brief 传感器校准 - 交互式校准
 * 
 * 引导用户将传感器放置在白色区域和黑线上进行校准
 */
void gray_sensor_calibrate(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
调用此函数后，按照串口提示将传感器依次放置在白色区域和黑线上。校准完成后会自动保存参数。

---

```c
/**
 * @brief 手动设置校准参数
 * 
 * @param channel 传感器通道
 * @param white_value 白色区域ADC值
 * @param black_value 黑线区域ADC值
 */
void gray_sensor_set_calibration(gray_sensor_channel_t channel, 
                                 uint16_t white_value, 
                                 uint16_t black_value);
```

**参数说明**：
- `channel`: 传感器通道（GRAY_SENSOR_LEFT 或 GRAY_SENSOR_RIGHT）
- `white_value`: 白色区域的ADC值（通常2000-4095）
- `black_value`: 黑线区域的ADC值（通常0-2000）

**返回值**：
- 无

**使用说明**：
如果已知校准参数，可直接调用此函数设置，无需交互式校准。

---

### 归一化读取函数

```c
/**
 * @brief 读取归一化后的传感器值
 * 
 * 根据校准数据将ADC值归一化到0.0-1.0范围
 * 0.0 = 黑线, 1.0 = 白色
 * 
 * @param channel 传感器通道
 * @return 归一化值 (0.0-1.0)，未校准返回-1.0
 */
float gray_sensor_read_normalized(gray_sensor_channel_t channel);
```

**参数说明**：
- `channel`: 传感器通道（GRAY_SENSOR_LEFT 或 GRAY_SENSOR_RIGHT）

**返回值**：
- 归一化值（0.0-1.0），读取失败返回-1.0

**使用说明**：
需先完成校准才能获得准确的归一化值。未校准时使用默认参数。

---

### 黑线检测函数

```c
/**
 * @brief 判断传感器是否在黑线上
 * 
 * @param channel 传感器通道
 * @param threshold 阈值 (0.0-1.0)，默认0.5
 * @return true=在黑线上, false=在白色区域
 */
bool gray_sensor_is_on_black_line(gray_sensor_channel_t channel, float threshold);
```

**参数说明**：
- `channel`: 传感器通道（GRAY_SENSOR_LEFT 或 GRAY_SENSOR_RIGHT）
- `threshold`: 判定阈值（0.0-1.0），归一化值小于此值判定为黑线

**返回值**：
- `true`: 传感器在黑线上
- `false`: 传感器在白色区域或读取失败

**使用说明**：
阈值可根据实际环境调整，默认0.5适用于大多数场景。

---

### 调试函数

```c
/**
 * @brief 打印传感器状态 (调试用)
 */
void gray_sensor_print_status(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
打印左右传感器的原始值、归一化值和黑线检测结果，用于调试。

---

```c
/**
 * @brief 获取ADC错误计数
 * 
 * @return 累计的ADC读取错误次数
 */
uint32_t gray_scanner_get_error_count(void);
```

**参数说明**：
- 无

**返回值**：
- 累计的ADC读取错误次数

**使用说明**：
用于监控ADC读取的可靠性，错误计数持续增加表示硬件或配置问题。

---

## 使用示例

### 基本使用示例（简化模式）

```c
#include "gray_sensor.h"
#include "esp_log.h"

static const char *TAG = "EXAMPLE";

void example_basic_usage(void)
{
    // 1. 初始化灰度传感器（简化模式）
    gray_sensor_init_simple();
    
    // 2. 读取原始ADC值
    uint16_t left_value, right_value;
    if (gray_sensor_read_both_raw_direct(&left_value, &right_value)) {
        ESP_LOGI(TAG, "左传感器: %d, 右传感器: %d", left_value, right_value);
    } else {
        ESP_LOGE(TAG, "读取传感器失败");
    }
    
    // 3. 读取归一化值
    float left_norm = gray_sensor_read_normalized(GRAY_SENSOR_LEFT);
    float right_norm = gray_sensor_read_normalized(GRAY_SENSOR_RIGHT);
    ESP_LOGI(TAG, "归一化值 - 左: %.3f, 右: %.3f", left_norm, right_norm);
    
    // 4. 检测黑线
    bool left_on_black = gray_sensor_is_on_black_line(GRAY_SENSOR_LEFT, 0.5);
    bool right_on_black = gray_sensor_is_on_black_line(GRAY_SENSOR_RIGHT, 0.5);
    ESP_LOGI(TAG, "黑线检测 - 左: %s, 右: %s", 
             left_on_black ? "是" : "否",
             right_on_black ? "是" : "否");
}
```

### 高级使用示例（高频采样模式）

```c
#include "gray_sensor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "EXAMPLE";

void control_task(void *pvParameters)
{
    while (1) {
        // 从缓存读取ADC值（非阻塞）
        uint16_t left, right;
        gray_scanner_get_cached_values(&left, &right);
        
        ESP_LOGI(TAG, "缓存值 - 左: %d, 右: %d", left, right);
        
        // 检查错误计数
        uint32_t errors = gray_scanner_get_error_count();
        if (errors > 0) {
            ESP_LOGW(TAG, "ADC错误计数: %lu", errors);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void example_high_frequency_mode(void)
{
    // 1. 初始化高频采样模式
    gray_scanner_init();
    
    // 2. 创建控制任务
    xTaskCreate(control_task, "control", 2048, NULL, 5, NULL);
    
    // 采样任务会在后台以3ms周期运行
}
```

### 校准示例

```c
#include "gray_sensor.h"

void example_calibration(void)
{
    // 方法1：交互式校准
    gray_sensor_calibrate();
    
    // 方法2：手动设置校准参数
    gray_sensor_set_calibration(GRAY_SENSOR_LEFT, 3400, 800);
    gray_sensor_set_calibration(GRAY_SENSOR_RIGHT, 3400, 800);
}
```

---

## 注意事项

### 硬件限制

- ⚠️ **ADC2与WiFi互斥**：ESP32的ADC2在WiFi启动后无法使用，如需同时使用WiFi，请改用ADC1通道
- ⚠️ **GPIO6-11不可用**：这些引脚连接到SPI Flash，不能用于ADC
- ⚠️ **输入电压范围**：ADC输入电压必须在0-3.3V范围内，超出范围会损坏芯片

### 软件限制

- ⚠️ **阻塞读取**：`gray_sensor_read_both_raw_direct()`会阻塞执行，不适合在中断或高优先级任务中调用
- ⚠️ **校准必要性**：未校准时使用默认参数，可能导致归一化值不准确
- ⚠️ **任务优先级**：高频采样任务优先级为6，高于语音任务（4），确保实时采样不被阻塞

### 线程安全

- ✅ `gray_scanner_get_cached_values()`使用原子变量，线程安全，可在中断中调用
- ❌ `gray_sensor_read_both_raw_direct()`不是线程安全的，不应在多个任务中同时调用
- ❌ 校准函数不是线程安全的，应在初始化阶段单线程调用

### 性能考虑

- 高频采样模式的采样周期为3ms，可根据需要调整（修改`adc_sampling_task`中的延时）
- 采样任务栈大小为2048字节，如需更复杂的处理可适当增加
- 连续读取失败200次会触发安全模式，可根据系统可靠性要求调整阈值

---

## 故障排除

### 常见问题

#### 问题1：读取ADC值始终为4095或0

**现象**：调用`gray_sensor_read_both_raw_direct()`返回的值始终为最大值4095或最小值0

**原因**：
1. 传感器未正确连接或电源未接通
2. GPIO引脚配置错误
3. 传感器损坏

**解决方案**：
1. 检查传感器的VCC、GND和信号线连接
2. 使用万用表测量传感器输出电压，应在0-3.3V范围内
3. 确认GPIO18和GPIO20未被其他组件占用
4. 更换传感器测试

#### 问题2：WiFi启动后ADC读取失败

**现象**：WiFi初始化后，`gray_sensor_read_both_raw_direct()`返回false

**原因**：ESP32的ADC2与WiFi驱动共享硬件资源，WiFi启动后ADC2无法使用

**解决方案**：
1. 改用ADC1通道（GPIO32-39）
2. 或者不使用WiFi功能
3. 或者在WiFi启动前完成所有ADC2读取操作

#### 问题3：归一化值不准确

**现象**：明明在黑线上，但归一化值显示为白色区域

**原因**：
1. 未进行校准或校准参数不正确
2. 环境光照条件变化
3. 传感器高度不合适

**解决方案**：
1. 重新执行`gray_sensor_calibrate()`进行校准
2. 在稳定光照环境下使用
3. 调整传感器距离地面的高度（推荐5-10mm）
4. 手动设置校准参数：`gray_sensor_set_calibration()`

#### 问题4：高频采样模式错误计数持续增加

**现象**：`gray_scanner_get_error_count()`返回值持续增加

**原因**：
1. WiFi已启动，导致ADC2无法使用
2. 硬件连接不稳定
3. 任务优先级过低，被其他任务抢占

**解决方案**：
1. 确认WiFi未启动
2. 检查硬件连接的稳定性
3. 提高采样任务优先级（当前为6）
4. 检查系统日志，查看具体错误信息

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [GPIO引脚分配文档](../../docs/GPIO_PIN_ALLOCATION.md)
- [配置参数指南](../../docs/CONFIGURATION_GUIDE.md)
- [板级配置组件](../board_config/README.md)

### 数据手册

- ESP32-S3技术参考手册 - ADC章节
- 灰度传感器模块数据手册（具体型号根据实际硬件）

### 代码示例

- 测试程序：`main/main.c` - 包含灰度传感器的完整使用示例
- PD控制器：`components/pd_controller/` - 使用灰度传感器进行循迹控制

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，完整的API文档和使用说明 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/gray_sensor/`  
**文档类型**: 组件使用说明
