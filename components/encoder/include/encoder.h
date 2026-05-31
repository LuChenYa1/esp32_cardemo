#ifndef _ENCODER_H_
#define _ENCODER_H_

#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 编码器单元类型定义
 */
typedef struct encoder_unit_t* encoder_unit_handle_t;

/**
 * @brief 编码器配置结构体
 */
typedef struct {
    gpio_num_t phase_a_gpio_num;  /*!< Phase A 输入引脚 */
    gpio_num_t phase_b_gpio_num;  /*!< Phase B 输入引脚 */
    int low_limit;                /*!< PCNT 下限值 */
    int high_limit;               /*!< PCNT 上限值 */
    uint32_t filter_val;          /*!< 滤波阈值 (0 to 1023) */
} encoder_config_t;

/**
 * @brief 默认编码器配置
 */
#define DEFAULT_ENCODER_CONFIG() (encoder_config_t){ \
        .phase_a_gpio_num = GPIO_NUM_NC, \
        .phase_b_gpio_num = GPIO_NUM_NC, \
        .low_limit = -1000, \
        .high_limit = 1000, \
        .filter_val = 100 \
    }

/**
 * @brief 创建并初始化编码器单元
 *
 * @param config 编码器配置参数
 * @param ret_encoder 返回编码器句柄
 * @return
 *     - ESP_OK: 成功
 *     - ESP_ERR_INVALID_ARG: 参数错误
 *     - ESP_ERR_NO_MEM: 内存不足
 *     - ESP_FAIL: 其他错误
 */
esp_err_t encoder_new_unit(const encoder_config_t *config, encoder_unit_handle_t *ret_encoder);

/**
 * @brief 删除编码器单元
 *
 * @param encoder 编码器句柄
 * @return
 *     - ESP_OK: 成功
 *     - ESP_FAIL: 失败
 */
esp_err_t encoder_del_unit(encoder_unit_handle_t encoder);

/**
 * @brief 获取编码器当前计数值
 *
 * @param encoder 编码器句柄
 * @param count 返回计数值
 * @return
 *     - ESP_OK: 成功
 *     - ESP_FAIL: 失败
 */
esp_err_t encoder_get_counter(encoder_unit_handle_t encoder, int *count);

/**
 * @brief 清零编码器计数值
 *
 * @param encoder 编码器句柄
 * @return
 *     - ESP_OK: 成功
 *     - ESP_FAIL: 失败
 */
esp_err_t encoder_clear_count(encoder_unit_handle_t encoder);

/**
 * @brief 启动编码器计数
 *
 * @param encoder 编码器句柄
 * @return
 *     - ESP_OK: 成功
 *     - ESP_FAIL: 失败
 */
esp_err_t encoder_start(encoder_unit_handle_t encoder);

/**
 * @brief 停止编码器计数
 *
 * @param encoder 编码器句柄
 * @return
 *     - ESP_OK: 成功
 *     - ESP_FAIL: 失败
 */
esp_err_t encoder_stop(encoder_unit_handle_t encoder);

/**
 * @brief 测量编码器速度 (单位: 脉冲/秒)
 *
 * @param encoder 编码器句柄
 * @param duration_ms 测量时间间隔 (毫秒)
 * @return 脉冲数/秒
 */
float encoder_measure_speed(encoder_unit_handle_t encoder, uint32_t duration_ms);

/**
 * @brief 获取编码器角度 (使用默认参数)
 *
 * @param encoder 编码器句柄
 * @return 角度值 (度)
 */
float encoder_get_angle(encoder_unit_handle_t encoder);

/**
 * @brief 获取编码器角度 (使用自定义参数)
 *
 * @param encoder 编码器句柄
 * @param lines 编码器每转线数
 * @param gear_ratio 减速箱减速比
 * @param scale 换算比例
 * @return 角度值 (度)
 */
float encoder_get_angle_with_params(encoder_unit_handle_t encoder, 
                                    float lines, 
                                    float gear_ratio, 
                                    float scale);

/**
 * @brief 获取编码器转速 (RPM)
 *
 * @param encoder 编码器句柄
 * @param duration_ms 测量时间间隔 (毫秒)
 * @param pulses_per_revolution 每转脉冲数
 * @return RPM (转/分钟)
 */
float encoder_get_rpm(encoder_unit_handle_t encoder, 
                      uint32_t duration_ms, 
                      uint32_t pulses_per_revolution);

/* ============================================================
 * GPIO 外部中断版本编码器（用于与 PCNT 版本对比）
 * 同样实现四倍频：A、B 两相的上升沿和下降沿都触发中断计数
 * ============================================================ */

/** @brief GPIO 中断版编码器句柄 */
typedef struct encoder_gpio_unit_t* encoder_gpio_handle_t;

/**
 * @brief 创建 GPIO 中断版编码器
 *
 * @param phase_a  A 相 GPIO
 * @param phase_b  B 相 GPIO
 * @param ret_enc  返回句柄
 * @return ESP_OK / ESP_ERR_NO_MEM / ESP_FAIL
 */
esp_err_t encoder_gpio_new_unit(gpio_num_t phase_a, gpio_num_t phase_b,
                                encoder_gpio_handle_t *ret_enc);

/**
 * @brief 删除 GPIO 中断版编码器，释放资源
 */
esp_err_t encoder_gpio_del_unit(encoder_gpio_handle_t enc);

/**
 * @brief 获取当前计数值（有符号，正转为正）
 */
int32_t encoder_gpio_get_count(encoder_gpio_handle_t enc);

/**
 * @brief 清零计数值
 */
void encoder_gpio_clear_count(encoder_gpio_handle_t enc);

/**
 * @brief 获取转速 RPM
 *
 * @param enc                  句柄
 * @param duration_ms          采样时间 (ms)
 * @param pulses_per_revolution 每转脉冲数（四倍频后为线数×4）
 * @return RPM
 */
float encoder_gpio_get_rpm(encoder_gpio_handle_t enc,
                           uint32_t duration_ms,
                           uint32_t pulses_per_revolution);

#ifdef __cplusplus
}
#endif

#endif