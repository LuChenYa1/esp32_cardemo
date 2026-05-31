/**
 * @file main_integrated.c
 * @brief STM32到ESP32移植项目 - 完整集成主程序
 *
 * 功能：
 * - 使用硬件定时器实现实时任务调度
 * - Timer 0 (1ms): 灰度传感器扫描、转弯检测
 * - Timer 1 (10ms): PD控制、转弯状态机
 * - FreeRTOS任务: ADC采样、数码管显示、舵机控制
 *
 * 架构：
 * - 高频实时任务使用硬件定时器中断
 * - 低频外设任务使用FreeRTOS任务
 * - 所有共享变量使用原子操作保护
 */

#include <stdio.h>
#include <stdatomic.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
// 定时器系统
#include "timer_system.h"
// 板级配置和GPIO管理
#include "board_config.h"
#include "gpio_manager.h"
// 灰度传感器和ADC采样
#include "gray_sensor.h"
// 转弯检测
#include "turn_detector.h"
// PD控制器
#include "pd_controller.h"
// 转弯状态机
#include "turn_statemachine.h"
// 数码管显示任务
#include "display_task.h"
// 舵机控制任务
#include "servo_task.h"
// 语音模块
#include "voice_module.h"
// 摄像头协议
#include "camera_protocol.h"
// 红外避障传感器
#include "ir_obstacle.h"
// 引脚定义
#include "pin_definitions.h"
#include "traffic_light.h"  // 添加交通灯组件头文件

// 外部声明语音模块的原子标志变量
extern _Atomic uint8_t Flag_Color;
extern _Atomic uint8_t Flag_Block;
extern _Atomic uint8_t Flag_Face;
extern _Atomic uint8_t Flag_QRCODE;
extern _Atomic uint8_t Flag_NUMBER;
extern _Atomic uint8_t Flag_LABEL;
extern _Atomic uint8_t Flag_20CLASS;

// 硬件外设
#include "pwm.h"
#include "tm1637.h"

static const char *TAG = "MAIN_INTEGRATED";
/**
 * @brief 交通灯控制任务
 * 
 * 每秒切换一次GPIO39和GPIO40的状态
 * 
 * @param pvParameters 任务参数（未使用）
 */
static void traffic_light_task(void *pvParameters)
{
    ESP_LOGI(TAG, "交通灯控制任务启动");
    
    // 初始化交通灯组件
    esp_err_t ret = traffic_light_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "交通灯初始化失败");
        vTaskDelete(NULL);  // 删除当前任务
        return;
    }
    
    traffic_light_state_t current_light = TRAFFIC_LIGHT_RED;
    
    while (1) {
        // 设置当前交通灯状态
        ret = traffic_light_set_state(current_light);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "设置交通灯状态失败");
        } else {
            ESP_LOGD(TAG, "交通灯状态设置为: %s", 
                current_light == TRAFFIC_LIGHT_RED ? "红灯" :
                current_light == TRAFFIC_LIGHT_YELLOW ? "黄灯" : "绿灯");
        }
        
        // 根据当前状态切换到下一个状态
        switch (current_light) {
            case TRAFFIC_LIGHT_RED:
                current_light = TRAFFIC_LIGHT_GREEN;
                break;
            case TRAFFIC_LIGHT_GREEN:
                current_light = TRAFFIC_LIGHT_YELLOW;
                break;
            case TRAFFIC_LIGHT_YELLOW:
                current_light = TRAFFIC_LIGHT_RED;
                break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(3000));  // 每3秒切换一次
    }
}

/**
 * @brief 初始化所有硬件外设
 *
 * @return ESP_OK 成功，ESP_FAIL 失败
 */
