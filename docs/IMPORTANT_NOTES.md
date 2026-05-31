# 项目注意事项

## 文档说明

本文档记录ESP32巡线小车项目的关键注意事项、已知问题、限制条件和最佳实践。这些信息对于项目开发、调试和维护至关重要。

**更新日期**: 2024-12-20  
**项目**: ESP32巡线小车  
**文档版本**: 1.0

### 重要性分级

- 🔴 **关键（Critical）**：必须遵守，违反可能导致系统故障或硬件损坏
- 🟡 **重要（Important）**：强烈建议遵守，违反可能导致功能异常
- 🟢 **一般（General）**：建议遵守，有助于提高系统稳定性

---

## 硬件限制

### 🔴 关键限制

#### 1. ADC2与WiFi冲突

**问题描述**：
ESP32-S3的ADC2与WiFi驱动共享硬件资源，WiFi启动后ADC2无法使用。

**影响范围**：
- GPIO0-20的ADC2通道（ADC2_CH0-CH9）
- 当前灰度传感器使用GPIO18/20（ADC2_CH7/CH9）

**解决方案**：
```c
// 方案1：不使用WiFi（当前项目采用）
// 保持ADC2可用

// 方案2：改用ADC1通道（推荐）
#define GRAY_SENSOR_LEFT_GPIO    GPIO_NUM_47  // ADC1_CH6
#define GRAY_SENSOR_RIGHT_GPIO   GPIO_NUM_48  // ADC1_CH7
```

**注意事项**：
- ⚠️ 如果需要WiFi功能，必须将灰度传感器改用ADC1通道
- ⚠️ ADC1通道：GPIO32-39, GPIO47-48
- ⚠️ 修改引脚需要同时更新硬件接线和软件配置

#### 2. GPIO6-11不可用（SPI Flash占用）

**问题描述**：
GPIO6-11连接到外部SPI Flash，被系统占用，不能用于应用程序。

**影响范围**：
- GPIO6, GPIO7, GPIO8, GPIO9, GPIO10, GPIO11

**后果**：
- 使用这些GPIO会导致系统无法启动
- Flash读写失败，程序崩溃

**解决方案**：
- ✅ 绝对不要使用GPIO6-11
- ✅ 在GPIO管理器中标记为系统保留
- ✅ 硬件设计时避开这些引脚

#### 3. GPIO34-39仅输入（不能输出）

**问题描述**：
GPIO34-39只能配置为输入模式，不能用于输出。

**影响范围**：
- GPIO34, GPIO35, GPIO36, GPIO37, GPIO38, GPIO39

**适用场景**：
- ✅ ADC采样（灰度传感器、温度传感器）
- ✅ 数字输入（按键、触摸传感器）
- ❌ PWM输出
- ❌ 数字输出（LED、继电器）

**当前使用情况**：
- GPIO34: TM1637_CLK（输入模式，开漏输出）
- GPIO35: UART1_TX（特殊情况，可以输出）
- GPIO36: UART1_RX（输入）
- GPIO37: TM1637_DIO（输入模式，开漏输出）
- GPIO38: 红外避障传感器（输入）
- GPIO39: 交通灯信号1（输出，需验证）

**注意事项**：
- ⚠️ TM1637使用开漏输出模式，可以在仅输入GPIO上工作
- ⚠️ GPIO39用于交通灯输出，需要验证是否正常工作
- ⚠️ 如果GPIO39输出异常，改用其他GPIO


### 🟡 重要限制

#### 4. I2C_SDA被灰度传感器占用

**问题描述**：
GPIO20原本用于I2C_SDA（PCF8574扩展芯片），但被灰度传感器临时占用。

**影响范围**：
- PCF8574 I2C扩展芯片不可用
- SSA5/SSD5扩展接口不可用

**解决方案**：
```c
// 将灰度传感器改用GPIO47/48
#define GRAY_SENSOR_LEFT_GPIO    GPIO_NUM_47
#define GRAY_SENSOR_RIGHT_GPIO   GPIO_NUM_48

// 恢复I2C_SDA到GPIO20
#define I2C_MASTER_SDA_GPIO      GPIO_NUM_20
```

**注意事项**：
- ⚠️ 当前配置下PCF8574不可用
- ⚠️ 如需使用PCF8574，必须修改灰度传感器引脚

#### 5. 电源电压限制

**问题描述**：
ESP32 GPIO输入电压不能超过3.3V，否则会损坏芯片。

**影响范围**：
- 所有GPIO输入引脚
- 5V传感器输出

