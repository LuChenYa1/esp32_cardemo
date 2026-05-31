#ifndef GRAY_SENSOR_H
#define GRAY_SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

// 高频采样缓冲区大小
#define ADC_SAMPLE_BUFFER_SIZE 10
#define ADC_SAMPLE_COUNT 2  // 两个通道

/**
 * @brief 灰度传感器通道定义
 */
typedef enum {
    GRAY_SENSOR_LEFT = 0,   // 左侧传感器 (GPIO18, ADC2_CH7)
    GRAY_SENSOR_RIGHT = 1   // 右侧传感器 (GPIO20, ADC2_CH9)
} gray_sensor_channel_t;

/**
 * @brief 灰度传感器校准数据结构
 */
typedef struct {
    uint16_t white_value;   // 白色区域ADC值
    uint16_t black_value;   // 黑线区域ADC值
    bool is_calibrated;     // 是否已校准
} gray_sensor_calibration_t;

/**
 * @brief ADC高频采样数据结构
 */
typedef struct {
    uint16_t left_value;    // 左传感器当前值
    uint16_t right_value;   // 右传感器当前值
    bool data_ready;        // 数据就绪标志
    SemaphoreHandle_t mutex; // 互斥锁
} gray_sensor_data_t;

/**
 * @brief 简化初始化灰度传感器（不启动采样任务）
 * 
 * 配置ADC2的通道7和通道8，不创建后台采样任务
 * 引脚: GPIO19 (ADC2_CH7), GPIO20 (ADC2_CH8)
 * 适用于单任务顺序执行模式
 */
void gray_sensor_init_simple(void);

/**
 * @brief 直接读取两个传感器的原始值（阻塞方式）
 * 
 * @param left_value 左传感器ADC值输出
 * @param right_value 右传感器ADC值输出
 * @return true 成功读取，false 读取失败
 */
bool gray_sensor_read_both_raw_direct(uint16_t *left_value, uint16_t *right_value);

/**
 * @brief 初始化灰度传感器（高频采样模式）
 * 
 * 配置ADC2的通道7和通道9，启动高频采样任务
 * 引脚: GPIO18 (ADC2_CH7), GPIO20 (ADC2_CH9)
 * 采样频率: 1kHz (每1ms采样一次)
 */
void gray_sensor_init(void);

/**
 * @brief 获取当前ADC值（从缓存读取，非阻塞）
 * 
 * @param left_value 左传感器ADC值输出
 * @param right_value 右传感器ADC值输出
 * @return true 成功获取数据，false 数据未就绪
 */
bool gray_sensor_get_current_values(uint16_t *left_value, uint16_t *right_value);

/**
 * @brief 获取当前ADC值（从DMA缓冲区读取）
 * 
 * @param left_value 左传感器ADC值输出
 * @param right_value 右传感器ADC值输出
 * @return true 成功获取数据，false 数据未就绪
 */
bool gray_sensor_get_current_values(uint16_t *left_value, uint16_t *right_value);

/**
 * @brief 读取灰度传感器原始ADC值（兼容旧接口）
 * 
 * @param channel 传感器通道 (GRAY_SENSOR_LEFT 或 GRAY_SENSOR_RIGHT)
 * @return ADC原始值 (0-4095)，失败返回-1
 */
int gray_sensor_read_raw(gray_sensor_channel_t channel);

/**
 * @brief 读取两个传感器的原始值（兼容旧接口）
 * 
 * @param left_value 左传感器ADC值输出
 * @param right_value 右传感器ADC值输出
 */
void gray_sensor_read_both_raw(uint16_t *left_value, uint16_t *right_value);

/**
 * @brief 传感器校准 - 交互式校准
 * 
 * 引导用户将传感器放置在白色区域和黑线上进行校准
 * 校准后的数据用于归一化处理
 */
void gray_sensor_calibrate(void);

/**
 * @brief 手动设置校准参数
 * 
 * @param channel 传感器通道
 * @param white_value 白色区域ADC值
 * @param black_value 黑线区域ADC值
 */
void gray_sensor_set_calibration(gray_sensor_channel_t channel, 
                                 uint16_t white_value, 
                                 uint16_t black_value);

/**
 * @brief 获取校准参数
 * 
 * @param channel 传感器通道
 * @return 校准数据结构指针
 */
const gray_sensor_calibration_t* gray_sensor_get_calibration(gray_sensor_channel_t channel);

/**
 * @brief 读取归一化后的传感器值
 * 
 * 根据校准数据将ADC值归一化到0.0-1.0范围
 * 0.0 = 黑线, 1.0 = 白色
 * 
 * @param channel 传感器通道
 * @return 归一化值 (0.0-1.0)，未校准返回-1.0
 */
float gray_sensor_read_normalized(gray_sensor_channel_t channel);

/**
 * @brief 读取两个传感器的归一化值
 * 
 * @param left_norm 左传感器归一化值输出
 * @param right_norm 右传感器归一化值输出
 */
bool gray_sensor_read_both_normalized(float *left_norm, float *right_norm);

/**
 * @brief 判断传感器是否在黑线上
 * 
 * @param channel 传感器通道
 * @param threshold 阈值 (0.0-1.0)，默认0.5
 * @return true=在黑线上, false=在白色区域
 */
bool gray_sensor_is_on_black_line(gray_sensor_channel_t channel, float threshold);

/**
 * @brief 打印传感器状态 (调试用)
 */
void gray_sensor_print_status(void);

/**
 * @brief 初始化ADC采样任务（高频采样模式）
 * 
 * 创建FreeRTOS任务，以1ms周期采样ADC值并缓存
 * 使用原子变量保护共享数据
 */
void gray_scanner_init(void);

/**
 * @brief 获取缓存的ADC值（中断安全）
 * 
 * 使用原子读取操作获取缓存的ADC值
 * 可在中断中安全调用
 * 
 * @param left 左传感器ADC值输出
 * @param right 右传感器ADC值输出
 */
void gray_scanner_get_cached_values(uint16_t *left, uint16_t *right);

/**
 * @brief 获取ADC错误计数
 * 
 * @return 累计的ADC读取错误次数
 */
uint32_t gray_scanner_get_error_count(void);

#ifdef __cplusplus
}
#endif

#endif // GRAY_SENSOR_H
