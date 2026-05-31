# PCF蜂鸣器音乐播放模块

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: ⏸️ 未使用

---

## 组件简介

PCF蜂鸣器模块通过PCF8574 I2C GPIO扩展芯片驱动蜂鸣器，使用软件PWM技术播放音乐。该模块支持多音符序列播放，可调节播放速度和占空比，适用于音乐播放、提示音等应用场景。

### 主要特性

- 基于PCF8574 I2C GPIO扩展芯片
- 软件PWM驱动蜂鸣器
- 支持音符序列播放（低音、中音、高音）
- 可调节播放速度（BPM）和占空比
- 支持休止符和不同音符时长
- 非阻塞播放，使用FreeRTOS任务管理
- 内置音乐数据示例

### 适用场景

适用于需要音乐播放或提示音的场景，如开机音乐、按键提示音、报警音等。通过I2C扩展GPIO，节省ESP32的直接GPIO资源。

---

## 硬件连接

### 引脚分配

| 功能 | 连接 | 说明 |
|------|------|------|
| 蜂鸣器+ | PCF8574 P0-P7（可配置） | 蜂鸣器正极 |
| 蜂鸣器- | GND | 蜂鸣器负极 |
| PCF8574 SDA | ESP32 I2C SDA | I2C数据线 |
| PCF8574 SCL | ESP32 I2C SCL | I2C时钟线 |

### 接线说明

1. 将PCF8574初始化并连接到ESP32的I2C总线
2. 将蜂鸣器正极连接到PCF8574的任意GPIO引脚（P0-P7）
3. 将蜂鸣器负极连接到GND
4. 如果蜂鸣器电流较大，建议使用三极管驱动电路

### 驱动电路（推荐）

```
PCF8574 Px ──┬── 1kΩ ──┬── NPN三极管基极
             │         │
             └─────────┴── 10kΩ下拉电阻 ── GND
                       
三极管集电极 ── 蜂鸣器+ ── VCC
三极管发射极 ── GND
```

### 注意事项

- ⚠️ **电流限制**：PCF8574每个引脚最大输出25mA，大功率蜂鸣器需要外部驱动
- ⚠️ **I2C速度**：使用400kHz I2C速度，测试PCF8574性能
- ⚠️ **软件PWM**：通过I2C快速切换GPIO实现PWM，频率受I2C速度限制
- ⚠️ **依赖组件**：需要先初始化PCF8574组件

---

## 功能说明

### 核心功能

#### 功能1：蜂鸣器初始化

配置PCF8574设备、蜂鸣器引脚、占空比和播放速度（BPM）。创建PWM生成任务和音乐播放任务。

#### 功能2：音符序列播放

支持播放音符数组，每个音符包含频率和持续时间。支持低音（L1-L7）、中音（M1-M7）、高音（H1-H7）和休止符（P0）。

#### 功能3：播放控制

提供播放、停止、速度调节等控制功能。播放状态查询功能可用于判断是否正在播放。

#### 功能4：软件PWM生成

通过FreeRTOS任务快速切换PCF8574 GPIO电平，生成指定频率的PWM信号驱动蜂鸣器。

### 音符定义

| 音符类型 | 音符 | 频率(Hz) | 说明 |
|---------|------|---------|------|
| 休止符 | P0 | 0 | 静音 |
| 低音 | L1-L7 | 262-494 | Do-Si |
| 中音 | M1-M7 | 523-988 | Do-Si |
| 高音 | H1-H7 | 1047-1976 | Do-Si |

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| pcf_device | 必需 | - | PCF8574设备句柄 |
| buzzer_pin | 0-7 | 0-7 | 蜂鸣器连接的PCF8574引脚 |
| duty_cycle | 50 | 0-100 | PWM占空比（%） |
| bpm | 120 | 60-240 | 播放速度（每分钟拍数） |

---

## API接口

### 初始化函数

```c
/**
 * @brief 初始化PCF蜂鸣器
 * 
 * @param config 蜂鸣器配置
 * @return ESP_OK 成功，其他值失败
 */
esp_err_t pcf_buzzer_init(const pcf_buzzer_config_t *config);
```

**参数说明**：
- `config`: 蜂鸣器配置结构体指针
  - `pcf_device`: PCF8574设备句柄（需先初始化PCF8574）
  - `buzzer_pin`: 蜂鸣器引脚号（0-7）
  - `duty_cycle`: PWM占空比（0-100）
  - `bpm`: 播放速度（每分钟拍数）

