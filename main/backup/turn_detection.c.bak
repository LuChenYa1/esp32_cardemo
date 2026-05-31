#include "turn_detection.h"
#include "gray_sensor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdatomic.h>

static const char *TAG = "TURN_DETECT";

// 原子变量，用于多任务安全访问
static _Atomic uint8_t g_turn_type = TURN_TYPE_NONE;
static _Atomic bool g_turning_in_progress = false;
static _Atomic bool g_detection_enabled = false;

// 任务句柄
static TaskHandle_t turn_detect_task_handle = NULL;

// 检测计数器
static uint8_t left_only_count = 0;
static uint8_t right_only_count = 0;
static uint8_t both_on_count = 0;

// 确认阈值（需要连续检测多少次才确认转弯）
// 10ms周期 * 10次 = 100ms连续检测
#define CONFIRM_THRESHOLD 3

/**
 * @brief 转弯检测任务
 */
static void turn_detection_task(void *pvParameters)
{
    ESP_LOGI(TAG, "转弯检测任务启动");

    uint16_t left_raw, right_raw;
    const gray_sensor_calibration_t *left_cal;
    const gray_sensor_calibration_t *right_cal;
    uint16_t left_threshold, right_threshold;

    // // 预先获取校准数据（避免每次循环都获取）
    // left_cal = gray_sensor_get_calibration(GRAY_SENSOR_LEFT);
    // right_cal = gray_sensor_get_calibration(GRAY_SENSOR_RIGHT);
    // 计算阈值
    left_threshold = (4095 + 1150) / 2;
    right_threshold = (4095 + 1150) / 2;
    // 等待传感器稳定
    vTaskDelay(pdMS_TO_TICKS(100));

    while (1)
    {
        if (!g_turning_in_progress)
        {
            if (!g_detection_enabled)
            {
                vTaskDelay(pdMS_TO_TICKS(100)); // 未启用时延时更长
                continue;
            }

            // 读取传感器原始值（非阻塞方式）
            if (!gray_sensor_get_current_values(&left_raw, &right_raw))
            {
                // 数据未就绪，使用较长延时避免紧密循环
                vTaskDelay(pdMS_TO_TICKS(10)); // 50ms延时，给ADC采样任务更多时间
                continue;
            }

            // 判断传感器状态
            bool left_on_black = (left_raw < left_threshold);
            bool right_on_black = (right_raw < right_threshold);

            // 转弯检测逻辑
            if (left_on_black && right_on_black)
            {
                // 两个都在黑线上 - 可能是十字路口
                both_on_count++;
                left_only_count = 0;
                right_only_count = 0;

                if (both_on_count >= CONFIRM_THRESHOLD && !g_turning_in_progress)
                {
                    g_turn_type = TURN_TYPE_RIGHT;
                    g_turning_in_progress = true;
                    ESP_LOGI(TAG, "检测到十字路口");
                }
            }
            // else if (left_on_black && !right_on_black)
            // {
            //     // 只有左传感器在黑线上 - 左转标记
            //     left_only_count++;
            //     right_only_count = 0;
            //     both_on_count = 0;

            //     if (left_only_count >= CONFIRM_THRESHOLD && !g_turning_in_progress)
            //     {
            //         g_turn_type = TURN_TYPE_LEFT;
            //         g_turning_in_progress = true;
            //         ESP_LOGI(TAG, "检测到左转标记");
            //     }
            // }
            // else if (!left_on_black && right_on_black)
            // {
            //     // 只有右传感器在黑线上 - 右转标记
            //     right_only_count++;
            //     left_only_count = 0;
            //     both_on_count = 0;

            //     if (right_only_count >= CONFIRM_THRESHOLD && !g_turning_in_progress)
            //     {
            //         g_turn_type = TURN_TYPE_RIGHT;
            //         g_turning_in_progress = true;
            //         ESP_LOGI(TAG, "检测到右转标记");
            //     }
            // }
            else
            {
                both_on_count = 0;
                left_only_count = 0;
                right_only_count = 0;
            }
        }
        // 延时，让出CPU给其他任务
        // 使用20ms延时，进一步降低CPU占用，给IDLE任务更多时间
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/**
 * @brief 初始化转弯检测模块
 */
void turn_detection_init(void)
{
    ESP_LOGI(TAG, "初始化转弯检测模块...");

    // 创建检测任务（转弯检测优先级应高于巡线，但低于ADC采样）
    BaseType_t ret = xTaskCreate(
        turn_detection_task,
        "turn_detect",
        3072, // 增加栈大小
        NULL,
        4, // 优先级4：高于巡线(3)，低于ADC采样(5)
        &turn_detect_task_handle);

    if (ret != pdPASS)
    {
        ESP_LOGE(TAG, "创建转弯检测任务失败");
        return;
    }

    ESP_LOGI(TAG, "转弯检测模块初始化完成");
}

/**
 * @brief 启动转弯检测
 */
void turn_detection_start(void)
{
    g_detection_enabled = true;
    ESP_LOGI(TAG, "转弯检测已启动");
}

/**
 * @brief 停止转弯检测
 */
void turn_detection_stop(void)
{
    g_detection_enabled = false;
    ESP_LOGI(TAG, "转弯检测已停止");
}

/**
 * @brief 获取当前转弯类型
 */
uint8_t turn_detection_get_type(void)
{
    return g_turn_type;
}

/**
 * @brief 检查是否正在转弯
 */
bool turn_detection_is_turning(void)
{
    return g_turning_in_progress;
}

/**
 * @brief 重置转弯状态
 */
void turn_detection_reset(void)
{
    g_turn_type = TURN_TYPE_NONE;
    g_turning_in_progress = false;
    both_on_count = 0;
    left_only_count = 0;
    right_only_count = 0;
    ESP_LOGI(TAG, "转弯状态已重置");
}
