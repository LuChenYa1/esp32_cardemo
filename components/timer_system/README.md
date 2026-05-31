# Timer System 定时器系统

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: main.c使用中

---

## 组件简介

Timer System（定时器系统）是ESP32-S3巡线小车的核心实时调度模块，基于ESP32硬件通用定时器（GP Timer）实现。该模块提供两个独立的硬件定时器，用于高精度、低延迟的实时任务调度，是整个控制系统的心跳。

### 主要特性

- 使用ESP32 GP Timer（通用定时器），1MHz分辨率（1us精度）
- Timer 0：3ms周期，用于灰度传感器扫描和转弯检测
- Timer 1：10ms周期，用于PD控制算法和转弯状态机
- 自动重载模式，精确周期控制
- 中断处理函数使用IRAM属性（避免cache miss）
- 中断计数器和时间戳记录（用于调试和性能测试）
- 独立的启动/停止控制
- 完整的错误处理和安全模式
- 看门狗定时器支持（5秒超时，自动重启）

### 适用场景

- 需要高精度实时控制的嵌入式系统
- 巡线小车、机器人等需要快速响应的控制系统
- 多任务实时调度场景
- 需要硬件定时器保证时序的应用

---

## 硬件连接

### 引脚分配

本组件不直接使用GPIO引脚，而是通过硬件定时器中断调用其他模块的功能。

| 功能 | GPIO引脚 | 接口标识 | 说明 |
|------|---------|---------|------|
| 无直接GPIO | - | - | 使用ESP32内部硬件定时器 |

### 定时器配置

| 定时器 | 周期 | 分辨率 | 中断优先级 | 用途 |
|--------|------|--------|-----------|------|
| Timer 0 | 3ms | 1us | 1（最高） | 灰度扫描、转弯检测 |
| Timer 1 | 10ms | 1us | 2（次高） | PD控制、状态机 |

### 注意事项

- ⚠️ 定时器使用ESP32内部硬件资源，不占用GPIO引脚
- ⚠️ 中断处理函数必须快速执行，避免阻塞其他中断
- ⚠️ Timer 0优先级高于Timer 1，确保转弯检测的实时性

---

## 功能说明

### 核心功能

#### 功能1：Timer 0 - 高频转弯检测（3ms周期）

Timer 0以3ms周期触发中断，执行以下任务：
1. 调用`turn_detector_tick()`执行转弯检测逻辑
2. 从共享变量读取ADC缓存值（由ADC采样任务更新）
3. 根据灰度传感器数据判断是否需要转弯
4. 设置转弯标志供Timer 1使用

**工作原理**：
- 使用IRAM_ATTR属性，确保代码在IRAM中执行（避免cache miss）
- 禁止调用任何阻塞API（vTaskDelay、printf、mutex等）
- 禁止直接调用ADC读取（会导致中断延迟）
- 通过原子变量与ADC采样任务通信

#### 功能2：Timer 1 - PD控制和状态机（10ms周期）

Timer 1以10ms周期触发中断，执行以下任务：
1. 调用`pd_controller_tick()`执行PD控制算法
2. 调用`turn_statemachine_tick()`执行转弯状态机
3. 根据灰度传感器数据计算电机速度
4. 控制电机执行转弯动作

**工作原理**：
- 检查转弯标志，转弯期间暂停PD控制
- 读取ADC缓存值和转弯请求
- 执行状态转换和电机控制
- 在转弯完成后清除转弯标志

#### 功能3：安全模式

当系统检测到严重错误（如ADC连续失败100次）时，自动进入安全模式：
1. 停止所有电机（设置速度为0）
2. 记录错误日志
3. 可选：触发蜂鸣器报警（如果已初始化）
4. 进入死循环等待重启

**注意**：`enter_safe_mode()`函数不会返回。

#### 功能4：看门狗定时器

