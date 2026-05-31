#include "include/gray_sensor.h"
#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdatomic.h>

static const char *TAG = "GRAY_SENSOR";

// ADC单次采样句柄
static adc_oneshot_unit_handle_t adc2_handle = NULL;

// 校准数据
static gray_sensor_calibration_t calibration_data[2] = {
    {.white_value = 3400, .black_value = 800, .is_calibrated = false}, // 左传感器默认值
    {.white_value = 3400, .black_value = 800, .is_calibrated = false}  // 右传感器默认值
};

// ========== ADC采样任务相关变量 ==========
// 使用原子变量保护共享数据（C11标准）
static _Atomic uint16_t g_adc_left_cached = 4095;
static _Atomic uint16_t g_adc_right_cached = 4095;
static _Atomic uint32_t g_adc_error_count = 0;

// ADC采样任务句柄
static TaskHandle_t adc_sampling_task_handle = NULL;

/**
 * @brief 简化初始化灰度传感器（不启动采样任务）
 */
void gray_sensor_init_simple(void)
{
    ESP_LOGI(TAG, "初始化灰度传感器简化模式...");

    // 初始化ADC2配置
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_2};
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc2_handle));

    // 配置ADC通道参数
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT, // 12位
        .atten = ADC_ATTEN_DB_11,         // 11dB衰减，测量0-3.3V
    };

    // 配置GPIO18作为ADC2通道7（左灰度传感器）
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_7, &config));
    // 配置GPIO20作为ADC2通道9（右灰度传感器）
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL_9, &config));

    ESP_LOGI(TAG, "灰度传感器简化模式初始化完成");
}

/**
 * @brief 直接读取两个传感器的原始值（阻塞方式）
 */
bool gray_sensor_read_both_raw_direct(uint16_t *left_value, uint16_t *right_value)
{
    if (adc2_handle == NULL)
    {
        ESP_LOGE(TAG, "ADC未初始化");
        return false;
    }

    int left_raw = 4095, right_raw = 4095;

    // 读取左传感器（GPIO18/ADC2_CH7）
    if (adc_oneshot_read(adc2_handle, ADC_CHANNEL_7, &left_raw) != ESP_OK)
    {
        return false;
    }

    // 读取右传感器（GPIO20/ADC2_CH9）
    if (adc_oneshot_read(adc2_handle, ADC_CHANNEL_9, &right_raw) != ESP_OK)
    {
        return false;
    }

    if (left_value != NULL)
    {
        *left_value = (uint16_t)left_raw;
    }
    if (right_value != NULL)
    {
        *right_value = (uint16_t)right_raw;
    }

    return true;
}

/**
 * @brief 读取灰度传感器原始ADC值（兼容旧接口）
 */
int gray_sensor_read_raw(gray_sensor_channel_t channel)
{
    uint16_t left_value, right_value;

    if (!gray_sensor_read_both_raw_direct(&left_value, &right_value))
    {
        ESP_LOGE(TAG, "读取ADC数据失败");
        return -1;
    }

    if (channel == GRAY_SENSOR_LEFT)
    {
        return (int)left_value;
    }
    else if (channel == GRAY_SENSOR_RIGHT)
    {
        return (int)right_value;
    }
    else
    {
        ESP_LOGE(TAG, "无效的传感器通道: %d", channel);
        return -1;
    }
}

/**
 * @brief 读取两个传感器的原始值（兼容旧接口）
 */
void gray_sensor_read_both_raw(uint16_t *left_value, uint16_t *right_value)
{
    if (!gray_sensor_read_both_raw_direct(left_value, right_value))
    {
        // 如果读取失败，设置默认值
        if (left_value != NULL)
        {
            *left_value = 0;
        }
        if (right_value != NULL)
        {
            *right_value = 0;
        }
    }
}

/**
 * @brief 传感器校准 - 交互式校准
 */
