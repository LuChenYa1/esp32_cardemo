/**
 * @file encoder.c
 * @brief 编码器驱动模块实现
 * 
 * 功能：
 * - 使用PCNT（脉冲计数器）读取编码器
 * - 支持单倍频和四倍频模式
 * - 提供速度和位置测量
 * 
 * 硬件：
 * - 编码器1-4使用GPIO41/42、45/46、14/15、16/17
 * - 每个编码器占用一个PCNT单元
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_check.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "encoder.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "encoder";

// 编码器结构体定义
struct encoder_unit_t {
    pcnt_unit_handle_t pcnt_unit_handle;
    pcnt_channel_handle_t chan_a;  // A相通道
    pcnt_channel_handle_t chan_b;  // B相通道（四倍频模式使用）
    gpio_num_t phase_a_gpio;
    gpio_num_t phase_b_gpio;
    int last_count;
    int current_count;
};

esp_err_t encoder_new_unit(const encoder_config_t *config, encoder_unit_handle_t *ret_encoder)
{
    ESP_RETURN_ON_FALSE(config, ESP_ERR_INVALID_ARG, TAG, "配置为空");
    ESP_RETURN_ON_FALSE(ret_encoder, ESP_ERR_INVALID_ARG, TAG, "返回句柄为空");

    encoder_unit_handle_t encoder = calloc(1, sizeof(struct encoder_unit_t));
    ESP_RETURN_ON_FALSE(encoder, ESP_ERR_NO_MEM, TAG, "分配编码器内存失败");

    // 配置PCNT单元
    pcnt_unit_config_t unit_config = {
        .high_limit = config->high_limit,
        .low_limit = config->low_limit,
    };

    esp_err_t ret = pcnt_new_unit(&unit_config, &encoder->pcnt_unit_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "创建PCNT单元失败");
        goto err;
    }

    // ---- 通道0：A相边沿计数，B相电平判断方向 ----
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = config->phase_a_gpio_num,
        .level_gpio_num = config->phase_b_gpio_num,
    };
    ret = pcnt_new_channel(encoder->pcnt_unit_handle, &chan_a_config, &encoder->chan_a);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "创建PCNT通道A失败");
        goto err;
    }
    // A相上升沿+1，下降沿-1
    ret = pcnt_channel_set_edge_action(encoder->chan_a, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置通道A边沿动作失败");
        goto err;
    }
    // B相高电平保持方向，低电平反转方向
    ret = pcnt_channel_set_level_action(encoder->chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置通道A电平动作失败");
        goto err;
    }

    // ---- 通道1：B相边沿计数，A相电平判断方向（四倍频/模式三）----
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = config->phase_b_gpio_num,
        .level_gpio_num = config->phase_a_gpio_num,
    };
    ret = pcnt_new_channel(encoder->pcnt_unit_handle, &chan_b_config, &encoder->chan_b);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "创建PCNT通道B失败");
        goto err;
    }
    // B相上升沿-1，下降沿+1（与通道A方向相反，保证两相叠加后方向一致）
    ret = pcnt_channel_set_edge_action(encoder->chan_b, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置通道B边沿动作失败");
        goto err;
    }
    // A相高电平保持方向，低电平反转方向
    ret = pcnt_channel_set_level_action(encoder->chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置通道B电平动作失败");
        goto err;
    }

    // 设置滤波器
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = config->filter_val,
    };
    ret = pcnt_unit_set_glitch_filter(encoder->pcnt_unit_handle, &filter_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "设置滤波器失败");
        goto err;
    }

    // 保存GPIO信息
    encoder->phase_a_gpio = config->phase_a_gpio_num;
    encoder->phase_b_gpio = config->phase_b_gpio_num;

    // 启用PCNT单元
    ret = pcnt_unit_enable(encoder->pcnt_unit_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "启用PCNT单元失败");
        goto err;
    }

    // 启动PCNT计数
    ret = pcnt_unit_start(encoder->pcnt_unit_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "启动PCNT计数失败");
        goto err;
    }

    // 清零计数器
    ret = pcnt_unit_clear_count(encoder->pcnt_unit_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "清零计数器失败");
        goto err;
    }

    *ret_encoder = encoder;
    return ESP_OK;

err:
    if (encoder) {
        free(encoder);
    }
    return ESP_FAIL;
}

esp_err_t encoder_del_unit(encoder_unit_handle_t encoder)
{
    ESP_RETURN_ON_FALSE(encoder, ESP_ERR_INVALID_ARG, TAG, "编码器句柄为空");

    // 停止并禁用PCNT单元
    pcnt_unit_stop(encoder->pcnt_unit_handle);
    pcnt_unit_disable(encoder->pcnt_unit_handle);

    // 先删除通道，再删除单元
    if (encoder->chan_a) pcnt_del_channel(encoder->chan_a);
    if (encoder->chan_b) pcnt_del_channel(encoder->chan_b);

    esp_err_t ret = pcnt_del_unit(encoder->pcnt_unit_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "删除PCNT单元失败");
        return ESP_FAIL;
    }

    free(encoder);
    return ESP_OK;
}

esp_err_t encoder_get_counter(encoder_unit_handle_t encoder, int *count)
{
    ESP_RETURN_ON_FALSE(encoder, ESP_ERR_INVALID_ARG, TAG, "编码器句柄为空");
    ESP_RETURN_ON_FALSE(count, ESP_ERR_INVALID_ARG, TAG, "计数值指针为空");

    return pcnt_unit_get_count(encoder->pcnt_unit_handle, count);
}

esp_err_t encoder_clear_count(encoder_unit_handle_t encoder)
{
    ESP_RETURN_ON_FALSE(encoder, ESP_ERR_INVALID_ARG, TAG, "编码器句柄为空");

    return pcnt_unit_clear_count(encoder->pcnt_unit_handle);
}

esp_err_t encoder_start(encoder_unit_handle_t encoder)
{
    ESP_RETURN_ON_FALSE(encoder, ESP_ERR_INVALID_ARG, TAG, "编码器句柄为空");

    return pcnt_unit_start(encoder->pcnt_unit_handle);
}

esp_err_t encoder_stop(encoder_unit_handle_t encoder)
{
    ESP_RETURN_ON_FALSE(encoder, ESP_ERR_INVALID_ARG, TAG, "编码器句柄为空");

    return pcnt_unit_stop(encoder->pcnt_unit_handle);
}

float encoder_measure_speed(encoder_unit_handle_t encoder, uint32_t duration_ms)
{
    ESP_RETURN_ON_FALSE(encoder, -1, TAG, "编码器句柄为空");

    int start_count;
    int end_count;
    float speed;

    // 获取起始计数值
    ESP_RETURN_ON_FALSE(encoder_get_counter(encoder, &start_count) == ESP_OK, -1, TAG, "获取起始计数值失败");

    // 延迟指定的时间
    vTaskDelay(pdMS_TO_TICKS(duration_ms));

    // 获取结束计数值
    ESP_RETURN_ON_FALSE(encoder_get_counter(encoder, &end_count) == ESP_OK, -1, TAG, "获取结束计数值失败");

    // 计算这段时间内的脉冲数差
    int pulse_diff = end_count - start_count;

    // 计算每秒的脉冲数 (pps - pulses per second)
    float time_sec = (float)duration_ms / 1000.0f;
    speed = (float)pulse_diff / time_sec;

    return speed;
}

float encoder_get_angle(encoder_unit_handle_t encoder)
{
    // 使用默认参数: lines=26, gear_ratio=90, scale=60
    return encoder_get_angle_with_params(encoder, 26.0f, 90.0f, 60.0f);
}

float encoder_get_angle_with_params(encoder_unit_handle_t encoder, 
                                    float lines, 
                                    float gear_ratio, 
                                    float scale)
{
    ESP_RETURN_ON_FALSE(encoder, 0.0f, TAG, "编码器句柄为空");

    int count;
    esp_err_t ret = encoder_get_counter(encoder, &count);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "获取编码器计数失败");
        return 0.0f;
    }

    // 计算角度: angle = count / lines / gear_ratio * scale
    // 参考STM32公式: angle = encoder_count / 26.0f / 90.0f * 60.0f
    float angle = (float)count / lines / gear_ratio * scale;

    return angle;
}

float encoder_get_rpm(encoder_unit_handle_t encoder, 
                      uint32_t duration_ms, 
                      uint32_t pulses_per_revolution)
{
    ESP_RETURN_ON_FALSE(encoder, -1.0f, TAG, "编码器句柄为空");
    ESP_RETURN_ON_FALSE(pulses_per_revolution > 0, -1.0f, TAG, "每转脉冲数必须大于0");

    int start_count;
    int end_count;

    // 获取起始计数值
    ESP_RETURN_ON_FALSE(encoder_get_counter(encoder, &start_count) == ESP_OK, -1.0f, TAG, "获取起始计数值失败");

    // 延迟指定的时间
    vTaskDelay(pdMS_TO_TICKS(duration_ms));

    // 获取结束计数值
    ESP_RETURN_ON_FALSE(encoder_get_counter(encoder, &end_count) == ESP_OK, -1.0f, TAG, "获取结束计数值失败");

    // 计算这段时间内的脉冲数差
    int pulse_diff = end_count - start_count;

    // 计算转数
    float revolutions = (float)pulse_diff / (float)pulses_per_revolution;

    // 计算时间（分钟）
    float time_min = (float)duration_ms / 60000.0f;

    // 计算RPM
    float rpm = revolutions / time_min;

    return rpm;
}

/* ============================================================
 * GPIO 外部中断版编码器实现
 * 四倍频：A、B 两相的上升沿 + 下降沿均触发中断
 * 方向判断：中断触发时读取另一相的电平来决定 +1 / -1
 * ============================================================ */