static esp_err_t init_hardware_peripherals(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "开始初始化硬件外设...");

    // 0. 初始化UART0 RS485互斥锁（必须在使用RS485之前初始化）
    ESP_LOGI(TAG, "初始化UART0 RS485互斥锁...");
    if (uart0_rs485_mutex_init() != 0)
    {
        ESP_LOGE(TAG, "UART0 RS485互斥锁初始化失败");
        return ESP_FAIL;
    }

    // 1. 初始化PWM（电机控制）
    ESP_LOGI(TAG, "初始化PWM...");
    ledc_init();

    // 2. 初始化TM1637数码管
    ESP_LOGI(TAG, "初始化TM1637数码管...");
    tm1637_init();

    // 3. 初始化红外避障传感器
    ESP_LOGI(TAG, "初始化红外避障传感器...");
    ir_obstacle_init();

    // 4. 初始化语音模块（UART1）
    ESP_LOGI(TAG, "初始化语音模块...");
    ret = voice_module_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "语音模块初始化失败");
        return ret;
    }

    // 5. 初始化摄像头协议（UART0，与舵机共用）
    ESP_LOGI(TAG, "初始化摄像头协议...");
    // 注意：摄像头使用UART0，与舵机共用，已在servo485_init()中初始化
    // 这里不需要额外初始化，只需确保RS485方向控制引脚已配置

    // 6. 初始化灰度传感器（ADC2通道）
    ESP_LOGI(TAG, "初始化灰度传感器...");
    gray_sensor_init_simple();

    // 等待硬件稳定
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "硬件外设初始化完成");
    return ESP_OK;
}

/**
 * @brief 初始化所有软件模块
 *
 * @return ESP_OK 成功，ESP_FAIL 失败
 */
static esp_err_t init_software_modules(void)
{
    ESP_LOGI(TAG, "开始初始化软件模块...");

    // 1. 初始化转弯检测模块
    ESP_LOGI(TAG, "初始化转弯检测模块...");
    turn_detector_init();

    // 2. 初始化PD控制器
    ESP_LOGI(TAG, "初始化PD控制器...");
    pd_controller_init();


    // 3. 初始化转弯状态机
    ESP_LOGI(TAG, "初始化转弯状态机...");
    turn_statemachine_init();

    ESP_LOGI(TAG, "软件模块初始化完成");
    return ESP_OK;
}

/**
 * @brief 创建所有FreeRTOS任务
 *
 * @return ESP_OK 成功，ESP_FAIL 失败
 */
static esp_err_t create_freertos_tasks(void)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "开始创建FreeRTOS任务...");

    // 1. 创建ADC采样任务（优先级5，最高）
    ESP_LOGI(TAG, "创建ADC采样任务...");
    gray_scanner_init();

    // 2. 创建数码管显示任务（优先级3）
    ESP_LOGI(TAG, "创建数码管显示任务...");
    ret = display_task_create();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "创建数码管显示任务失败");
        return ret;
    }

    // 3. 创建舵机控制任务（优先级3）
    ESP_LOGI(TAG, "创建舵机控制任务...");
    TaskHandle_t servo_handle = servo_task_create();
    if (servo_handle == NULL)
    {
        ESP_LOGE(TAG, "创建舵机控制任务失败");
        return ESP_FAIL;
    }

    // 4. 创建语音模块任务（优先级4）
    ESP_LOGI(TAG, "创建语音模块任务...");
    ret = voice_module_task_create();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "创建语音模块任务失败");
        return ret;
    }

    // 4. 创建交通灯控制任务（优先级2，低优先级后台任务）
    ESP_LOGI(TAG, "创建交通灯控制任务...");
    BaseType_t task_ret = xTaskCreatePinnedToCore(
        traffic_light_task,
        "traffic_light",
        2048,
        NULL,
        2,  // 优先级2，低于显示任务
        NULL,
        1   // 固定到Core 1
    );
    if (task_ret != pdPASS)
    {
        ESP_LOGE(TAG, "创建交通灯控制任务失败");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "FreeRTOS任务创建完成");
    return ESP_OK;
}

/**
 * @brief 系统自检
 *
 * 检查关键硬件和软件模块是否正常工作
 *
 * @return ESP_OK 成功，ESP_FAIL 失败
 */
static esp_err_t system_self_check(void)
{
    ESP_LOGI(TAG, "开始系统自检...");

    // // 1. 检查ADC读取
    // uint16_t left_raw, right_raw;
    // if (!gray_sensor_read_both_raw_direct(&left_raw, &right_raw))
    // {
    //     ESP_LOGE(TAG, "自检失败：ADC读取失败");
    //     return ESP_FAIL;
    // }
    // ESP_LOGI(TAG, "ADC自检通过：左=%d, 右=%d", left_raw, right_raw);

    // // 2. 检查ADC值在合理范围内（0-4095）
    // if (left_raw > 4095 || right_raw > 4095)
    // {
    //     ESP_LOGE(TAG, "自检失败：ADC值超出范围");
    //     return ESP_FAIL;
    // }

    // 3. 检查电机PWM（设置为0，确保初始停止）
    set_motor1_speed(0, 0);
    set_motor2_speed(0, 0);
    set_motor3_speed(0, 0);
    set_motor4_speed(0, 0);
    ESP_LOGI(TAG, "电机PWM自检通过");

    ESP_LOGI(TAG, "系统自检完成");
    return ESP_OK;
}

