/**
 * @file pd_controller.c
 * @brief PD控制器模块实现
 */

#include "pd_controller.h"
#include "gray_sensor.h"
#include "turn_detector.h"
#include "pwm.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "pd_controller";

// ==================== PD控制参数（volatile，主循环写，中断读） ====================

static volatile uint16_t g_line_speed = DEFAULT_SPEED;
static volatile float g_line_kp = DEFAULT_KP;
static volatile float g_line_kd = DEFAULT_KD;
static volatile bool g_pd_enabled = false;  // PD控制器使能标志（默认禁用，等待语音命令启动）

// ==================== PD控制状态（仅在Timer 1中断中访问） ====================

static float g_last_error = 0.0f;

// ==================== 内联辅助函数 ====================

/**
 * @brief 限制值在指定范围内
 */
static inline float clamp_float(float value, float min, float max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/**
 * @brief 限制整数值在指定范围内
 */
static inline int16_t clamp_int16(int16_t value, int16_t min, int16_t max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// ==================== 公共接口实现 ====================

void pd_controller_init(void)
{
    ESP_LOGI(TAG, "初始化PD控制器...");
    
    // 初始化PD参数
    g_line_speed = DEFAULT_SPEED;
    g_line_kp = DEFAULT_KP;
    g_line_kd = DEFAULT_KD;
    
    // 初始化状态（默认禁用）
    g_pd_enabled = false;
    g_last_error = 0.0f;
    
    ESP_LOGI(TAG, "PD控制器初始化完成（默认禁用）");
    ESP_LOGI(TAG, "  速度: %d", g_line_speed);
    ESP_LOGI(TAG, "  Kp: %.2f", g_line_kp);
    ESP_LOGI(TAG, "  Kd: %.2f", g_line_kd);
}

void pd_controller_tick(void)
{
    // 检查PD控制器是否使能
    if (!g_pd_enabled) {
        return;
    }
    
    // 检查是否正在转弯，转弯期间暂停PD控制
    if (turn_detector_is_turning()) {
        // // 立即停止所有电机，防止继续前进
        // set_motor1_speed(0, 0);
        // set_motor2_speed(0, 0);
        // set_motor3_speed(0, 0);
        // set_motor4_speed(0, 0);
        return;
    }
    
    // 1. 读取缓存的ADC值（中断安全）
    uint16_t left_raw, right_raw;
    gray_scanner_get_cached_values(&left_raw, &right_raw);
    
    // 2. 归一化处理（使用校准参数）
    // 归一化公式：norm = (raw - black) / (white - black)
    // 0.0 = 黑线, 1.0 = 白色
    float left_norm = (float)(left_raw - LEFT_BLACK_VALUE) / (float)(LEFT_WHITE_VALUE - LEFT_BLACK_VALUE);
    float right_norm = (float)(right_raw - RIGHT_BLACK_VALUE) / (float)(RIGHT_WHITE_VALUE - RIGHT_BLACK_VALUE);
    
    // 限制在[0, 1]范围内
    left_norm = clamp_float(left_norm, 0.0f, 1.0f);
    right_norm = clamp_float(right_norm, 0.0f, 1.0f);
    
    // 3. 计算误差（白底黑线）
    // error = 510 * (right_norm - left_norm)
    // 当右传感器更白（right_norm更大）时，误差为正，需要向右转
    // 当左传感器更白（left_norm更大）时，误差为负，需要向左转
    float error = 510.0f * (right_norm - left_norm);
    
    // 4. 应用误差死区
    // 当误差很小时（|error| < 30），认为在直线上，误差设为0
    if (fabsf(error) < ERROR_DEADZONE) {
        error = 0.0f;
    }
    
    // 5. 计算PD输出
    // output = Kp * error + Kd * (error - last_error)
    float output = g_line_kp * error + g_line_kd * (error - g_last_error);
    
    // 6. 限制输出范围在[-speed, +speed]
    float speed_f = (float)g_line_speed;
    output = clamp_float(output, -speed_f, speed_f);
    
    // 7. 计算左右轮速度
    // left_speed = speed - output
    // right_speed = speed + output
    // 当output为正时（需要向右转），左轮加速，右轮减速
    // 当output为负时（需要向左转），左轮减速，右轮加速
    int16_t left_speed = (int16_t)(speed_f - output);
    int16_t right_speed = (int16_t)(speed_f + output);
    
    // 8. 限制速度范围在[0, 1023]
    left_speed = clamp_int16(left_speed, PWM_MIN_SPEED, PWM_MAX_SPEED);
    right_speed = clamp_int16(right_speed, PWM_MIN_SPEED, PWM_MAX_SPEED);
    
    // 9. 调用PWM接口设置四个电机速度
    // 注意：根据pwm.h的接口定义，参数范围是0-1023（直接对应硬件占空比）
    // 电机1和电机2是左侧，电机3和电机4是右侧
    set_motor1_speed(left_speed, 0);    // 电机1：左前
    set_motor2_speed(left_speed, 0);   // 电机2：左后
    set_motor3_speed(right_speed, 0);  // 电机3：右前
    set_motor4_speed(right_speed, 0);  // 电机4：右后
    
    // 10. 更新状态
    g_last_error = error;
}

void pd_controller_set_params(uint16_t speed, float kp, float kd)
{
    // 限制速度范围
    if (speed > PWM_MAX_SPEED) {
        speed = PWM_MAX_SPEED;
    }
    
    g_line_speed = speed;
    g_line_kp = kp;
    g_line_kd = kd;
    
    ESP_LOGI(TAG, "PD参数已更新: 速度=%d, Kp=%.2f, Kd=%.2f", speed, kp, kd);
}

void pd_controller_get_params(uint16_t *speed, float *kp, float *kd)
{
    if (speed != NULL) {
        *speed = g_line_speed;
    }
    if (kp != NULL) {
        *kp = g_line_kp;
    }
    if (kd != NULL) {
        *kd = g_line_kd;
    }
}

float pd_controller_get_last_error(void)
{
    return g_last_error;
}

void pd_controller_reset(void)
{
    g_last_error = 0.0f;
    ESP_LOGI(TAG, "PD控制器状态已重置");
}

void pd_controller_enable(void)
{
    g_pd_enabled = true;
    ESP_LOGI(TAG, "PD控制器已启用");
}

void pd_controller_disable(void)
{
    g_pd_enabled = false;
    ESP_LOGI(TAG, "PD控制器已禁用");
}

bool pd_controller_is_enabled(void)
{
    return g_pd_enabled;
}