配置Task Watchdog Timer (TWDT)提供最后一道防线：
- 超时时间：5秒
- 监控核心0的IDLE任务
- 超时触发panic重启

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| TIMER_0_INTERVAL_MS | 3 | 1-10 | Timer 0周期（毫秒） |
| TIMER_1_INTERVAL_MS | 10 | 5-50 | Timer 1周期（毫秒） |
| TIMER_0_PRIORITY | 1 | 1-7 | Timer 0中断优先级（1最高） |
| TIMER_1_PRIORITY | 2 | 1-7 | Timer 1中断优先级 |
| WATCHDOG_TIMEOUT_MS | 5000 | 1000-10000 | 看门狗超时时间（毫秒） |

---

## API接口

### 初始化函数

```c
/**
 * @brief 初始化定时器系统
 * 
 * 配置Timer 0和Timer 1，注册中断处理函数，但不启动定时器
 * 
 * @return 
 *   - ESP_OK: 初始化成功
 *   - ESP_FAIL: 初始化失败
 */
esp_err_t timer_system_init(void);
```

**参数说明**：
- 无

**返回值**：
- `ESP_OK`: 初始化成功
- `ESP_FAIL`: 初始化失败（会记录详细错误日志）

**使用说明**：
在所有外设初始化完成后调用，但在启动定时器之前。初始化失败时应停止系统启动。

---

### 主要功能函数

#### 启动定时器

```c
/**
 * @brief 启动定时器系统
 * 
 * 同时启动Timer 0和Timer 1
 * 
 * @return 
 *   - ESP_OK: 启动成功
 *   - ESP_FAIL: 启动失败
 */
esp_err_t timer_system_start(void);
```

**参数说明**：
- 无

**返回值**：
- `ESP_OK`: 启动成功
- `ESP_FAIL`: 启动失败

**使用说明**：
在所有模块初始化完成后调用，开始实时控制。

#### 停止定时器

```c
/**
 * @brief 停止定时器系统
 * 
 * 同时停止Timer 0和Timer 1
 * 
 * @return 
 *   - ESP_OK: 停止成功
 *   - ESP_FAIL: 停止失败
 */
esp_err_t timer_system_stop(void);
```

**参数说明**：
- 无

**返回值**：
- `ESP_OK`: 停止成功
- `ESP_FAIL`: 停止失败

**使用说明**：
用于调试或紧急停止。停止后可以重新启动。

#### 获取中断计数

```c
/**
 * @brief 获取Timer 0中断计数器（用于调试）
 * 
 * @return Timer 0中断次数
 */
uint32_t timer_system_get_timer0_count(void);

/**
 * @brief 获取Timer 1中断计数器（用于调试）
 * 
 * @return Timer 1中断次数
 */
uint32_t timer_system_get_timer1_count(void);
```

**参数说明**：
- 无

**返回值**：
- 中断触发次数（从启动开始累计）

**使用说明**：
用于监控定时器运行状态和性能测试。Timer 0应该每秒触发约333次（3ms周期），Timer 1应该每秒触发100次（10ms周期）。

#### 安全模式

```c
/**
 * @brief 进入安全模式
 * 
 * 当系统检测到严重错误时调用此函数：
 * - 停止所有电机（设置速度为0）
 * - 记录错误日志
 * - 可选：触发蜂鸣器报警
 * - 进入死循环等待重启
 * 
 * 注意：此函数不会返回
 */
void enter_safe_mode(void);
```

**参数说明**：
- 无

**返回值**：
- 无（函数不会返回）

**使用说明**：
仅在检测到严重错误时调用，如ADC连续失败、传感器故障等。

#### 看门狗初始化

```c
/**
 * @brief 初始化看门狗定时器
 * 
 * 配置Task Watchdog Timer (TWDT)：
 * - 超时时间：5秒
 * - 监控IDLE任务
 * - 超时触发panic重启
 * 
 * @return 
 *   - ESP_OK: 初始化成功
 *   - ESP_FAIL: 初始化失败
 */
esp_err_t watchdog_init(void);
```

