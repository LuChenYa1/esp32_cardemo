/**
 * @file pcf_buzzer.h
 * @brief PCF8574 驱动蜂鸣器播放音乐
 * 
 * 使用 PCF8574 的一个引脚通过软件 PWM 驱动蜂鸣器
 * 测试 PCF8574 在 400kHz I2C 速度下的性能
 */

#ifndef PCF_BUZZER_H
#define PCF_BUZZER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "pcf8574.h"

#ifdef __cplusplus
extern "C" {
#endif

// 音符频率定义
#define P0  0       // 休止符

// 低音频率
#define L1  262
#define L2  294
#define L3  330
#define L4  349
#define L5  392
#define L6  440
#define L7  494

// 中音频率
#define M1  523
#define M2  587
#define M3  659
#define M4  698
#define M5  784
#define M6  880
#define M7  988

// 高音频率
#define H1  1047
#define H2  1175
#define H3  1319
#define H4  1397
#define H5  1568
#define H6  1760
#define H7  1976

/**
 * @brief 音符结构
 */
typedef struct {
    uint16_t frequency;  // 音符频率 (Hz)
    float period;        // 音符持续时间（拍）
} Note;

/**
 * @brief PCF 蜂鸣器配置
 */
typedef struct {
    i2c_dev_t *pcf_device;   // PCF8574 设备句柄
    uint8_t buzzer_pin;      // 蜂鸣器引脚 (0-7)
    uint8_t duty_cycle;      // 占空比 (0-100)
    uint16_t bpm;            // 节拍速度（每分钟拍数）
} pcf_buzzer_config_t;

/**
 * @brief 初始化 PCF 蜂鸣器
 * 
 * @param config 蜂鸣器配置
 * @return esp_err_t 
 */
esp_err_t pcf_buzzer_init(const pcf_buzzer_config_t *config);

/**
 * @brief 播放音符序列
 * 
 * @param notes 音符数组
 * @param note_count 音符数量
 * @return esp_err_t 
 */
esp_err_t pcf_buzzer_play(const Note *notes, size_t note_count);

/**
 * @brief 停止播放
 * 
 * @return esp_err_t 
 */
esp_err_t pcf_buzzer_stop(void);

/**
 * @brief 检查是否正在播放
 * 
 * @return true 正在播放
 * @return false 未播放
 */
bool pcf_buzzer_is_playing(void);

/**
 * @brief 设置播放速度（BPM）
 * 
 * @param bpm 每分钟拍数
 * @return esp_err_t 
 */
esp_err_t pcf_buzzer_set_bpm(uint16_t bpm);

/**
 * @brief 反初始化 PCF 蜂鸣器
 * 
 * @return esp_err_t 
 */
esp_err_t pcf_buzzer_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // PCF_BUZZER_H
