/**
 * @file pcf_buzzer.c
 * @brief PCF8574 驱动蜂鸣器实现
 */

#include "pcf_buzzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"

static const char *TAG = "PCF_BUZZER";

// PWM 控制结构
typedef struct {
    uint16_t frequency;      // 当前频率
    bool enabled;            // 是否启用
    uint8_t duty_cycle;      // 占空比
} pwm_control_t;

// 全局变量
static i2c_dev_t *g_pcf_device = NULL;
static uint8_t g_buzzer_mask = 0;
static uint8_t g_pcf_state = 0;
static pwm_control_t g_pwm_control = {0};
static TaskHandle_t g_pwm_task_handle = NULL;
static TaskHandle_t g_player_task_handle = NULL;
static QueueHandle_t g_note_queue = NULL;
static bool g_is_playing = false;
static uint16_t g_bpm = 132;

// PCF8574 切换耗时补偿（微秒）
#define PCF_SWITCH_DELAY_US  123

/**
 * @brief PWM 生成任务
 * 高优先级任务，负责生成 PWM 波形
 */
static void pwm_generator_task(void *pvParameters)
{
    ESP_LOGI(TAG, "PWM 生成任务启动");
    
    while (1) {
        if (g_pwm_control.enabled && g_pwm_control.frequency > 0) {
            // 计算周期时间（微秒）
            uint32_t period_us = 1000000 / g_pwm_control.frequency;
            
            // 计算高电平和低电平时间
            uint32_t high_time = period_us * g_pwm_control.duty_cycle / 100;
            uint32_t low_time = period_us - high_time;
            
            // 补偿 PCF8574 切换耗时
            if (high_time > PCF_SWITCH_DELAY_US) {
                high_time -= PCF_SWITCH_DELAY_US;
            } else {
                high_time = 0;
            }
            
            if (low_time > PCF_SWITCH_DELAY_US) {
                low_time -= PCF_SWITCH_DELAY_US;
            } else {
                low_time = 0;
            }
            
            // 拉高
            g_pcf_state |= g_buzzer_mask;
            pcf8574_port_write(g_pcf_device, g_pcf_state);
            if (high_time > 0) {
                esp_rom_delay_us(high_time);
            }
            
            // 拉低
            g_pcf_state &= ~g_buzzer_mask;
            pcf8574_port_write(g_pcf_device, g_pcf_state);
            if (low_time > 0) {
                esp_rom_delay_us(low_time);
            }
        } else {
            // 禁用时保持低电平
            g_pcf_state &= ~g_buzzer_mask;
            pcf8574_port_write(g_pcf_device, g_pcf_state);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

/**
 * @brief 音乐播放任务
 * 负责控制音符播放流程
 */
static void music_player_task(void *pvParameters)
{
    ESP_LOGI(TAG, "音乐播放任务启动");
    
    Note note;
    
    while (1) {
        // 从队列接收音符
        if (xQueueReceive(g_note_queue, &note, portMAX_DELAY) == pdTRUE) {
            // 计算音符持续时间（毫秒）
            float note_duration_ms = (60000.0f / g_bpm) * note.period;
            
            if (note.frequency == P0) {
                // 休止符：禁用 PWM
                g_pwm_control.enabled = false;
                ESP_LOGD(TAG, "休止符: %.1f 拍 (%.0f ms)", note.period, note_duration_ms);
            } else {
                // 音符：设置频率并启用 PWM
                g_pwm_control.frequency = note.frequency;
                g_pwm_control.enabled = true;
                ESP_LOGD(TAG, "音符: %d Hz, %.1f 拍 (%.0f ms)", 
                         note.frequency, note.period, note_duration_ms);
            }
            
            // 延时音符持续时间（减去 5ms 静音间隔）
            uint32_t delay_ms = (uint32_t)note_duration_ms;
            if (delay_ms > 5) {
                vTaskDelay(pdMS_TO_TICKS(delay_ms - 5));
            }
            
            // 5ms 静音间隔（区分连续相同音符）
            g_pwm_control.enabled = false;
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}

esp_err_t pcf_buzzer_init(const pcf_buzzer_config_t *config)
{
    if (config == NULL || config->pcf_device == NULL) {
        ESP_LOGE(TAG, "无效的配置参数");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (config->buzzer_pin > 7) {
        ESP_LOGE(TAG, "无效的引脚号: %d", config->buzzer_pin);
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "初始化 PCF 蜂鸣器");
    ESP_LOGI(TAG, "  引脚: P%d", config->buzzer_pin);
    ESP_LOGI(TAG, "  占空比: %d%%", config->duty_cycle);
    ESP_LOGI(TAG, "  BPM: %d", config->bpm);
    
    // 保存配置
    g_pcf_device = config->pcf_device;
    g_buzzer_mask = 1 << config->buzzer_pin;
    g_pwm_control.duty_cycle = config->duty_cycle;
    g_bpm = config->bpm;
    
    // 初始化 PCF 状态（蜂鸣器引脚拉低）
    esp_err_t ret = pcf8574_port_read(g_pcf_device, &g_pcf_state);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "读取 PCF8574 失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    g_pcf_state &= ~g_buzzer_mask;
    ret = pcf8574_port_write(g_pcf_device, g_pcf_state);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "写入 PCF8574 失败: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // 创建音符队列
    g_note_queue = xQueueCreate(10, sizeof(Note));
    if (g_note_queue == NULL) {
        ESP_LOGE(TAG, "创建队列失败");
        return ESP_ERR_NO_MEM;
    }
    
    // 创建 PWM 生成任务（高优先级）
    BaseType_t task_ret = xTaskCreate(
        pwm_generator_task,
        "pcf_pwm",
        configMINIMAL_STACK_SIZE * 3,
        NULL,
        15,  // 高优先级
        &g_pwm_task_handle
    );
    
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "创建 PWM 任务失败");
        vQueueDelete(g_note_queue);
        return ESP_ERR_NO_MEM;
    }
    
    // 创建音乐播放任务
    task_ret = xTaskCreate(
        music_player_task,
        "music_player",
        configMINIMAL_STACK_SIZE * 3,
        NULL,
        10,  // 中等优先级
        &g_player_task_handle
    );
    
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "创建播放任务失败");
        vTaskDelete(g_pwm_task_handle);
        vQueueDelete(g_note_queue);
        return ESP_ERR_NO_MEM;
    }
    
    ESP_LOGI(TAG, "PCF 蜂鸣器初始化完成");
    return ESP_OK;
}

esp_err_t pcf_buzzer_play(const Note *notes, size_t note_count)
{
    if (notes == NULL || note_count == 0) {
        ESP_LOGE(TAG, "无效的音符数据");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (g_note_queue == NULL) {
        ESP_LOGE(TAG, "蜂鸣器未初始化");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "开始播放音乐，共 %d 个音符", note_count);
    g_is_playing = true;
    
    // 将所有音符发送到队列
    for (size_t i = 0; i < note_count; i++) {
        if (xQueueSend(g_note_queue, &notes[i], pdMS_TO_TICKS(100)) != pdTRUE) {
            ESP_LOGW(TAG, "队列已满，音符 %d 发送失败", i);
        }
    }
    
    ESP_LOGI(TAG, "音符已全部加入播放队列");
    return ESP_OK;
}

esp_err_t pcf_buzzer_stop(void)
{
    ESP_LOGI(TAG, "停止播放");
    
    // 清空队列
    if (g_note_queue != NULL) {
        xQueueReset(g_note_queue);
    }
    
    // 禁用 PWM
    g_pwm_control.enabled = false;
    g_is_playing = false;
    
    // 确保蜂鸣器引脚拉低
    if (g_pcf_device != NULL) {
        g_pcf_state &= ~g_buzzer_mask;
        pcf8574_port_write(g_pcf_device, g_pcf_state);
    }
    
    return ESP_OK;
}

bool pcf_buzzer_is_playing(void)
{
    if (g_note_queue == NULL) {
        return false;
    }
    
    // 检查队列是否为空
    return (uxQueueMessagesWaiting(g_note_queue) > 0) || g_pwm_control.enabled;
}

esp_err_t pcf_buzzer_set_bpm(uint16_t bpm)
{
    if (bpm < 40 || bpm > 240) {
        ESP_LOGE(TAG, "无效的 BPM 值: %d (范围: 40-240)", bpm);
        return ESP_ERR_INVALID_ARG;
    }
    
    g_bpm = bpm;
    ESP_LOGI(TAG, "设置 BPM: %d", bpm);
    return ESP_OK;
}

esp_err_t pcf_buzzer_deinit(void)
{
    ESP_LOGI(TAG, "反初始化 PCF 蜂鸣器");
    
    // 停止播放
    pcf_buzzer_stop();
    
    // 删除任务
    if (g_pwm_task_handle != NULL) {
        vTaskDelete(g_pwm_task_handle);
        g_pwm_task_handle = NULL;
    }
    
    if (g_player_task_handle != NULL) {
        vTaskDelete(g_player_task_handle);
        g_player_task_handle = NULL;
    }
    
    // 删除队列
    if (g_note_queue != NULL) {
        vQueueDelete(g_note_queue);
        g_note_queue = NULL;
    }
    
    g_pcf_device = NULL;
    
    return ESP_OK;
}
