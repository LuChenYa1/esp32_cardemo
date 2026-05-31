# Timer System 快速参考

## 📋 初始化检查清单

```c
void app_main(void)
{
    // ✅ 1. 初始化外设
    gray_sensor_init_simple();
    ledc_init();
    
    // ✅ 2. 创建ADC采样任务
    gray_scanner_init();
    
    // ✅ 3. 初始化控制模块
    turn_detector_init();
    pd_controller_init();
    turn_statemachine_init();
    
    // ✅ 4. 初始化定时器（检查返回值！）
    if (timer_system_init() != ESP_OK) {
        ESP_LOGE(TAG, "定时器初始化失败");
        return;  // 停止启动
    }
    
    // ✅ 5. 配置看门狗（在启动定时器之前）
    if (watchdog_init() != ESP_OK) {
        ESP_LOGW(TAG, "看门狗初始化失败");
    }
    
    // ✅ 6. 启动定时器
    timer_system_start();
    
    // ✅ 7. 主循环
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
```

## 🔧 核心API

| 函数 | 功能 | 返回值 |
|------|------|--------|
| `timer_system_init()` | 初始化定时器系统 | `ESP_OK` / `ESP_FAIL` |
| `timer_system_start()` | 启动定时器 | `ESP_OK` / `ESP_FAIL` |
| `timer_system_stop()` | 停止定时器 | `ESP_OK` / `ESP_FAIL` |
| `watchdog_init()` | 初始化看门狗 | `ESP_OK` / `ESP_FAIL` |
| `enter_safe_mode()` | 进入安全模式 | 不返回 |

## 📊 系统参数

| 参数 | 值 | 说明 |
|------|-----|------|
| Timer 0周期 | 1ms | 灰度扫描、转弯检测 |
| Timer 1周期 | 10ms | PD控制、状态机 |
| Timer 0优先级 | 1 | 最高优先级 |
| Timer 1优先级 | 2 | 次高优先级 |
| 看门狗超时 | 5秒 | 监控IDLE任务 |
| ADC错误阈值 | 100次 | 触发安全模式 |

## ⚠️ 常见错误

### 错误1：定时器初始化失败

**症状**：`timer_system_init()` 返回 `ESP_FAIL`

**可能原因**：
- 定时器资源已被占用
- 内存不足
- 硬件故障

**解决方法**：
1. 检查是否有其他代码使用了Timer Group 0
2. 增加堆内存大小
3. 查看详细错误日志

### 错误2：看门狗初始化失败

**症状**：`watchdog_init()` 返回 `ESP_FAIL`

**可能原因**：
- 看门狗已被初始化
- 配置参数错误

**解决方法**：
1. 确保只调用一次 `watchdog_init()`
2. 检查 `esp_task_wdt_config_t` 配置
3. 系统可以继续运行（但没有看门狗保护）

### 错误3：系统进入安全模式

**症状**：所有电机停止，日志显示"系统进入安全模式"

**可能原因**：
- ADC连续失败超过100次
- 传感器连接问题
- 硬件故障

**解决方法**：
1. 检查灰度传感器连接（GPIO19/20）
2. 检查ADC2是否与WiFi冲突
3. 查看ADC错误计数：`gray_scanner_get_error_count()`
4. 手动重启或等待看门狗重启

## 🐛 调试技巧

### 监控定时器运行状态

```c
// 每秒打印一次中断计数
uint32_t last_count0 = 0;
uint32_t last_count1 = 0;

while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    uint32_t count0 = timer_system_get_timer0_count();
    uint32_t count1 = timer_system_get_timer1_count();
    
    ESP_LOGI(TAG, "Timer0: %lu/s, Timer1: %lu/s",
             count0 - last_count0,
             count1 - last_count1);
    
    last_count0 = count0;
    last_count1 = count1;
}

// 预期输出：
// Timer0: 1000/s (±10%)
// Timer1: 100/s (±10%)
```

### 监控ADC健康状态

```c
uint32_t error_count = gray_scanner_get_error_count();

if (error_count > 50) {
    ESP_LOGW(TAG, "警告：ADC错误计数较高 (%lu)", error_count);
}

if (error_count > 90) {
    ESP_LOGE(TAG, "严重：ADC即将触发安全模式！");
}
```

### 打印传感器状态

```c
// 打印原始ADC值和归一化值
gray_sensor_print_status();

// 输出示例：
// 左传感器: 原始=2500, 归一化=0.650 [白色]
// 右传感器: 原始=1200, 归一化=0.100 [黑线]
```

## 📚 相关文档

- **详细文档**：`README.md`
- **实现总结**：`TASK13_IMPLEMENTATION_SUMMARY.md`
- **示例程序**：`main/main_timer_watchdog_example.c`
- **设计文档**：`.kiro/specs/stm32-to-esp32-migration/design.md`

## 🆘 获取帮助

如果遇到问题：
1. 查看详细错误日志
2. 检查初始化顺序
3. 验证硬件连接
4. 参考示例程序
5. 查阅ESP-IDF官方文档
