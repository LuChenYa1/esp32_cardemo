/**
 * @file turn_statemachine.c
 * @brief 六状态转弯状态机实现
 */

#include "turn_statemachine.h"
#include "turn_detector.h"
#include "gray_sensor.h"
#include "pwm.h"
#include "esp_log.h"
#include <stdatomic.h>

static const char *TAG = "turn_statemachine";

// ==================== 共享变量（原子保护） ====================

// 状态机当前状态（原子变量）
static _Atomic TurnState_t g_turn_state = TURN_IDLE;

// ==================== 内部变量（仅在Timer 1中断中访问） ====================

// 当前状态持续的tick数（1 tick = 10ms）
static uint16_t g_tick_count = 0;

// 转弯方向：1=左转，2=右转
static uint8_t g_turn_dir = 0;

// ==================== 可配置参数（volatile，主循环写，中断读） ====================

// 后退时间（单位：tick，1 tick = 10ms）
static volatile uint8_t g_turn_back_ticks = TURN_BACK_TICKS_DEFAULT;

// ==================== 内部辅助函数 ====================

/**
 * @brief 停止所有电机
 */
static inline void IRAM_ATTR car_stop(void)
{
    set_motor1_speed(0, 0);
    set_motor2_speed(0, 0);
    set_motor3_speed(0, 0);
    set_motor4_speed(0, 0);
}

/**
 * @brief 小车后退
 * @param speed 后退速度（0-1023）
 */
static inline void IRAM_ATTR car_move_backward(uint16_t speed)
{
    // 后退：所有车轮反转
    set_motor1_speed(0, speed);
    set_motor2_speed(0, speed);
    set_motor3_speed(0, speed);
    set_motor4_speed(0, speed);
}

/**
 * @brief 小车右转
 * @param speed 转弯速度（0-1023）
 */
static inline void IRAM_ATTR car_turn_right(uint16_t speed)
{
    // 右转：左轮正转，右轮反转
    set_motor1_speed(speed, 0);   // 左前轮正转
    set_motor2_speed(speed, 0);  // 左后轮正转
    set_motor3_speed(0, speed);  // 右前轮反转
    set_motor4_speed(0, speed);  // 右后轮反转
}

/**
 * @brief 小车左转
 * @param speed 转弯速度（0-1023）
 */
static inline void IRAM_ATTR car_turn_left(uint16_t speed)
{
    // 左转：左轮反转，右轮正转
    set_motor1_speed(0, speed);   // 左前轮反转
    set_motor2_speed(0, speed);  // 左后轮反转
    set_motor3_speed(speed, 0);  // 右前轮正转
    set_motor4_speed(speed, 0);  // 右后轮正转
}

// ==================== 公共接口实现 ====================

void turn_statemachine_init(void)
{
    ESP_LOGI(TAG, "初始化转弯状态机...");
    
    // 初始化状态变量
    atomic_store(&g_turn_state, TURN_IDLE);
    
    // 初始化内部变量
    g_tick_count = 0;
    g_turn_dir = 0;
    
    ESP_LOGI(TAG, "转弯状态机初始化完成");
    ESP_LOGI(TAG, "  停车时间: %d ticks (100ms)", TURN_STOP_TICKS);
    ESP_LOGI(TAG, "  后退时间: %d ticks (%dms)", g_turn_back_ticks, g_turn_back_ticks * 10);
    ESP_LOGI(TAG, "  微调时间: %d ticks (100ms)", TURN_ADJUST_TICKS);
}

