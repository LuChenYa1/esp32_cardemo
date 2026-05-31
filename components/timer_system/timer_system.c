/**
 * @file timer_system.c
 * @brief ESP32硬件定时器系统实现
 */

#include "timer_system.h"
#include "turn_detector.h"
#include "pd_controller.h"
#include "turn_statemachine.h"
#include "pwm.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "timer_system";

// ==================== 全局变量 ====================

// 定时器句柄
static gptimer_handle_t timer0_handle = NULL;
static gptimer_handle_t timer1_handle = NULL;

// 中断计数器（用于调试和测试）
static volatile uint32_t g_timer0_count = 0;
static volatile uint32_t g_timer1_count = 0;

// 时间戳记录（用于周期精度测试）
static volatile uint64_t g_timer0_last_timestamp = 0;
static volatile uint64_t g_timer1_last_timestamp = 0;

// ==================== 中断处理函数 ====================

/**
 * @brief Timer 0中断处理函数（3ms周期）
 *
 * 功能：
 * - 灰度传感器扫描（从共享变量读取ADC缓存值）
 * - 转弯检测逻辑
 *
 * 注意：
 * - 使用IRAM_ATTR属性，确保代码在IRAM中执行（避免cache miss）
 * - 禁止调用任何阻塞API（vTaskDelay、printf、mutex等）
 * - 禁止直接调用ADC读取（会导致中断延迟）
 */
static bool IRAM_ATTR timer0_isr_handler(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    // 递增中断计数器
    g_timer0_count++;

    // 记录时间戳（用于周期精度测试）
    g_timer0_last_timestamp = edata->count_value;

    // 执行转弯检测逻辑
    // 注意：turn_detector_tick内部会调用gray_scanner_get_cached_values读取ADC缓存值
    turn_detector_tick();

    return false; // 不需要yield
}

/**
 * @brief Timer 1中断处理函数（10ms周期）
 *
 * 功能：
 * - PD控制算法
 * - 六状态转弯状态机
 *
 * 注意：
 * - 使用IRAM_ATTR属性，确保代码在IRAM中执行
 * - 禁止调用任何阻塞API
 * - 从共享变量读取ADC缓存值和转弯标志
 */
static bool IRAM_ATTR timer1_isr_handler(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    // 递增中断计数器
    g_timer1_count++;

    // 记录时间戳（用于周期精度测试）
    g_timer1_last_timestamp = edata->count_value;

    // 执行PD控制算法
    // 注意：pd_controller_tick内部会：
    // 1. 检查转弯标志，转弯期间暂停控制
    // 2. 读取ADC缓存值
    // 3. 执行PD控制算法
    // 4. 设置电机速度
    pd_controller_tick();

    // 执行转弯状态机
    // 注意：turn_statemachine_tick内部会：
    // 1. 检查转弯请求
    // 2. 执行状态转换
    // 3. 控制电机运动（转弯动作）
    // 4. 在转弯完成后清除转弯标志
    turn_statemachine_tick();

    return false; // 不需要yield
}

// ==================== 公共接口实现 ====================

esp_err_t timer_system_init(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "初始化定时器系统...");

    // ========== 配置Timer 0（1ms周期） ==========

    gptimer_config_t timer0_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz，1us分辨率
    };

    ret = gptimer_new_timer(&timer0_config, &timer0_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Timer 0创建失败: %s", esp_err_to_name(ret));
        return ret;
    }

    // 配置Timer 0报警事件（3ms = 3000us）
    gptimer_alarm_config_t timer0_alarm_config = {
        .alarm_count = 3000, // 3ms - 高频转弯检测
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };

    ret = gptimer_set_alarm_action(timer0_handle, &timer0_alarm_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Timer 0设置报警失败: %s", esp_err_to_name(ret));
        gptimer_del_timer(timer0_handle);
        return ret;
    }

    // 注册Timer 0中断回调
    gptimer_event_callbacks_t timer0_cbs = {
        .on_alarm = timer0_isr_handler,
    };

    ret = gptimer_register_event_callbacks(timer0_handle, &timer0_cbs, NULL);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Timer 0注册回调失败: %s", esp_err_to_name(ret));
        gptimer_del_timer(timer0_handle);
        return ret;
    }

    // 使能Timer 0
    ret = gptimer_enable(timer0_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Timer 0使能失败: %s", esp_err_to_name(ret));
        gptimer_del_timer(timer0_handle);
        return ret;
    }

    ESP_LOGI(TAG, "Timer 0初始化成功（1ms周期，优先级%d）", TIMER_0_PRIORITY);

    // ========== 配置Timer 1（10ms周期） ==========

    gptimer_config_t timer1_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1MHz，1us分辨率
    };

    ret = gptimer_new_timer(&timer1_config, &timer1_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Timer 1创建失败: %s", esp_err_to_name(ret));
        gptimer_disable(timer0_handle);
        gptimer_del_timer(timer0_handle);
        return ret;
    }

    // 配置Timer 1报警事件（10ms = 10000us）
    gptimer_alarm_config_t timer1_alarm_config = {
        .alarm_count = 10000, // 10ms
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };

    ret = gptimer_set_alarm_action(timer1_handle, &timer1_alarm_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Timer 1设置报警失败: %s", esp_err_to_name(ret));
        gptimer_disable(timer0_handle);
        gptimer_del_timer(timer0_handle);
        gptimer_del_timer(timer1_handle);
        return ret;
    }

    // 注册Timer 1中断回调
    gptimer_event_callbacks_t timer1_cbs = {
        .on_alarm = timer1_isr_handler,
    };

    ret = gptimer_register_event_callbacks(timer1_handle, &timer1_cbs, NULL);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Timer 1注册回调失败: %s", esp_err_to_name(ret));
        gptimer_disable(timer0_handle);
        gptimer_del_timer(timer0_handle);
        gptimer_del_timer(timer1_handle);
        return ret;
    }

    // 使能Timer 1
    ret = gptimer_enable(timer1_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Timer 1使能失败: %s", esp_err_to_name(ret));
        gptimer_disable(timer0_handle);
        gptimer_del_timer(timer0_handle);
        gptimer_del_timer(timer1_handle);
        return ret;
    }

    ESP_LOGI(TAG, "Timer 1初始化成功（10ms周期，优先级%d）", TIMER_1_PRIORITY);

    ESP_LOGI(TAG, "定时器系统初始化完成");
    return ESP_OK;
}