**解决方案**：
```
5V传感器 ──┬── 10kΩ ──┬── ESP32 GPIO
           │          │
          GND       4.7kΩ
                     │
                    GND
```

**注意事项**：
- ⚠️ 使用5V传感器时必须使用电平转换电路或分压电阻
- ⚠️ 电机驱动板不能直接连接ESP32 GPIO
- ⚠️ 舵机信号线可以直接连接（通常3.3V-5V兼容）

---

## 软件限制

### 🔴 关键限制

#### 1. 中断中不能使用阻塞API

**问题描述**：
定时器中断处理函数中不能调用阻塞API，否则会导致系统崩溃。

**禁止操作**：
```c
// ❌ 错误示例
void IRAM_ATTR timer_isr(void *arg) {
    vTaskDelay(pdMS_TO_TICKS(10));  // 禁止！会导致崩溃
    printf("Hello\n");               // 禁止！会导致延迟
    xSemaphoreTake(mutex, portMAX_DELAY);  // 禁止！会死锁
    adc_oneshot_read(adc_handle, channel, &value);  // 禁止！会延迟
}
```

**允许操作**：
```c
// ✅ 正确示例
void IRAM_ATTR timer_isr(void *arg) {
    // 读取原子变量（非阻塞）
    uint16_t left = atomic_load(&cached_left_value);
    
    // 简单计算
    float error = calculate_error(left, right);
    
    // 设置GPIO（非阻塞）
    gpio_set_level(GPIO_NUM_2, 1);
    
    // 调用其他IRAM函数
    pd_controller_tick();
}
```

**注意事项**：
- ⚠️ 中断函数必须使用`IRAM_ATTR`属性
- ⚠️ 中断函数执行时间应尽量短（<2ms）
- ⚠️ 使用原子变量或临界区保护共享数据

#### 2. FreeRTOS任务栈大小限制

**问题描述**：
任务栈大小不足会导致栈溢出，系统崩溃。

**推荐栈大小**：
| 任务类型 | 栈大小 | 说明 |
|---------|--------|------|
| 简单任务 | 2048字节 | 基本逻辑，无复杂计算 |
| 中等任务 | 4096字节 | 有浮点运算、字符串处理 |
| 复杂任务 | 8192字节 | 有递归、大数组、复杂算法 |

**当前任务配置**：
```c
// ADC采样任务（优先级6）
xTaskCreate(adc_sampling_task, "adc_sampling", 2048, NULL, 6, NULL);

// 数码管显示任务（优先级3）
xTaskCreate(display_task, "display", 2048, NULL, 3, NULL);

// 舵机控制任务（优先级3）
xTaskCreate(servo_task, "servo", 2048, NULL, 3, NULL);

// 语音模块任务（优先级4）
xTaskCreate(voice_task, "voice", 4096, NULL, 4, NULL);
```

**注意事项**：
- ⚠️ 栈溢出会导致系统崩溃，难以调试
- ⚠️ 使用`uxTaskGetStackHighWaterMark()`监控栈使用情况
- ⚠️ 如果任务频繁崩溃，尝试增加栈大小

#### 3. UART0 RS485互斥锁

**问题描述**：
UART0被舵机和摄像头共用，需要使用互斥锁保护。

**使用方法**：
```c
// 初始化互斥锁（在使用UART0之前）
uart0_rs485_mutex_init();

// 发送数据前获取锁
if (uart0_rs485_mutex_lock(100) == 0) {
    // 设置RS485为发送模式
    gpio_set_level(RS485_DIR_GPIO, 1);
    
    // 发送数据
    uart_write_bytes(UART0_PORT, data, len);
    
    // 等待发送完成
    uart_wait_tx_done(UART0_PORT, 100);
    
    // 设置RS485为接收模式
    gpio_set_level(RS485_DIR_GPIO, 0);
    
    // 释放锁
    uart0_rs485_mutex_unlock();
}
```

**注意事项**：
- ⚠️ 必须在使用UART0之前初始化互斥锁
- ⚠️ 获取锁后必须释放，否则会死锁
- ⚠️ 不要在中断中使用互斥锁

### 🟡 重要限制

#### 4. 浮点运算性能

**问题描述**：
ESP32-S3有硬件浮点单元，但浮点运算仍比整数运算慢。

**性能对比**：
- 整数加法：1个时钟周期
- 浮点加法：约5个时钟周期
- 浮点除法：约20个时钟周期