void turn_statemachine_tick(void)
{
    TurnState_t current_state = atomic_load(&g_turn_state);
    
    switch (current_state) {
        case TURN_IDLE:
        {
            // 检查转弯请求
            uint8_t turn_type = turn_detector_get_type();
            
            if (turn_type == TURN_TYPE_CROSS) {  // 十字路口
                // 默认右转
                g_turn_dir = 2;
                
                // 停止所有电机
                car_stop();
                
                // 重置tick计数器
                g_tick_count = 0;
                
                // 转换到TURN_STOP状态
                atomic_store(&g_turn_state, TURN_STOP);
            }
            // 可选：支持左转和右转检测
            else if (turn_type == TURN_TYPE_LEFT) {
                g_turn_dir = 1;
                car_stop();
                g_tick_count = 0;
                atomic_store(&g_turn_state, TURN_STOP);
            }
            else if (turn_type == TURN_TYPE_RIGHT) {
                g_turn_dir = 2;
                car_stop();
                g_tick_count = 0;
                atomic_store(&g_turn_state, TURN_STOP);
            }
            break;
        }
        
        case TURN_STOP:
        {
            // 停车100ms（10 ticks）
            g_tick_count++;
            
            if (g_tick_count >= TURN_STOP_TICKS) {
                // 开始后退
                car_move_backward(TURN_BACK_SPEED);
                
                // 重置tick计数器
                g_tick_count = 0;
                
                // 转换到TURN_BACK状态
                atomic_store(&g_turn_state, TURN_BACK);
            }
            break;
        }
        
        case TURN_BACK:
        {
            // 后退190ms（19 ticks）
            g_tick_count++;
            
            if (g_tick_count >= g_turn_back_ticks) {
                // 停车
                car_stop();
                
                // 重置tick计数器
                g_tick_count = 0;
                
                // 转换到TURN_PHASE1状态
                atomic_store(&g_turn_state, TURN_PHASE1);
            }
            break;
        }
        
        case TURN_PHASE1:
        {
            // 转弯第一阶段：转弯直到传感器离开黑线
            g_tick_count++;
            
            // 读取缓存的ADC值
            uint16_t left_raw, right_raw;
            gray_scanner_get_cached_values(&left_raw, &right_raw);
            
            if (g_turn_dir == 2) {
                // 右转：等左传感器离开黑线（最少250ms = 25 ticks）
                car_turn_right(TURN_SPEED);
                
                bool sensor_off = (left_raw >= LEFT_THRESHOLD);
                
                if (sensor_off && g_tick_count >= TURN_PHASE1_MIN_TICKS_RIGHT) {
                    // 传感器已离开黑线，进入第二阶段
                    g_tick_count = 0;
                    atomic_store(&g_turn_state, TURN_PHASE2);
                }
            }
            else if (g_turn_dir == 1) {
                // 左转：等右传感器离开黑线（最少150ms = 15 ticks）
                car_turn_left(TURN_SPEED);
                
                bool sensor_off = (right_raw >= RIGHT_THRESHOLD);
                
                if (sensor_off && g_tick_count >= TURN_PHASE1_MIN_TICKS_LEFT) {
                    // 传感器已离开黑线，进入第二阶段
                    g_tick_count = 0;
                    atomic_store(&g_turn_state, TURN_PHASE2);
                }
            }
            break;
        }
        
        case TURN_PHASE2:
        {
            // 转弯第二阶段：转弯直到传感器找到新黑线
            
            // 读取缓存的ADC值
            uint16_t left_raw, right_raw;
            gray_scanner_get_cached_values(&left_raw, &right_raw);
            
            if (g_turn_dir == 2) {
                // 右转：等右传感器找到新黑线
                car_turn_right(TURN_SPEED);
                
                if (right_raw < RIGHT_THRESHOLD) {
                    // 找到新黑线，停车
                    car_stop();
                    
                    // 重置tick计数器
                    g_tick_count = 0;
                    
                    // 转换到TURN_ADJUST状态
                    atomic_store(&g_turn_state, TURN_ADJUST);
                }
            }
            else if (g_turn_dir == 1) {
                // 左转：等左传感器找到新黑线
                car_turn_left(TURN_SPEED);
                
                if (left_raw < LEFT_THRESHOLD) {
                    // 找到新黑线，停车
                    car_stop();
                    
                    // 重置tick计数器
                    g_tick_count = 0;
                    
                    // 转换到TURN_ADJUST状态
                    atomic_store(&g_turn_state, TURN_ADJUST);
                }
            }
            break;
        }
        
        case TURN_ADJUST:
        {
            // 微调停车100ms（10 ticks）
            g_tick_count++;
            
            if (g_tick_count >= TURN_ADJUST_TICKS) {
                // 清除转弯标志
                turn_detector_clear_flags();
                
                // 回到TURN_IDLE状态
                atomic_store(&g_turn_state, TURN_IDLE);
                
                // 重置内部变量
                g_tick_count = 0;
                g_turn_dir = 0;
            }
            break;
        }
        
        default:
            // 未知状态，回到IDLE
            atomic_store(&g_turn_state, TURN_IDLE);
            g_tick_count = 0;
            g_turn_dir = 0;
            break;
    }
}

TurnState_t turn_statemachine_get_state(void)
{
    return atomic_load(&g_turn_state);
}

void turn_statemachine_get_debug_info(uint16_t *tick_count, uint8_t *turn_dir)
{
    if (tick_count != NULL) {
        *tick_count = g_tick_count;
    }
    if (turn_dir != NULL) {
        *turn_dir = g_turn_dir;
    }
}

void turn_statemachine_set_back_ticks(uint8_t ticks)
{
    g_turn_back_ticks = ticks;
    ESP_LOGI(TAG, "后退时间已设置为: %d ticks (%dms)", ticks, ticks * 10);
}

uint8_t turn_statemachine_get_back_ticks(void)
{
    return g_turn_back_ticks;
}