**参数说明**：
- 无

**返回值**：
- `ESP_OK`: 初始化成功
- `ESP_FAIL`: 初始化失败

**使用说明**：
建议在启动定时器之前调用。看门狗初始化失败不是致命错误，系统可以继续运行但没有看门狗保护。

---

## 使用示例

### 基本使用示例

```c
#include "timer_system.h"
#include "esp_log.h"

static const char *TAG = "TIMER_EXAMPLE";

void app_main(void)
{
    // 1. 初始化定时器系统
    esp_err_t ret = timer_system_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "定时器初始化失败");
        return;
    }
    ESP_LOGI(TAG, "定时器初始化成功");
    
    // 2. 启动定时器
    ret = timer_system_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "定时器启动失败");
        return;
    }
    ESP_LOGI(TAG, "定时器已启动");
    
    // 3. 监控中断计数（每秒打印一次）
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        uint32_t count0 = timer_system_get_timer0_count();
        uint32_t count1 = timer_system_get_timer1_count();
        
        ESP_LOGI(TAG, "Timer 0: %lu次, Timer 1: %lu次", count0, count1);
        // 预期：Timer 0约333次/秒，Timer 1约100次/秒
    }
}
```

### 高级使用示例 - 集成到巡线系统

```c
#include "timer_system.h"
#include "board_config.h"
#include "gray_sensor.h"
#include "turn_detector.h"
#include "pd_controller.h"
#include "turn_statemachine.h"
#include "pwm.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    esp_err_t ret;
    
    // 1. 初始化GPIO管理器
    ESP_LOGI(TAG, "初始化GPIO管理器...");
    ret = board_config_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO管理器初始化失败");
        return;
    }
    
    // 2. 初始化所有外设
    ESP_LOGI(TAG, "初始化硬件外设...");
    gray_sensor_init_simple();  // 灰度传感器
    ledc_init();                 // PWM电机控制
    
    // 3. 创建ADC采样任务
    ESP_LOGI(TAG, "创建ADC采样任务...");
    gray_scanner_init();
    
    // 4. 初始化控制模块
    ESP_LOGI(TAG, "初始化控制模块...");
    turn_detector_init();        // 转弯检测
    pd_controller_init();        // PD控制器
    turn_statemachine_init();    // 转弯状态机
    
    // 5. 初始化定时器系统
    ESP_LOGI(TAG, "初始化定时器系统...");
    ret = timer_system_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "定时器初始化失败，停止启动");
        return;
    }
    
    // 6. 配置看门狗（可选但推荐）
    ESP_LOGI(TAG, "配置看门狗定时器...");
    ret = watchdog_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "看门狗初始化失败，系统将继续运行但没有看门狗保护");
    }
    
    // 7. 等待所有任务启动
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // 8. 启动定时器（开始实时控制）
    ESP_LOGI(TAG, "启动定时器系统...");
    ret = timer_system_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "定时器启动失败，停止启动");
        return;
    }
    
    ESP_LOGI(TAG, "系统启动完成！");
    
    // 9. 主循环可以处理其他任务
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // 可选：监控系统状态
        uint32_t count0 = timer_system_get_timer0_count();
        uint32_t count1 = timer_system_get_timer1_count();
        ESP_LOGI(TAG, "定时器运行正常 - Timer0: %lu, Timer1: %lu", count0, count1);
    }
}
```

---

## 注意事项

### 硬件限制

- ⚠️ ESP32-S3有4个通用定时器（GP Timer），本模块使用其中2个
- ⚠️ 定时器资源有限，避免在其他模块中重复创建定时器
- ⚠️ 中断优先级设置需要与其他中断协调，避免冲突

### 软件限制