#include "freertos/portmacro.h"

struct encoder_gpio_unit_t {
    gpio_num_t phase_a;
    gpio_num_t phase_b;
    volatile int32_t count;  // 原子操作目标，用 portENTER_CRITICAL 保护
    portMUX_TYPE mux;
};

/* A 相中断服务函数 */
static void IRAM_ATTR encoder_gpio_isr_a(void *arg)
{
    struct encoder_gpio_unit_t *enc = (struct encoder_gpio_unit_t *)arg;
    // 读取当前 A、B 电平
    int a = gpio_get_level(enc->phase_a);
    int b = gpio_get_level(enc->phase_b);
    // A 相边沿：a == b 时正转 +1，否则反转 -1
    portENTER_CRITICAL_ISR(&enc->mux);
    enc->count += (a == b) ? 1 : -1;
    portEXIT_CRITICAL_ISR(&enc->mux);
}

/* B 相中断服务函数 */
static void IRAM_ATTR encoder_gpio_isr_b(void *arg)
{
    struct encoder_gpio_unit_t *enc = (struct encoder_gpio_unit_t *)arg;
    int a = gpio_get_level(enc->phase_a);
    int b = gpio_get_level(enc->phase_b);
    // B 相边沿：a != b 时正转 +1，否则反转 -1
    portENTER_CRITICAL_ISR(&enc->mux);
    enc->count += (a != b) ? 1 : -1;
    portEXIT_CRITICAL_ISR(&enc->mux);
}

