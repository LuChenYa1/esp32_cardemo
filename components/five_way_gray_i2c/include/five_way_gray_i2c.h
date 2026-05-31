#ifndef FIVE_WAY_GRAY_I2C_H
#define FIVE_WAY_GRAY_I2C_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "pin_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

// 五路灰度传感器引脚定义在 pin_definitions.h 中
// #define FIVE_WAY_GRAY_I2C_PORT      I2C_NUM_1
// #define FIVE_WAY_GRAY_I2C_SCL_GPIO  GPIO_NUM_18
// #define FIVE_WAY_GRAY_I2C_SDA_GPIO  GPIO_NUM_20
// #define FIVE_WAY_GRAY_I2C_ADDR      0x4F
// #define FIVE_WAY_GRAY_I2C_FREQ_HZ   400000

/**
 * @brief 五路灰度传感器数量
 */
#define FIVE_WAY_GRAY_SENSOR_COUNT  5

/**
 * @brief 默认黑线检测阈值
 * 
 * 传感器值小于此阈值时判定为黑线
 * 范围：0-1023（10位ADC）
 * 注意：传感器实际输出为10位ADC值，不是12位
 */
#define FIVE_WAY_GRAY_BLACK_THRESHOLD  512

/**
 * @brief 五路灰度传感器校准数据结构
 */
typedef struct {
    uint16_t white_value;   // 白色区域传感器值
    uint16_t black_value;   // 黑线区域传感器值
    bool is_calibrated;     // 是否已校准
} five_way_gray_calibration_t;

/**
 * @brief 初始化五路灰度传感器
 * 
 * 配置独立I2C总线（I2C_NUM_1，GPIO18/20），创建后台扫描任务
 * 后台任务以2ms周期自动读取传感器数据并缓存
 * 
 * 注意：与二路ADC灰度传感器互斥使用（共用GPIO18/20）
 * 
 * @return 
 *     - ESP_OK 初始化成功
 *     - ESP_FAIL 初始化失败
 */
esp_err_t five_way_gray_i2c_init(void);

/**
 * @brief 获取缓存的传感器值（中断安全）
 * 
 * 从后台任务缓存中读取最新的传感器数据
 * 使用原子操作，可在中断中安全调用
 * 
 * @param values 5个uint16_t的数组，用于存储传感器值（0-1023，10位ADC）
 *               索引0-4分别对应传感器1-5
 */
void five_way_gray_i2c_get_values(uint16_t values[FIVE_WAY_GRAY_SENSOR_COUNT]);

/**
 * @brief 获取单个传感器的值
 * 
 * @param index 传感器索引（0-4）
 * @return 传感器值（0-1023），索引无效时返回0
 */
uint16_t five_way_gray_i2c_get_value(uint8_t index);

/**
 * @brief 检测是否有任意传感器检测到黑线
 * 
 * @param threshold 黑线检测阈值，传感器值小于此值判定为黑线
 *                  使用0则采用默认阈值FIVE_WAY_GRAY_BLACK_THRESHOLD
 * @return 
 *     - true 至少一个传感器检测到黑线
 *     - false 所有传感器都在白色区域
 */
bool five_way_gray_i2c_any_black_detected(uint16_t threshold);

/**
 * @brief 获取统计信息
 * 
 * @param read_count 输出参数，累计读取次数，可为NULL
 * @param error_count 输出参数，累计错误次数，可为NULL
 */
void five_way_gray_i2c_get_stats(uint32_t *read_count, uint32_t *error_count);

/**
 * @brief 打印传感器状态（调试用）
 * 
 * 输出5个传感器的当前值、黑线检测状态、统计信息
 */
void five_way_gray_i2c_print_status(void);

/**
 * @brief 传感器校准 - 交互式校准
 * 
 * 引导用户将传感器放置在白色区域和黑线上进行校准
 * 校准后的数据用于归一化处理和黑线检测
 * 
 * 校准流程：
 * 1. 提示将传感器放在白色区域，5秒后开始采集
 * 2. 采集100次，记录最大值作为白色基准
 * 3. 提示将传感器放在黑线上，5秒后开始采集
 * 4. 采集100次，记录最小值作为黑线基准
 * 5. 保存校准数据
 */
void five_way_gray_i2c_calibrate(void);

/**
 * @brief 手动设置校准参数
 * 
 * @param index 传感器索引（0-4）
 * @param white_value 白色区域传感器值
 * @param black_value 黑线区域传感器值
 */
void five_way_gray_i2c_set_calibration(uint8_t index, uint16_t white_value, uint16_t black_value);

/**
 * @brief 获取校准参数
 * 
 * @param index 传感器索引（0-4）
 * @return 校准数据结构指针，索引无效时返回NULL
 */
const five_way_gray_calibration_t* five_way_gray_i2c_get_calibration(uint8_t index);

/**
 * @brief 读取归一化后的传感器值
 * 
 * 根据校准数据将传感器值归一化到0.0-1.0范围
 * 0.0 = 黑线, 1.0 = 白色
 * 
 * @param index 传感器索引（0-4）
 * @return 归一化值（0.0-1.0），未校准或索引无效返回-1.0
 */
float five_way_gray_i2c_read_normalized(uint8_t index);

/**
 * @brief 读取所有传感器的归一化值
 * 
 * @param normalized 5个float的数组，用于存储归一化值
 */
void five_way_gray_i2c_read_all_normalized(float normalized[FIVE_WAY_GRAY_SENSOR_COUNT]);

#ifdef __cplusplus
}
#endif

#endif // FIVE_WAY_GRAY_I2C_H
