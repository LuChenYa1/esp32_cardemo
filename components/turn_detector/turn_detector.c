/**
 * @file turn_detector.c
 * @brief 转弯检测模块实现
 */

#include "turn_detector.h"
#include "gray_sensor.h"
#include "voice_module.h"
#include "esp_log.h"
#include <stdatomic.h>

static const char *TAG = "turn_detector";

// ==================== 共享变量（原子保护） ====================

// 转弯类型：0=无，1=左转，2=右转，3=十字路口
static _Atomic uint8_t g_turn_type = TURN_TYPE_NONE;

// 转弯进行中标志
static _Atomic bool g_turning_in_progress = false;

// ==================== 内部变量（仅在Timer 0中断中访问） ====================

// 连续检测计数器（无需原子保护，仅在中断中访问）
static uint8_t g_both_on_count = 0;
static uint8_t g_left_on_count = 0;
static uint8_t g_right_on_count = 0;

// ==================== 公共接口实现 ====================

void turn_detector_init(void)
{
    ESP_LOGI(TAG, "初始化转弯检测模块...");

    // 初始化共享变量
    atomic_store(&g_turn_type, TURN_TYPE_NONE);
    atomic_store(&g_turning_in_progress, false);

    // 初始化计数器
    g_both_on_count = 0;
    g_left_on_count = 0;
    g_right_on_count = 0;

    ESP_LOGI(TAG, "转弯检测模块初始化完成");
    ESP_LOGI(TAG, "  左传感器阈值: %d", LEFT_THRESHOLD);
    ESP_LOGI(TAG, "  右传感器阈值: %d", RIGHT_THRESHOLD);
    ESP_LOGI(TAG, "  确认次数: %d", CONFIRM_COUNT);
}

void turn_detector_tick(void)
{
    // 检查运行模式：只在巡线模式下检测转弯
    if (voice_module_get_run_mode() == 0) {
        // 命令模式：不检测转弯，重置所有计数器
        g_both_on_count = 0;
        g_left_on_count = 0;
        g_right_on_count = 0;
        return;
    }
    
    // 如果正在转弯，暂停检测
    if (atomic_load(&g_turning_in_progress))
    {
        // 重置所有计数器
        g_both_on_count = 0;
        g_left_on_count = 0;
        g_right_on_count = 0;
        return;
    }

    // 读取缓存的ADC值（中断安全）
    uint16_t left_raw, right_raw;
    gray_scanner_get_cached_values(&left_raw, &right_raw);

    // 使用阈值判断传感器是否在黑线上
    // 注意：ADC值越小表示越黑，阈值以下为黑线
    bool left_on_black = (left_raw < LEFT_THRESHOLD);
    bool right_on_black = (right_raw < RIGHT_THRESHOLD);

    // ========== 十字路口检测（两个传感器同时在黑线上） ==========
    if (left_on_black && right_on_black)
    {
        g_both_on_count++;

        // 连续15次确认后，标记为十字路口
        if (g_both_on_count >= CONFIRM_COUNT)
        {
            atomic_store(&g_turn_type, TURN_TYPE_CROSS);
            atomic_store(&g_turning_in_progress, true);
            g_both_on_count = 0; // 重置计数器
        }
    }
    else
    {
        // 未同时检测到黑线，重置计数器
        g_both_on_count = 0;
    }

// ========== 左转检测（可选功能，仅左传感器在黑线上） ==========
// 注意：此功能默认禁用，可根据需要启用
#if 0
    if (left_on_black && !right_on_black) {
        g_left_on_count++;
        
        if (g_left_on_count >= CONFIRM_COUNT) {
            atomic_store(&g_turn_type, TURN_TYPE_LEFT);
            atomic_store(&g_turning_in_progress, true);
            g_left_on_count = 0;
        }
    } else {
        g_left_on_count = 0;
    }
#endif

// ========== 右转检测（可选功能，仅右传感器在黑线上） ==========
// 注意：此功能默认禁用，可根据需要启用
#if 0
    if (!left_on_black && right_on_black) {
        g_right_on_count++;
        
        if (g_right_on_count >= CONFIRM_COUNT) {
            atomic_store(&g_turn_type, TURN_TYPE_RIGHT);
            atomic_store(&g_turning_in_progress, true);
            g_right_on_count = 0;
        }
    } else {
        g_right_on_count = 0;
    }
#endif
}

uint8_t turn_detector_get_type(void)
{
    return atomic_load(&g_turn_type);
}

bool turn_detector_is_turning(void)
{
    return atomic_load(&g_turning_in_progress);
}

void turn_detector_clear_flags(void)
{
    atomic_store(&g_turn_type, TURN_TYPE_NONE);
    atomic_store(&g_turning_in_progress, false);
}

void turn_detector_set_turning(bool in_progress)
{
    atomic_store(&g_turning_in_progress, in_progress);
}

void turn_detector_get_debug_info(uint8_t *both_count, uint8_t *left_count, uint8_t *right_count)
{
    if (both_count != NULL)
    {
        *both_count = g_both_on_count;
    }
    if (left_count != NULL)
    {
        *left_count = g_left_on_count;
    }
    if (right_count != NULL)
    {
        *right_count = g_right_on_count;
    }
}