esp_err_t encoder_gpio_new_unit(gpio_num_t phase_a, gpio_num_t phase_b,
                                encoder_gpio_handle_t *ret_enc)
{
    ESP_RETURN_ON_FALSE(ret_enc, ESP_ERR_INVALID_ARG, TAG, "返回句柄为空");

    struct encoder_gpio_unit_t *enc = calloc(1, sizeof(struct encoder_gpio_unit_t));
    ESP_RETURN_ON_FALSE(enc, ESP_ERR_NO_MEM, TAG, "分配GPIO编码器内存失败");

    enc->phase_a = phase_a;
    enc->phase_b = phase_b;
    enc->count   = 0;
    enc->mux     = (portMUX_TYPE)portMUX_INITIALIZER_UNLOCKED;

    // 配置 A、B 相 GPIO：输入模式，双边沿中断
    gpio_config_t io_cfg = {
        .pin_bit_mask = (1ULL << phase_a) | (1ULL << phase_b),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_ANYEDGE,  // 上升沿 + 下降沿都触发
    };
    esp_err_t ret = gpio_config(&io_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO配置失败");
        free(enc);
        return ret;
    }

    // 安装 GPIO 中断服务（若已安装则忽略错误）
    gpio_install_isr_service(0);

    ret = gpio_isr_handler_add(phase_a, encoder_gpio_isr_a, enc);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "添加A相中断失败");
        free(enc);
        return ret;
    }
    ret = gpio_isr_handler_add(phase_b, encoder_gpio_isr_b, enc);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "添加B相中断失败");
        gpio_isr_handler_remove(phase_a);
        free(enc);
        return ret;
    }

    *ret_enc = enc;
    ESP_LOGI(TAG, "GPIO中断编码器初始化完成 A=%d B=%d", phase_a, phase_b);
    return ESP_OK;
}

esp_err_t encoder_gpio_del_unit(encoder_gpio_handle_t enc)
{
    ESP_RETURN_ON_FALSE(enc, ESP_ERR_INVALID_ARG, TAG, "句柄为空");
    gpio_isr_handler_remove(enc->phase_a);
    gpio_isr_handler_remove(enc->phase_b);
    free(enc);
    return ESP_OK;
}

int32_t encoder_gpio_get_count(encoder_gpio_handle_t enc)
{
    if (!enc) return 0;
    portENTER_CRITICAL(&enc->mux);
    int32_t val = enc->count;
    portEXIT_CRITICAL(&enc->mux);
    return val;
}

void encoder_gpio_clear_count(encoder_gpio_handle_t enc)
{
    if (!enc) return;
    portENTER_CRITICAL(&enc->mux);
    enc->count = 0;
    portEXIT_CRITICAL(&enc->mux);
}

float encoder_gpio_get_rpm(encoder_gpio_handle_t enc,
                           uint32_t duration_ms,
                           uint32_t pulses_per_revolution)
{
    if (!enc || pulses_per_revolution == 0) return 0.0f;

    int32_t start = encoder_gpio_get_count(enc);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    int32_t end = encoder_gpio_get_count(enc);

    int32_t diff = end - start;
    float revolutions = (float)diff / (float)pulses_per_revolution;
    float time_min    = (float)duration_ms / 60000.0f;
    return revolutions / time_min;
}