**优化建议**：
```c
// ❌ 低效
float result = (float)value / 4095.0f;

// ✅ 高效（如果可以用整数）
int result = (value * 1000) / 4095;

// ✅ 高效（预计算常量）
#define INV_4095 (1.0f / 4095.0f)
float result = (float)value * INV_4095;
```

**注意事项**：
- ⚠️ PD控制器使用浮点运算，已优化
- ⚠️ 中断中尽量避免浮点除法
- ⚠️ 预计算常量可以提高性能

---

## 已知问题和解决方案

### 问题1：灰度传感器ADC值跳变

**现象**：
ADC读取值不稳定，跳变剧烈。

**原因**：
- 信号线受PWM干扰
- 传感器电源不稳定
- 缺少滤波电容

**解决方案**：
1. 在传感器输出端添加0.1uF滤波电容
2. 使用屏蔽线连接传感器
3. 远离PWM信号线和电机
4. 在软件中添加中值滤波或均值滤波

**代码示例**：
```c
// 中值滤波
uint16_t median_filter(uint16_t *buffer, int size) {
    // 排序
    for (int i = 0; i < size - 1; i++) {
        for (int j = i + 1; j < size; j++) {
            if (buffer[i] > buffer[j]) {
                uint16_t temp = buffer[i];
                buffer[i] = buffer[j];
                buffer[j] = temp;
            }
        }
    }
    // 返回中值
    return buffer[size / 2];
}
```

### 问题2：电机启动后系统重启

**现象**：
电机启动瞬间，ESP32重启。

**原因**：
- 电机启动电流过大，导致电源电压跌落
- ESP32掉电重启

**解决方案**：
1. 使用独立电源供电电机驱动板
2. 在ESP32电源输入端添加大电容（100uF-1000uF）
3. 使用稳压模块（如LM2596）为ESP32供电
4. 降低电机启动速度（PWM从0逐步增加）

**代码示例**：
```c
// 缓启动
void motor_soft_start(int target_speed) {
    for (int speed = 0; speed <= target_speed; speed += 50) {
        set_motor_speed(speed);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
```

### 问题3：I2C通信偶尔失败

**现象**：
I2C设备偶尔无响应，通信失败。

**原因**：
- 缺少上拉电阻
- 上拉电阻值不合适
- 总线电容过大（线太长）

**解决方案**：
1. 确认SCL和SDA有4.7kΩ上拉电阻
2. 缩短I2C总线长度（<30cm）
3. 降低I2C时钟频率（从400kHz降到100kHz）
4. 添加重试机制

**代码示例**：
```c
// I2C读取重试
esp_err_t i2c_read_with_retry(uint8_t addr, uint8_t *data, int max_retry) {
    for (int i = 0; i < max_retry; i++) {
        esp_err_t ret = i2c_master_read(addr, data, 1);
        if (ret == ESP_OK) {
            return ESP_OK;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return ESP_FAIL;
}
```

### 问题4：定时器中断频率不准确

**现象**：
监控中断计数发现频率偏差较大（>10%）。

**原因**：
- 中断处理函数执行时间过长
- 系统负载过高
- 其他高优先级中断抢占

**解决方案**：
1. 优化中断处理函数，减少执行时间
2. 降低系统负载，减少任务数量
3. 调整中断优先级
4. 使用示波器测量实际中断频率

**监控代码**：
```c
// 监控中断频率
uint32_t last_count = 0;
while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    uint32_t current_count = timer_system_get_timer0_count();
    uint32_t delta = current_count - last_count;
    ESP_LOGI(TAG, "Timer 0频率: %lu Hz (期望333Hz)", delta);
    last_count = current_count;
}
```

---

## 性能优化建议

### 🟢 一般建议

#### 1. 使用IRAM属性

**说明**：
将频繁调用的函数放在IRAM中，避免cache miss。

**使用方法**：
```c
// 中断处理函数必须使用IRAM_ATTR
void IRAM_ATTR timer_isr(void *arg) {
    // 中断处理代码
}

// 频繁调用的函数建议使用IRAM_ATTR
static inline IRAM_ATTR float calculate_error(uint16_t left, uint16_t right) {
    // 计算代码
}
```

**注意事项**：
- ⚠️ IRAM空间有限（约200KB），不要滥用
- ⚠️ 只对性能关键函数使用IRAM_ATTR
- ⚠️ 中断处理函数必须使用IRAM_ATTR

#### 2. 使用内联函数

**说明**：
将小函数声明为内联，减少函数调用开销。

