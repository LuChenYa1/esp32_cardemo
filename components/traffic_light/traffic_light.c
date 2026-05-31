#include "traffic_light.h"
#include "esp_log.h"

static const char *TAG = "TRAFFIC_LIGHT";

// 保存当前状态
static traffic_light_state_t current_state = TRAFFIC_LIGHT_RED;
static bool is_initialized = false;  // 初始化标志

/**
 * @brief 初始化红绿灯GPIO引脚
 * 
 * 使用固定引脚：GPIO39（信号线1）、GPIO40（信号线2）
 * 
 * @return 
 *     - ESP_OK: 初始化成功
 *     - ESP_FAIL: GPIO配置失败
 */
esp_err_t traffic_light_init(void) {
    // 配置信号线1（GPIO39）
    gpio_config_t io_conf_1 = {
        .pin_bit_mask = (1ULL << TRAFFIC_LIGHT_SIGNAL1_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    esp_err_t ret = gpio_config(&io_conf_1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "信号线1 GPIO配置失败");
        return ESP_FAIL;
    }

    // 配置信号线2（GPIO40）
    gpio_config_t io_conf_2 = {
        .pin_bit_mask = (1ULL << TRAFFIC_LIGHT_SIGNAL2_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ret = gpio_config(&io_conf_2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "信号线2 GPIO配置失败");
        return ESP_FAIL;
    }

    is_initialized = true;

    ESP_LOGI(TAG, "红绿灯初始化成功 - 信号线1: GPIO%d, 信号线2: GPIO%d", 
             TRAFFIC_LIGHT_SIGNAL1_GPIO, TRAFFIC_LIGHT_SIGNAL2_GPIO);

    // 默认设置为红灯
    traffic_light_set_state(TRAFFIC_LIGHT_RED);
    
    return ESP_OK;
}

/**
 * @brief 设置红绿灯状态
 * 
 * 控制逻辑：
 * - 红灯: 10 (信号线1=高, 信号线2=低)
 * - 黄灯: 01 (信号线1=低, 信号线2=高)
 * - 绿灯: 11 (信号线1=高, 信号线2=高)
 * 
 * @param state 目标状态
 * 
 * @return 
 *     - ESP_OK: 设置成功
 *     - ESP_ERR_INVALID_STATE: 红绿灯未初始化
 */
esp_err_t traffic_light_set_state(traffic_light_state_t state) {
    if (!is_initialized) {
        ESP_LOGE(TAG, "红绿灯未初始化");
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t signal_1, signal_2;

    switch (state) {
        case TRAFFIC_LIGHT_RED:
            signal_1 = 1;  // 高电平
            signal_2 = 0;  // 低电平
            ESP_LOGI(TAG, "设置为红灯 (10)");
            break;

        case TRAFFIC_LIGHT_YELLOW:
            signal_1 = 0;  // 低电平
            signal_2 = 1;  // 高电平
            ESP_LOGI(TAG, "设置为黄灯 (01)");
            break;

        case TRAFFIC_LIGHT_GREEN:
            signal_1 = 1;  // 高电平
            signal_2 = 1;  // 高电平
            ESP_LOGI(TAG, "设置为绿灯 (11)");
            break;

        default:
            ESP_LOGW(TAG, "未知状态，默认设置为红灯");
            signal_1 = 1;
            signal_2 = 0;
            state = TRAFFIC_LIGHT_RED;
            break;
    }

    // 设置GPIO电平
    gpio_set_level(TRAFFIC_LIGHT_SIGNAL1_GPIO, signal_1);
    gpio_set_level(TRAFFIC_LIGHT_SIGNAL2_GPIO, signal_2);

    // 更新当前状态
    current_state = state;
    
    return ESP_OK;
}

/**
 * @brief 获取当前红绿灯状态
 * 
 * @return 当前状态
 */
traffic_light_state_t traffic_light_get_state(void) {
    return current_state;
}
