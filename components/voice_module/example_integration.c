/**
 * @file example_integration.c
 * @brief 语音控制模块集成示例
 * 
 * 展示如何在实际的巡线系统中集成语音控制模块
 * 
 * 注意：这是一个示例文件，不会被编译到最终程序中
 */

#include "voice_module.h"
#include "timer_system.h"
#include "pd_controller.h"
#include "turn_statemachine.h"
#include "gray_sensor.h"
#include "pwm.h"
#include "esp_log.h"

static const char *TAG = "integration_example";

/**
 * @brief 示例1: 在主程序中集成语音控制
 */
void example_1_basic_integration(void)
{
    ESP_LOGI(TAG, "示例1: 基本集成");
    
    // 初始化所有模块
    voice_module_init();
    voice_module_task_create();
    
    // 主循环
    while (1) {
        // 检查运行模式
        uint8_t run_mode = voice_module_get_run_mode();
        
        if (run_mode == 1) {
            ESP_LOGI(TAG, "小车运行中");
            // 正常执行巡线控制
        } else {
            ESP_LOGI(TAG, "小车已停止");
            // 停止所有电机
            set_motor1_speed(0, 0);
            set_motor2_speed(0, 0);
            set_motor3_speed(0, 0);
            set_motor4_speed(0, 0);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief 示例2: 在PD控制器中集成语音控制
 * 
 * 在Timer 1中断中调用，确保停止状态下不执行PD控制
 */
void example_2_pd_controller_integration(void)
{
    // 这个函数在Timer 1中断中调用（10ms周期）
    
    // 检查运行模式
    if (voice_module_get_run_mode() == 0) {
        // 停止状态，不执行PD控制
        // 停止所有电机
        set_motor1_speed(0, 0);
        set_motor2_speed(0, 0);
        set_motor3_speed(0, 0);
        set_motor4_speed(0, 0);
        return;
    }
    
    // 运行状态，正常执行PD控制
    uint16_t left_raw, right_raw;
    gray_scanner_get_cached_values(&left_raw, &right_raw);
    
    // PD控制算法
    // ... (省略具体实现)
}

/**
 * @brief 示例3: 在转弯状态机中集成语音控制
 * 
 * 在Timer 1中断中调用，确保停止状态下重置状态机
 */
void example_3_statemachine_integration(void)
{
    // 这个函数在Timer 1中断中调用（10ms周期）
    
    // 检查运行模式
    if (voice_module_get_run_mode() == 0) {
        // 停止状态，重置状态机到IDLE
        // atomic_store(&g_turn_state, TURN_IDLE);
        // atomic_store(&g_turning_in_progress, false);
        return;
    }
    
    // 运行状态，正常执行状态机
    // ... (省略具体实现)
}

/**
 * @brief 示例4: 完整的app_main集成
 */
void example_4_complete_app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "STM32到ESP32巡线小车系统启动");
    ESP_LOGI(TAG, "========================================");
    
    // 1. 初始化硬件外设
    ESP_LOGI(TAG, "初始化硬件外设...");
    // pwm_init();
    // gray_sensor_init();
    // ... 其他外设初始化
    
    // 2. 初始化语音控制模块（可选）
    ESP_LOGI(TAG, "初始化语音控制模块...");
    esp_err_t err = voice_module_init();
    if (err == ESP_OK) {
        voice_module_task_create();
        ESP_LOGI(TAG, "✓ 语音控制模块已启用");
    } else {
        ESP_LOGW(TAG, "⚠ 语音控制模块未启用（可选功能）");
    }
    
    // 3. 初始化定时器系统
    ESP_LOGI(TAG, "初始化定时器系统...");
    // timer_system_init();
    
    // 4. 创建FreeRTOS任务
    ESP_LOGI(TAG, "创建FreeRTOS任务...");
    // adc_sampling_task_create();
    // display_task_create();
    // servo_task_create();
    
    // 5. 启动定时器
    ESP_LOGI(TAG, "启动定时器...");
    // timer_start(TIMER_GROUP_0, TIMER_0);
    // timer_start(TIMER_GROUP_0, TIMER_1);
    
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "系统启动完成");
    ESP_LOGI(TAG, "========================================");
    
    // 6. 主循环：监控系统状态
    while (1) {
        // 获取运行模式
        uint8_t run_mode = voice_module_get_run_mode();
        
        // 定期打印状态（每5秒）
        static uint32_t last_print_time = 0;
        uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        
        if (current_time - last_print_time >= 5000) {
            ESP_LOGI(TAG, "系统状态: %s", 
                     run_mode ? "运行中" : "已停止");
            last_print_time = current_time;
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief 示例5: 带安全检查的集成
 */
void example_5_safe_integration(void)
{
    ESP_LOGI(TAG, "示例5: 带安全检查的集成");
    
    // 初始化语音模块
    voice_module_init();
    voice_module_task_create();
    
    // 主循环
    while (1) {
        uint8_t run_mode = voice_module_get_run_mode();
        
        if (run_mode == 1) {
            // 运行状态：执行巡线控制
            
            // 安全检查1：检查ADC是否正常
            // if (g_adc_error_count > 100) {
            //     ESP_LOGE(TAG, "ADC错误过多，强制停止");
            //     voice_module_set_run_mode(0);
            //     continue;
            // }
            
            // 安全检查2：检查是否有紧急停止信号
            // if (emergency_stop_triggered()) {
            //     ESP_LOGW(TAG, "紧急停止触发");
            //     voice_module_set_run_mode(0);
            //     continue;
            // }
            
            // 正常执行
            ESP_LOGD(TAG, "巡线控制运行中");
            
        } else {
            // 停止状态：确保所有电机停止
            set_motor1_speed(0, 0);
            set_motor2_speed(0, 0);
            set_motor3_speed(0, 0);
            set_motor4_speed(0, 0);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief 示例6: 手动控制与语音控制结合
 */
void example_6_manual_and_voice_control(void)
{
    ESP_LOGI(TAG, "示例6: 手动控制与语音控制结合");
    
    // 初始化语音模块
    voice_module_init();
    voice_module_task_create();
    
    // 假设有一个按键用于手动控制
    bool manual_button_pressed = false;
    
    while (1) {
        // 检查手动按键
        // manual_button_pressed = read_button();
        
        if (manual_button_pressed) {
            // 手动控制优先级更高
            ESP_LOGI(TAG, "手动停止");
            voice_module_set_run_mode(0);
        }
        
        // 语音控制会自动更新运行模式
        uint8_t run_mode = voice_module_get_run_mode();
        
        if (run_mode == 1) {
            ESP_LOGD(TAG, "运行中");
        } else {
            ESP_LOGD(TAG, "已停止");
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief 使用说明
 * 
 * 这些示例展示了如何在不同场景下集成语音控制模块：
 * 
 * 1. example_1_basic_integration()
 *    - 最简单的集成方式
 *    - 适合快速原型开发
 * 
 * 2. example_2_pd_controller_integration()
 *    - 在PD控制器中集成
 *    - 确保停止状态下不执行控制
 * 
 * 3. example_3_statemachine_integration()
 *    - 在转弯状态机中集成
 *    - 确保停止状态下重置状态机
 * 
 * 4. example_4_complete_app_main()
 *    - 完整的系统集成示例
 *    - 包含所有模块的初始化
 * 
 * 5. example_5_safe_integration()
 *    - 带安全检查的集成
 *    - 适合生产环境
 * 
 * 6. example_6_manual_and_voice_control()
 *    - 手动控制与语音控制结合
 *    - 支持多种控制方式
 */