/**
 * @brief 主函数
 */
void app_main(void)
{
    esp_err_t ret;
    esp_log_level_set("gpio", ESP_LOG_WARN);
    esp_log_level_set("*", ESP_LOG_NONE);
    // 全局设置：只输出 WARNING(警告) 和 ERROR(错误) 级别日志
    // esp_log_level_set("*", ESP_LOG_WARN);
    uint32_t last_timer0_count = 0;
    uint32_t last_timer1_count = 0;
    uint32_t last_adc_error_count = 0;
    uint8_t last_run_mode = 0xFF; // 上次运行模式（用于检测变化）
    uint16_t left_gray_value = 0, right_gray_value = 0; // 用于存储灰度传感器值
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "STM32到ESP32移植项目 - 完整集成版本");
    ESP_LOGI(TAG, "========================================");

    // ==================== 阶段0：初始化GPIO管理器 ====================
    ESP_LOGI(TAG, "初始化GPIO管理器...");
    ret = board_config_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "GPIO管理器初始化失败，系统停止");
        return;
    }

    // ==================== 阶段1：初始化硬件外设 ====================
    ret = init_hardware_peripherals();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "硬件外设初始化失败，系统停止");
        return;
    }

    // 打印GPIO分配表（用于调试）
    ESP_LOGI(TAG, "打印GPIO分配表：");
    gpio_manager_print_allocation_table();

    // ==================== 阶段2：初始化软件模块 ====================
    ret = init_software_modules();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "软件模块初始化失败，系统停止");
        return;
    }

    // ==================== 阶段3：系统自检 ====================
    ret = system_self_check();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "系统自检失败，系统停止");
        return;
    }

    // // ==================== 阶段4：配置看门狗 ====================
    // ESP_LOGI(TAG, "配置看门狗定时器...");
    // ret = watchdog_init();
    // if (ret != ESP_OK)
    // {
    //     ESP_LOGE(TAG, "看门狗初始化失败");
    //     // 看门狗失败不是致命错误，继续运行
    // }

    // ==================== 阶段5：初始化定时器系统 ====================
    ESP_LOGI(TAG, "初始化定时器系统...");
    ret = timer_system_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "定时器系统初始化失败，系统停止");
        return;
    }

    // ==================== 阶段6：创建FreeRTOS任务 ====================
    ret = create_freertos_tasks();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "FreeRTOS任务创建失败，系统停止");
        return;
    }

    // 等待所有任务启动
    vTaskDelay(pdMS_TO_TICKS(500));

    // ==================== 阶段7：启动定时器系统 ====================
    ESP_LOGI(TAG, "启动定时器系统...");
    ret = timer_system_start();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "定时器系统启动失败，系统停止");
        return;
    }

    // ==================== 系统启动完成 ====================
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "系统启动完成！");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Timer 0 (3ms): 灰度扫描 + 转弯检测");
    ESP_LOGI(TAG, "Timer 1 (10ms): PD控制 + 转弯状态机");
    ESP_LOGI(TAG, "ADC采样任务: 1ms周期");
    ESP_LOGI(TAG, "数码管显示任务: 5秒轮播（温湿度/超声波测距/红外避障状态）");
    ESP_LOGI(TAG, "交通灯控制任务: 每秒切换GPIO39/40状态");
    ESP_LOGI(TAG, "语音模块任务: UART1接收语音命令");
    ESP_LOGI(TAG, "摄像头协议: UART0（与舵机共用RS485）");
    ESP_LOGI(TAG, "========================================");

    // ==================== 主循环：语音控制 + 系统监控 ====================

    ESP_LOGI(TAG, "进入主循环（语音控制电机启停 + 系统监控）");

    while (1)
    {
        // 每200ms执行一次主循环任务
        vTaskDelay(pdMS_TO_TICKS(200));

        // ========== 1. 检查运行模式变化（语音控制电机启停） ==========
        uint8_t current_run_mode = voice_module_get_run_mode();
        if (current_run_mode != last_run_mode)
        {
            last_run_mode = current_run_mode;

            if (current_run_mode == 1)
            {
                // 巡线模式：启动电机（PD控制器会自动控制电机）
                ESP_LOGI(TAG, "切换到巡线模式 - 电机启动");
                pd_controller_enable();
            }
            else
            {
                // 命令模式：停止电机
                ESP_LOGI(TAG, "切换到命令模式 - 电机停止");
                pd_controller_disable();
                set_motor1_speed(0, 0);
                set_motor2_speed(0, 0);
                set_motor3_speed(0, 0);
                set_motor4_speed(0, 0);
            }
        }

        // ========== 2. 摄像头控制已移至语音模块（收到命令时执行一次） ==========
        // 不再需要在主循环中持续读取摄像头

        // ========== 3. 系统状态监控（每5秒打印一次） ==========
        static uint32_t monitor_count = 0;
        // monitor_count++;
        // if (monitor_count >= 25) // 200ms * 25 = 5000ms = 5秒
        // {
        //     monitor_count = 0;

        //     // 获取定时器中断计数
        //     uint32_t timer0_count = timer_system_get_timer0_count();
        //     uint32_t timer1_count = timer_system_get_timer1_count();
        //     uint32_t adc_error_count = gray_scanner_get_error_count();

        //     // 获取最新的灰度传感器值
        //     gray_scanner_get_cached_values(&left_gray_value, &right_gray_value);

        //     // 计算5秒内的中断次数
        //     uint32_t timer0_delta = timer0_count - last_timer0_count;
        //     uint32_t timer1_delta = timer1_count - last_timer1_count;
        //     uint32_t adc_error_delta = adc_error_count - last_adc_error_count;

        //     ESP_LOGI(TAG, "系统状态 - Timer0: %lu次/5s, Timer1: %lu次/5s, ADC错误: %lu次",
        //              timer0_delta, timer1_delta, adc_error_delta);

        //     // 打印灰度传感器值
        //     ESP_LOGI(TAG, "灰度传感器 - 左: %u, 右: %u", left_gray_value, right_gray_value);

        //     // 检查定时器是否正常工作
        //     // Timer 0应该每5秒触发约1667次（3ms周期，3000us，5000ms/3ms≈1667）
        //     // Timer 1应该每5秒触发约500次（10ms周期，10000us）
        //     if (timer0_delta < 1500 || timer0_delta > 1800)
        //     {
        //         ESP_LOGW(TAG, "警告：Timer 0中断频率异常（期望1667次/5s，实际%lu次/5s）", timer0_delta);
        //     }
        //     if (timer1_delta < 450 || timer1_delta > 550)
        //     {
        //         ESP_LOGW(TAG, "警告：Timer 1中断频率异常（期望500次/5s，实际%lu次/5s）", timer1_delta);
        //     }

        //     // 检查ADC错误率
        //     if (adc_error_delta > 50)
        //     {
        //         ESP_LOGW(TAG, "警告：ADC错误率较高（%lu次/5s）", adc_error_delta);
        //     }

        //     // 更新上次计数
        //     last_timer0_count = timer0_count;
        //     last_timer1_count = timer1_count;
        //     last_adc_error_count = adc_error_count;

        //     // 打印转弯状态（调试用）
        //     TurnState_t turn_state = turn_statemachine_get_state();
        //     uint8_t turn_type = turn_detector_get_type();
        //     bool is_turning = turn_detector_is_turning();

        //     ESP_LOGI(TAG, "转弯状态 - 状态机: %d, 转弯类型: %d, 转弯中: %d",
        //              turn_state, turn_type, is_turning);

        //     // 打印运行模式
        //     ESP_LOGI(TAG, "运行模式: %s", current_run_mode ? "巡线模式" : "命令模式");
        // }
        // else
        // {
        //     // 每200ms都读取一次灰度传感器值并打印（但不记录到统计数据中）
        //     gray_scanner_get_cached_values(&left_gray_value, &right_gray_value);
        //     ESP_LOGI(TAG, "灰度传感器 - 左: %u, 右: %u", left_gray_value, right_gray_value);
        // }
    }
}
