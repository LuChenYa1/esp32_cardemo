/**
 * @file ssdx.c
 * @brief SSD扩展接口ADC采集模块实现
 * 
 * 功能：
 * - 通过ADC2读取SSD接口的模拟信号
 * - 支持多通道ADC采集
 * 
 * 接口：
 * - 使用SSD扩展接口的ADC通道
 * 
 * 注意：
 * - ADC2与WiFi冲突，启用WiFi时不可用
 */

#include "include/ssdx.h"
#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

// 定义日志标签，用于标识此源文件的日志输出
static const char *TAG = "SSDX_ADC";

// 声明ADC单次采样单元句柄，用于管理ADC1的采样操作
static adc_oneshot_unit_handle_t adc2_handle;
//28 a6 19 a5
/**
 * @brief 初始化ADC功能
 * 
 * 此函数配置ADC1单元，并设置GPIO1和GPIO2作为ADC输入引脚
 * 配置范围为0-3.3V，适用于读取模拟电压信号
 */
void ssdx_adc_init(void) {
    // 初始化ADC1配置结构体
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_2  // 指定使用ADC单元1
    };
    // 创建新的ADC单次采样单元并检查是否成功
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc2_handle));

    // 配置ADC通道参数
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,  // 使用默认位宽（12位）
        .atten = ADC_ATTEN_DB_11,          // 设置衰减为11dB，可测量0-3.3V电压
    };

    // 配置GPIO1作为ADC通道0
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_7, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_8, &config));
    // // 配置GPIO2作为ADC通道1
    // ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_1, &config));

    // 输出初始化成功的日志信息
    // ESP_LOGI(TAG, "GPIO1 (ADC1_CH0) 已初始化成功，可以开始进行ADC转换了。");
}

/**
 * @brief 读取指定ADC通道的电压值
 * 
 * @param channel ADC通道号（0或1），分别对应GPIO1和GPIO2
 * @return 成功时返回原始值，失败时返回-1
 * 
 */
int ssdx_read_voltage(int channel) {
    int adc_raw;      // 存储ADC原始读数

    adc_channel_t adc_channel;
    
    // 根据输入参数确定使用的ADC通道
    if (channel == 7) {
        adc_channel = ADC_CHANNEL_7;  // 选择通道0（GPIO1）
    } else if (channel == 8) {
        adc_channel = ADC_CHANNEL_8;  // 选择通道1（GPIO2）
    } else {
        // 如果传入的通道号不在有效范围内，记录错误并返回-1
        ESP_LOGE(TAG, "无效通道: %d", channel);
        return -1;
    }

    // 从指定ADC通道读取原始数值，并检查是否成功
    ESP_ERROR_CHECK(adc_oneshot_read(adc2_handle, adc_channel, &adc_raw));

    return adc_raw;
}