**返回值**：
- `ESP_OK`: 初始化成功
- `ESP_ERR_INVALID_ARG`: 参数无效
- `ESP_ERR_NO_MEM`: 内存不足

**使用说明**：
在使用蜂鸣器前必须先初始化PCF8574设备，然后调用此函数初始化蜂鸣器。

---

### 播放函数

```c
/**
 * @brief 播放音符序列
 * 
 * @param notes 音符数组
 * @param note_count 音符数量
 * @return ESP_OK 成功，其他值失败
 */
esp_err_t pcf_buzzer_play(const Note *notes, size_t note_count);
```

**参数说明**：
- `notes`: 音符数组指针
- `note_count`: 音符数量

**返回值**：
- `ESP_OK`: 播放成功
- `ESP_ERR_INVALID_ARG`: 参数无效
- `ESP_ERR_INVALID_STATE`: 蜂鸣器未初始化

**使用说明**：
音符数组中每个元素包含频率和持续时间。播放是非阻塞的，函数立即返回。

---

### 停止函数

```c
/**
 * @brief 停止播放
 * 
 * @return ESP_OK 成功
 */
esp_err_t pcf_buzzer_stop(void);
```

**参数说明**：
- 无

**返回值**：
- `ESP_OK`: 停止成功

**使用说明**：
停止当前正在播放的音乐，清空播放队列。

---

### 状态查询函数

```c
/**
 * @brief 检查是否正在播放
 * 
 * @return true 正在播放，false 未播放
 */
bool pcf_buzzer_is_playing(void);
```

**参数说明**：
- 无

**返回值**：
- `true`: 正在播放
- `false`: 未播放

**使用说明**：
可用于判断播放是否结束，或避免重复播放。

---

### 速度设置函数

```c
/**
 * @brief 设置播放速度（BPM）
 * 
 * @param bpm 每分钟拍数
 * @return ESP_OK 成功
 */
esp_err_t pcf_buzzer_set_bpm(uint16_t bpm);
```

**参数说明**：
- `bpm`: 播放速度（每分钟拍数，建议60-240）

**返回值**：
- `ESP_OK`: 设置成功

**使用说明**：
可在播放过程中动态调整播放速度。

---

### 反初始化函数

```c
/**
 * @brief 反初始化PCF蜂鸣器
 * 
 * @return ESP_OK 成功
 */
esp_err_t pcf_buzzer_deinit(void);
```

**参数说明**：
- 无

**返回值**：
- `ESP_OK`: 反初始化成功

**使用说明**：
释放蜂鸣器资源，删除相关任务。

---

## 使用示例

### 基本使用示例

```c
#include "pcf_buzzer.h"
#include "pcf8574.h"

void pcf_buzzer_example(void)
{
    // 1. 初始化PCF8574设备
    i2c_dev_t pcf_dev;
    pcf8574_init_desc(&pcf_dev, 0x20, I2C_NUM_0, GPIO_NUM_21, GPIO_NUM_22);
    
    // 2. 配置蜂鸣器
    pcf_buzzer_config_t buzzer_config = {
        .pcf_device = &pcf_dev,
        .buzzer_pin = 0,        // 使用P0引脚
        .duty_cycle = 50,       // 50%占空比
        .bpm = 120              // 120 BPM
    };
    
    // 3. 初始化蜂鸣器
    esp_err_t ret = pcf_buzzer_init(&buzzer_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "蜂鸣器初始化失败");
        return;
    }
    
    // 4. 定义简单音符序列
    Note simple_melody[] = {
        {M1, 1}, {M2, 1}, {M3, 1}, {M4, 1},
        {M5, 1}, {M6, 1}, {M7, 1}, {H1, 2}
    };
    
    // 5. 播放音乐
    pcf_buzzer_play(simple_melody, sizeof(simple_melody) / sizeof(Note));
    
    // 6. 等待播放完成
    while (pcf_buzzer_is_playing()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // 7. 清理资源
    pcf_buzzer_deinit();
    pcf8574_free_desc(&pcf_dev);
}
```

### 播放内置音乐示例

```c
#include "pcf_buzzer.h"
#include "music_data.h"

void play_builtin_music(void)
{
    // 初始化PCF8574和蜂鸣器（省略）
    
    // 播放内置音乐
    pcf_buzzer_play(MoChouXiang, MOCHOUXIANG_NOTE_COUNT);
    
    ESP_LOGI(TAG, "正在播放音乐...");
}
```