void gray_sensor_calibrate(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "开始传感器校准...");
    ESP_LOGI(TAG, "========================================");

    // 校准白色区域
    ESP_LOGI(TAG, "请将传感器放置在白色区域上");
    ESP_LOGI(TAG, "5秒后开始采集白色数据...");
    vTaskDelay(pdMS_TO_TICKS(5000));

    uint16_t white_left = 0, white_right = 0;
    ESP_LOGI(TAG, "正在采集白色区域数据...");

    for (int i = 0; i < 100; i++)
    {
        uint16_t left, right;
        if (gray_sensor_read_both_raw_direct(&left, &right))
        {
            if (left > white_left)
                white_left = left;
            if (right > white_right)
                white_right = right;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGI(TAG, "白色区域采集完成: 左=%d, 右=%d", white_left, white_right);

    // 校准黑线区域
    ESP_LOGI(TAG, "请将传感器放置在黑线上");
    ESP_LOGI(TAG, "5秒后开始采集黑线数据...");
    vTaskDelay(pdMS_TO_TICKS(5000));

    uint16_t black_left = 4095, black_right = 4095;
    ESP_LOGI(TAG, "正在采集黑线数据...");

    for (int i = 0; i < 100; i++)
    {
        uint16_t left, right;
        if (gray_sensor_read_both_raw_direct(&left, &right))
        {
            if (left < black_left)
                black_left = left;
            if (right < black_right)
                black_right = right;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGI(TAG, "黑线区域采集完成: 左=%d, 右=%d", black_left, black_right);

    // 保存校准数据
    calibration_data[GRAY_SENSOR_LEFT].white_value = white_left;
    calibration_data[GRAY_SENSOR_LEFT].black_value = black_left;
    calibration_data[GRAY_SENSOR_LEFT].is_calibrated = true;

    calibration_data[GRAY_SENSOR_RIGHT].white_value = white_right;
    calibration_data[GRAY_SENSOR_RIGHT].black_value = black_right;
    calibration_data[GRAY_SENSOR_RIGHT].is_calibrated = true;

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "校准完成！");
    ESP_LOGI(TAG, "左传感器: 白色=%d, 黑线=%d", white_left, black_left);
    ESP_LOGI(TAG, "右传感器: 白色=%d, 黑线=%d", white_right, black_right);
    ESP_LOGI(TAG, "========================================");
}

/**
 * @brief 手动设置校准参数
 */
void gray_sensor_set_calibration(gray_sensor_channel_t channel,
                                 uint16_t white_value,
                                 uint16_t black_value)
{
    if (channel >= 2)
    {
        ESP_LOGE(TAG, "无效的传感器通道");
        return;
    }

    calibration_data[channel].white_value = white_value;
    calibration_data[channel].black_value = black_value;
    calibration_data[channel].is_calibrated = true;

    ESP_LOGI(TAG, "传感器%d校准参数已设置: 白色=%d, 黑线=%d",
             channel, white_value, black_value);
}

/**
 * @brief 获取校准参数
 */
const gray_sensor_calibration_t *gray_sensor_get_calibration(gray_sensor_channel_t channel)
{
    if (channel >= 2)
    {
        return NULL;
    }
    return &calibration_data[channel];
}

/**
 * @brief 读取归一化后的传感器值
 */
float gray_sensor_read_normalized(gray_sensor_channel_t channel)
{
    if (channel >= 2)
    {
        ESP_LOGE(TAG, "无效的传感器通道");
        return -1.0f;
    }

    // 检查是否已校准
    if (!calibration_data[channel].is_calibrated)
    {
        ESP_LOGW(TAG, "传感器%d未校准，使用默认参数", channel);
    }

    // 读取原始值
    int raw = gray_sensor_read_raw(channel);
    if (raw < 0)
    {
        return -1.0f;
    }

    // 归一化处理 (0=黑线, 1=白色)
    float normalized = (float)(raw - 1150) / (float)(4095 - 1150);

    // 限制在0-1范围内
    if (normalized < 0.0f)
        normalized = 0.0f;
    if (normalized > 1.0f)
        normalized = 1.0f;

    return normalized;
}

/**
 * @brief 读取两个传感器的归一化值
 */
bool gray_sensor_read_both_normalized(float *left_norm, float *right_norm)
{
    if (left_norm != NULL)
    {
        *left_norm = gray_sensor_read_normalized(GRAY_SENSOR_LEFT);
    }

    if (right_norm != NULL)
    {
        *right_norm = gray_sensor_read_normalized(GRAY_SENSOR_RIGHT);
    }

    // 返回true表示成功读取
    return true;
}

/**
 * @brief 判断传感器是否在黑线上
 */
bool gray_sensor_is_on_black_line(gray_sensor_channel_t channel, float threshold)
{
    float normalized = gray_sensor_read_normalized(channel);

    if (normalized < 0.0f)
    {
        return false; // 读取失败
    }

    // 归一化值小于阈值表示在黑线上
    return (normalized < threshold);
}

/**
 * @brief 打印传感器状态 (调试用)
 */
void gray_sensor_print_status(void)
{
    uint16_t left_raw, right_raw;
    float left_norm, right_norm;

    if (gray_sensor_read_both_raw_direct(&left_raw, &right_raw))
    {
        gray_sensor_read_both_normalized(&left_norm, &right_norm);

        ESP_LOGI(TAG, "左传感器: 原始=%d, 归一化=%.3f %s",
                 left_raw, left_norm,
                 (left_norm < 0.5f) ? "[黑线]" : "[白色]");

        ESP_LOGI(TAG, "右传感器: 原始=%d, 归一化=%.3f %s",
                 right_raw, right_norm,
                 (right_norm < 0.5f) ? "[黑线]" : "[白色]");
    }
    else
    {
        ESP_LOGW(TAG, "传感器数据未就绪");
    }
}

// ========== ADC采样任务实现 ==========

/**
 * @brief 安全模式函数声明（在timer_system中实现）
 */
extern void enter_safe_mode(void);

/**
 * @brief ADC采样任务
 *
 * 以1ms周期运行，阻塞式读取ADC值并更新共享变量
 * 实现错误处理：读取失败时保持上次值，连续失败100次进入安全模式
 *
 * @param pvParameters 任务参数（未使用）
 */
static void adc_sampling_task(void *pvParameters)
{
    // 上一次有效的ADC值
    uint16_t last_valid_left = 4095;
    uint16_t last_valid_right = 4095;
    uint32_t error_count = 0;

    ESP_LOGI(TAG, "ADC采样任务已启动，周期: 3ms");

    while (1)
    {
        int left_raw = 4095, right_raw = 4095;

        // 尝试读取左传感器（GPIO18/ADC2_CH7）
        esp_err_t err_left = adc_oneshot_read(adc2_handle, ADC_CHANNEL_7, &left_raw);

        // 尝试读取右传感器（GPIO20/ADC2_CH9）
        esp_err_t err_right = adc_oneshot_read(adc2_handle, ADC_CHANNEL_9, &right_raw);

        if (err_left == ESP_OK && err_right == ESP_OK)
        {
            // 读取成功，更新缓存和有效值
            atomic_store(&g_adc_left_cached, (uint16_t)left_raw);
            atomic_store(&g_adc_right_cached, (uint16_t)right_raw);
            last_valid_left = (uint16_t)left_raw;
            last_valid_right = (uint16_t)right_raw;

            // 重置错误计数
            if (error_count > 0)
            {
                error_count = 0;
                atomic_store(&g_adc_error_count, 0);
            }
        }
        else
        {
            // 读取失败，保持上次有效值（缓存值不变）
            error_count++;
            atomic_store(&g_adc_error_count, error_count);

            ESP_LOGW(TAG, "ADC读取失败 (错误计数: %lu): 左=%s, 右=%s",
                     error_count,
                     esp_err_to_name(err_left),
                     esp_err_to_name(err_right));

            // 连续失败超过200次，进入安全模式
            if (error_count >= 200) {
                ESP_LOGE(TAG, "ADC连续失败超过200次，进入安全模式");
                enter_safe_mode();
                // enter_safe_mode()会进入死循环，不会返回
            }
        }
        // 3ms周期延时
        vTaskDelay(pdMS_TO_TICKS(3));
    }
}

/**
 * @brief 初始化ADC采样任务（高频采样模式）
 *
 * 初始化ADC2并创建采样任务
 */
void gray_scanner_init(void)
{
    ESP_LOGI(TAG, "初始化灰度传感器扫描器（高频采样模式）...");

    // 如果ADC未初始化，先初始化
    if (adc2_handle == NULL)
    {
        gray_sensor_init_simple();
    }

    // 创建ADC采样任务
    BaseType_t ret = xTaskCreate(
        adc_sampling_task,        // 任务函数
        "adc_sampling",           // 任务名称
        2048,                     // 栈大小（字节）
        NULL,                     // 任务参数
        6,                        // 优先级（6，高于语音任务的4，确保ADC采样不被阻塞）
        &adc_sampling_task_handle // 任务句柄
    );

    if (ret != pdPASS)
    {
        ESP_LOGE(TAG, "创建ADC采样任务失败");
        return;
    }

    ESP_LOGI(TAG, "ADC采样任务创建成功，优先级: 6（高于语音任务，确保实时采样）");
}

/**
 * @brief 获取缓存的ADC值（中断安全）
 *
 * 使用原子读取操作，可在中断中安全调用
 *
 * @param left 左传感器ADC值输出
 * @param right 右传感器ADC值输出
 */
void gray_scanner_get_cached_values(uint16_t *left, uint16_t *right)
{
    if (left != NULL)
    {
        *left = atomic_load(&g_adc_left_cached);
    }

    if (right != NULL)
    {
        *right = atomic_load(&g_adc_right_cached);
    }
}

/**
 * @brief 获取ADC错误计数
 *
 * @return 累计的ADC读取错误次数
 */
uint32_t gray_scanner_get_error_count(void)
{
    return atomic_load(&g_adc_error_count);
}