**使用方法**：
```c
// 内联函数
static inline float normalize_adc(uint16_t raw, uint16_t black, uint16_t white) {
    return (float)(raw - black) / (float)(white - black);
}
```

**注意事项**：
- ⚠️ 只对小函数（<10行）使用inline
- ⚠️ 编译器可能忽略inline建议

#### 3. 预计算常量

**说明**：
将运行时不变的计算结果预先计算，避免重复计算。

**示例**：
```c
// ❌ 低效
float result = value / 4095.0f;  // 每次都计算除法

// ✅ 高效
#define INV_4095 (1.0f / 4095.0f)  // 编译时计算
float result = value * INV_4095;   // 运行时只需乘法
```

#### 4. 使用原子变量

**说明**：
使用原子变量代替互斥锁，提高并发性能。

**使用方法**：
```c
// 定义原子变量
_Atomic uint16_t cached_left_value = 0;

// 写入（任务中）
atomic_store(&cached_left_value, new_value);

// 读取（中断中）
uint16_t value = atomic_load(&cached_left_value);
```

**注意事项**：
- ⚠️ 原子变量只适用于简单数据类型
- ⚠️ 复杂数据结构仍需使用互斥锁

---

## 调试技巧和常用工具

### 调试技巧

#### 1. 使用ESP_LOG分级日志

```c
// 设置日志级别
esp_log_level_set("*", ESP_LOG_WARN);  // 全局WARNING级别
esp_log_level_set("MY_TAG", ESP_LOG_DEBUG);  // 特定模块DEBUG级别

// 使用不同级别日志
ESP_LOGE(TAG, "错误信息");    // 错误
ESP_LOGW(TAG, "警告信息");    // 警告
ESP_LOGI(TAG, "信息");        // 信息
ESP_LOGD(TAG, "调试信息");    // 调试
ESP_LOGV(TAG, "详细信息");    // 详细
```

#### 2. 使用GPIO管理器调试

```c
// 打印GPIO分配表
gpio_manager_print_allocation_table();

// 检查GPIO冲突
esp_err_t ret = gpio_manager_register(GPIO_NUM_18, GPIO_FUNC_ADC, "test", "测试");
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "GPIO18已被占用");
}
```

#### 3. 监控任务栈使用

```c
// 获取任务栈剩余空间
UBaseType_t stack_left = uxTaskGetStackHighWaterMark(NULL);
ESP_LOGI(TAG, "任务栈剩余: %u字节", stack_left * sizeof(StackType_t));
```

#### 4. 监控堆内存使用

```c
// 获取堆内存信息
ESP_LOGI(TAG, "空闲堆: %u字节", esp_get_free_heap_size());
ESP_LOGI(TAG, "最小空闲堆: %u字节", esp_get_minimum_free_heap_size());
```

### 常用工具

#### 1. 串口监视器

```bash
# ESP-IDF监视器（推荐）
idf.py monitor

# 或使用其他串口工具
# - PuTTY
# - Tera Term
# - minicom
```

#### 2. 逻辑分析仪

推荐使用逻辑分析仪调试：
- UART通信
- I2C通信
- 编码器信号
- PWM信号

#### 3. 示波器

推荐使用示波器测量：
- PWM波形
- ADC输入信号
- 电源电压波动

#### 4. 万用表

基本测量：
- 电源电压
- GPIO电平
- 传感器输出电压

---

## 代码规范和最佳实践

### 命名规范

#### 1. 文件命名

```
组件名.c          // 源文件
组件名.h          // 头文件
组件名_test.c     // 测试文件
```

#### 2. 函数命名

```c
// 模块名_功能_动作
void gray_sensor_init(void);
bool gray_sensor_read_raw(uint16_t *value);
float pd_controller_calculate_output(float error);
```

#### 3. 变量命名

```c
// 小写字母+下划线
uint16_t left_value;
float error_deadzone;
_Atomic uint8_t flag_color;
```

#### 4. 宏定义命名

```c
// 大写字母+下划线
#define DEFAULT_SPEED 700
#define TIMER_0_INTERVAL_MS 3
#define GPIO_NUM_18 18
```

### 注释规范

#### 1. 文件头注释

```c
/**
 * @file gray_sensor.c
 * @brief 灰度传感器驱动
 * 
 * 提供灰度传感器的初始化、读取和校准功能
 * 
 * @author 项目团队
 * @date 2024-12-20
 */
```

#### 2. 函数注释

```c
/**
 * @brief 读取灰度传感器原始值
 * 
 * @param left_value 左传感器ADC值输出
 * @param right_value 右传感器ADC值输出
 * @return true 成功，false 失败
 */
bool gray_sensor_read_both_raw(uint16_t *left_value, uint16_t *right_value);
```