### 动态调整速度示例

```c
void adjust_speed_example(void)
{
    // 初始化并开始播放（省略）
    
    // 播放过程中调整速度
    vTaskDelay(pdMS_TO_TICKS(5000));
    pcf_buzzer_set_bpm(150);  // 加快到150 BPM
    ESP_LOGI(TAG, "加快播放速度");
    
    vTaskDelay(pdMS_TO_TICKS(5000));
    pcf_buzzer_set_bpm(90);   // 减慢到90 BPM
    ESP_LOGI(TAG, "减慢播放速度");
}
```

### 提示音示例

```c
void beep_example(void)
{
    // 短促的提示音
    Note beep[] = {
        {M5, 0.25f}, {P0, 0.25f},
        {M5, 0.25f}, {P0, 0.25f}
    };
    
    pcf_buzzer_play(beep, sizeof(beep) / sizeof(Note));
}

void alarm_example(void)
{
    // 报警音
    Note alarm[] = {
        {H1, 0.5f}, {H5, 0.5f},
        {H1, 0.5f}, {H5, 0.5f},
        {H1, 0.5f}, {H5, 0.5f}
    };
    
    pcf_buzzer_play(alarm, sizeof(alarm) / sizeof(Note));
}
```

---

## 注意事项

### 硬件限制

- ⚠️ **电流限制**：PCF8574单引脚最大25mA，大功率蜂鸣器需要外部驱动电路
- ⚠️ **频率限制**：软件PWM频率受I2C速度限制，高频音符可能失真
- ⚠️ **音质**：软件PWM音质不如硬件PWM，适合简单音乐播放

### 软件限制

- ⚠️ **I2C占用**：播放时会频繁使用I2C总线，可能影响其他I2C设备
- ⚠️ **CPU占用**：软件PWM需要占用一定CPU资源
- ⚠️ **任务优先级**：PWM生成任务优先级较高，避免被其他任务阻塞

### 线程安全

- 播放控制函数不是线程安全的，多线程调用需要加锁保护
- 不要在中断中调用播放函数

### 性能考虑

- I2C速度设置为400kHz以获得最佳性能
- 占空比建议设置为50%，过高或过低会影响音质
- BPM建议范围60-240，过快或过慢会影响播放效果

---

## 故障排除

### 常见问题

#### 问题1：无声音输出

**现象**：蜂鸣器不发声

**原因**：硬件连接错误或蜂鸣器损坏

**解决方案**：
1. 检查蜂鸣器是否正确连接到PCF8574引脚
2. 使用万用表测量PCF8574引脚是否有电平变化
3. 确认蜂鸣器类型（有源/无源），本模块适用于无源蜂鸣器
4. 检查蜂鸣器是否需要外部驱动电路

#### 问题2：音调不准

**现象**：播放的音调与预期不符

**原因**：I2C速度不足或PWM频率不准确

**解决方案**：
1. 确认I2C速度设置为400kHz
2. 检查系统负载，避免任务阻塞PWM生成
3. 调整PWM任务优先级
4. 使用逻辑分析仪检查实际PWM频率

#### 问题3：播放卡顿

**现象**：音乐播放不流畅，有停顿

**原因**：I2C总线冲突或任务调度问题

**解决方案**：
1. 避免在播放时频繁访问I2C总线
2. 提高PWM任务优先级
3. 检查是否有其他高优先级任务阻塞
4. 减少系统负载

#### 问题4：初始化失败

**现象**：pcf_buzzer_init()返回错误

**原因**：PCF8574未初始化或参数错误

**解决方案**：
1. 确认PCF8574已正确初始化
2. 检查蜂鸣器引脚号是否在0-7范围内
3. 检查占空比是否在0-100范围内
4. 确认有足够的内存创建任务

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [PCF8574组件文档](../pcf8574/README.md)

### 数据手册

- PCF8574数据手册
- 无源蜂鸣器规格书

### 代码示例

- 示例代码：`components/pcf_buzzer/pcf_buzzer.c` - 完整的驱动实现
- 音乐数据：`components/pcf_buzzer/include/music_data.h` - 内置音乐示例

### 相关组件

- [pcf8574](../pcf8574/README.md) - PCF8574 I2C GPIO扩展驱动
- [buzzer](../buzzer/README.md) - 直接GPIO驱动蜂鸣器

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，支持音符序列播放 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/pcf_buzzer/`  
**文档类型**: 组件使用说明