- ⚠️ **中断中禁止阻塞API**：不能调用`vTaskDelay`、`printf`、`mutex`等阻塞函数
- ⚠️ **中断中禁止直接ADC读取**：`adc_oneshot_read()`会导致中断延迟，必须使用ADC缓存值
- ⚠️ **使用原子操作**：访问共享变量必须使用`_Atomic`或临界区保护
- ⚠️ **IRAM属性必须**：中断函数必须使用`IRAM_ATTR`属性，确保代码在IRAM中执行
- ⚠️ **中断执行时间限制**：Timer 0中断应<500us，Timer 1中断应<2ms

### 线程安全

- 定时器中断与FreeRTOS任务并发执行，需要注意数据同步
- 使用原子变量（`_Atomic`）保护共享数据
- 中断中不能使用互斥锁（mutex），会导致死锁
- 中断计数器使用`volatile`修饰，确保可见性

### 性能考虑

- Timer 0周期3ms，每秒触发约333次，中断开销约0.17ms/次
- Timer 1周期10ms，每秒触发100次，中断开销约0.5ms/次
- 总中断开销约占CPU时间的5-10%
- 周期精度：±10%以内（受系统负载影响）
- 建议监控中断计数，确保定时器正常工作

---

## 故障排除

### 常见问题

#### 问题1：定时器初始化失败

**现象**：`timer_system_init()`返回`ESP_FAIL`，日志显示"Timer 0创建失败"或"Timer 1创建失败"

**原因**：
1. 定时器资源已被其他模块占用
2. 内存不足
3. 定时器配置参数错误

**解决方案**：
1. 检查是否有其他模块创建了GP Timer
2. 增加堆内存大小（menuconfig -> Component config -> ESP32-specific -> Minimum free heap size）
3. 检查定时器配置参数是否正确

#### 问题2：中断频率异常

**现象**：监控中断计数发现Timer 0不是约333次/秒，或Timer 1不是100次/秒

**原因**：
1. 中断处理函数执行时间过长，导致中断丢失
2. 系统负载过高，影响定时器精度
3. 定时器配置错误

**解决方案**：
1. 优化中断处理函数，减少执行时间
2. 降低系统负载，减少其他任务的CPU占用
3. 检查定时器周期配置（TIMER_0_INTERVAL_MS、TIMER_1_INTERVAL_MS）
4. 使用示波器或逻辑分析仪测量实际中断频率

#### 问题3：系统进入安全模式

**现象**：日志显示"系统进入安全模式！"，所有电机停止，系统进入死循环

**原因**：
1. ADC连续失败超过100次
2. 其他关键传感器故障

**解决方案**：
1. 检查灰度传感器硬件连接
2. 检查ADC采样任务是否正常运行
3. 查看日志中的ADC错误计数
4. 如果是硬件故障，修复后重启系统

#### 问题4：看门狗超时重启

**现象**：系统运行一段时间后自动重启，日志显示"Task watchdog got triggered"

**原因**：
1. IDLE任务被阻塞超过5秒
2. 中断处理函数执行时间过长
3. 系统负载过高，IDLE任务无法运行

**解决方案**：
1. 检查是否有任务长时间占用CPU（使用`vTaskDelay`让出CPU）
2. 优化中断处理函数，减少执行时间
3. 降低系统负载，减少任务数量或降低任务优先级
4. 如果确实需要长时间运行，可以增加看门狗超时时间（修改`watchdog_init()`中的配置）

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [GPIO引脚分配文档](../../docs/GPIO_PIN_ALLOCATION.md)
- [配置参数指南](../../docs/CONFIGURATION_GUIDE.md)
- [STM32到ESP32迁移规格](.kiro/specs/stm32-to-esp32-migration/)

### 数据手册

- [ESP-IDF GP Timer API文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gptimer.html)
- [ESP32-S3技术参考手册](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)

### 代码示例

- [基础测试程序](../../main/main_timer_test.c)
- [看门狗示例](../../main/main_timer_watchdog_example.c)
- [完整集成示例](../../main/main.c)

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，完成定时器系统基础功能 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/timer_system/`  
**文档类型**: 组件使用说明