esp_err_t timer_system_start(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "启动定时器系统...");

    // 启动Timer 0
    ret = gptimer_start(timer0_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Timer 0启动失败: %s", esp_err_to_name(ret));
        return ret;
    }

    // 启动Timer 1
    ret = gptimer_start(timer1_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Timer 1启动失败: %s", esp_err_to_name(ret));
        gptimer_stop(timer0_handle);
        return ret;
    }

    ESP_LOGI(TAG, "定时器系统已启动");
    return ESP_OK;
}

esp_err_t timer_system_stop(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "停止定时器系统...");

    // 停止Timer 0
    ret = gptimer_stop(timer0_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Timer 0停止失败: %s", esp_err_to_name(ret));
    }

    // 停止Timer 1
    ret = gptimer_stop(timer1_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Timer 1停止失败: %s", esp_err_to_name(ret));
    }

    ESP_LOGI(TAG, "定时器系统已停止");
    return ESP_OK;
}

uint32_t timer_system_get_timer0_count(void)
{
    return g_timer0_count;
}

uint32_t timer_system_get_timer1_count(void)
{
    return g_timer1_count;
}

/**
 * @brief 进入安全模式
 *
 * 当系统检测到严重错误（如ADC连续失败100次）时调用此函数
 * 功能：
 * 1. 停止所有电机（设置速度为0）
 * 2. 记录错误日志
 * 3. 可选：触发蜂鸣器报警（如果已初始化）
 * 4. 进入死循环等待重启
 *
 * 注意：此函数不会返回
 */
void enter_safe_mode(void)
{
    // 停止所有电机
    set_motor1_speed(0, 0);
    set_motor2_speed(0, 0);
    set_motor3_speed(0, 0);
    set_motor4_speed(0, 0);

    // 记录错误日志
    ESP_LOGE(TAG, "========================================");
    ESP_LOGE(TAG, "系统进入安全模式！");
    ESP_LOGE(TAG, "所有电机已停止");
    ESP_LOGE(TAG, "请检查系统错误并重启设备");
    ESP_LOGE(TAG, "========================================");

    // 可选：触发蜂鸣器报警（3次短响）
    // 注意：如果蜂鸣器未初始化，此代码会被跳过
    // buzzer_beep(200);
    // vTaskDelay(pdMS_TO_TICKS(200));
    // buzzer_beep(200);
    // vTaskDelay(pdMS_TO_TICKS(200));
    // buzzer_beep(200);

    // 进入死循环，等待看门狗重启或手动重启
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief 初始化看门狗定时器
 *
 * 配置Task Watchdog Timer (TWDT)：
 * - 超时时间：5秒
 * - 监控核心0的IDLE任务
 * - 超时触发panic重启
 *
 * @return
 *   - ESP_OK: 初始化成功
 *   - 其他: 初始化失败
 */
esp_err_t watchdog_init(void)
{
    ESP_LOGI(TAG, "初始化看门狗定时器...");

    // 配置任务看门狗（Task Watchdog Timer）
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = 5000,         // 5秒超时
        .idle_core_mask = (1 << 0), // 监控核心0的IDLE任务
        .trigger_panic = true,      // 超时触发panic重启
    };

    esp_err_t ret = esp_task_wdt_init(&twdt_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "看门狗初始化失败: %s", esp_err_to_name(ret));
        return ret;
    }

    // 将IDLE任务添加到看门狗
    // 注意：IDLE任务句柄通过xTaskGetIdleTaskHandle()获取
    TaskHandle_t idle_task_handle = xTaskGetIdleTaskHandle();
    if (idle_task_handle != NULL)
    {
        ret = esp_task_wdt_add(idle_task_handle);
        if (ret != ESP_OK)
        {
            ESP_LOGE(TAG, "添加IDLE任务到看门狗失败: %s", esp_err_to_name(ret));
            return ret;
        }
    }
    else
    {
        ESP_LOGW(TAG, "无法获取IDLE任务句柄");
    }

    ESP_LOGI(TAG, "看门狗定时器已启用，超时时间：5秒");
    return ESP_OK;
}