#### 3. 行内注释

```c
// 归一化ADC值到0.0-1.0范围
float norm = (float)(raw - black) / (float)(white - black);

/* 多行注释：
 * 复杂算法的详细说明
 * 分步骤解释
 */
```

### 错误处理

#### 1. 返回值检查

```c
// ✅ 正确
esp_err_t ret = gpio_manager_register(GPIO_NUM_18, GPIO_FUNC_ADC, "sensor", "传感器");
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "GPIO注册失败");
    return ESP_FAIL;
}

// ❌ 错误
gpio_manager_register(GPIO_NUM_18, GPIO_FUNC_ADC, "sensor", "传感器");  // 未检查返回值
```

#### 2. 参数验证

```c
// ✅ 正确
bool gray_sensor_read_raw(uint16_t *value) {
    if (value == NULL) {
        ESP_LOGE(TAG, "参数为空");
        return false;
    }
    // 继续处理
}
```

#### 3. 资源清理

```c
// ✅ 正确
esp_err_t init_component(void) {
    void *buffer = malloc(1024);
    if (buffer == NULL) {
        return ESP_FAIL;
    }
    
    esp_err_t ret = do_something(buffer);
    if (ret != ESP_OK) {
        free(buffer);  // 清理资源
        return ESP_FAIL;
    }
    
    free(buffer);
    return ESP_OK;
}
```

---

## 迁移过程中的关键决策

### 决策1：使用硬件定时器代替软件定时器

**原因**：
- 硬件定时器精度更高（1us）
- 不受系统负载影响
- 中断延迟更小

**影响**：
- 需要使用ESP-IDF的GP Timer API
- 中断处理函数必须快速执行
- 不能在中断中使用阻塞API

### 决策2：使用ADC2通道（临时方案）

**原因**：
- 硬件接线已完成，使用GPIO18/20
- 当前项目不需要WiFi功能

**影响**：
- 无法使用WiFi功能
- 如需WiFi，必须修改硬件接线

**未来改进**：
- 将灰度传感器改用GPIO47/48（ADC1通道）
- 恢复I2C_SDA到GPIO20，启用PCF8574

### 决策3：使用原子变量代替互斥锁

**原因**：
- 原子变量性能更高
- 可以在中断中安全使用
- 避免死锁问题

**影响**：
- 只适用于简单数据类型
- 复杂数据结构仍需互斥锁

### 决策4：PD控制器在中断中执行

**原因**：
- 需要高频实时控制（10ms周期）
- FreeRTOS任务调度延迟较大

**影响**：
- 中断处理函数必须快速执行
- 不能在中断中使用阻塞API
- 需要使用IRAM_ATTR属性

---

## 快速检查清单

### 硬件检查

- [ ] 所有模块共地（GND连接）
- [ ] ESP32电源电压3.2V-3.4V
- [ ] 传感器输出电压≤3.3V
- [ ] I2C总线有4.7kΩ上拉电阻
- [ ] 电机驱动板独立供电
- [ ] GPIO6-11未使用
- [ ] TX/RX引脚未接反

### 软件检查

- [ ] GPIO管理器已初始化
- [ ] 没有GPIO冲突
- [ ] ADC通道配置正确（ADC1 vs ADC2）
- [ ] UART波特率匹配
- [ ] I2C地址正确
- [ ] 中断函数使用IRAM_ATTR
- [ ] 中断中未使用阻塞API
- [ ] 任务栈大小足够

### 功能检查

- [ ] ADC读取值在合理范围（0-4095）
- [ ] I2C设备响应正常
- [ ] UART能收发数据
- [ ] PWM输出波形正确
- [ ] 定时器中断频率正确
- [ ] 电机能正常转动
- [ ] 传感器能检测黑线

---

## 参考资料

### 相关文档

- [项目根目录README](../README.md)
- [GPIO引脚分配文档](GPIO_PIN_ALLOCATION.md)
- [配置参数指南](CONFIGURATION_GUIDE.md)
- [STM32到ESP32迁移规格](../.kiro/specs/stm32-to-esp32-migration/)

### 技术文档

- [ESP32-S3技术参考手册](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)
- [ESP-IDF编程指南](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)
- [FreeRTOS文档](https://www.freertos.org/Documentation/RTOS_book.html)

---

**项目**: ESP32巡线小车  
**文档类型**: 项目注意事项  
**维护者**: 项目团队
