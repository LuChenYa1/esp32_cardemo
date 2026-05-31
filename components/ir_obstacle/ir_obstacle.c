#include "include/ir_obstacle.h"
#include "gpio_manager.h"
#include "esp_log.h"

static const char *TAG = "IR_OBSTACLE";

/**
 * @brief 初始化红外避障传感器
 */
void ir_obstacle_init(void)
{
    // 1. 注册GPIO到gpio_manager（检测冲突）
    esp_err_t ret = gpio_manager_register(IR_OBSTACLE_GPIO, GPIO_FUNC_GPIO_IN, 
                                          "ir_obstacle", "红外避障传感器");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO%d注册失败，可能与其他模块冲突", IR_OBSTACLE_GPIO);
        return;
    }

    // 2. 配置GPIO为输入模式
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << IR_OBSTACLE_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,       // 启用上拉电阻
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE          // 不使用中断
    };
    
    // 3. 应用GPIO配置
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "红外避障传感器初始化完成 (GPIO%d)", IR_OBSTACLE_GPIO);
    
    // 4. 打印初始状态
    ir_obstacle_print_status();
}

/**
 * @brief 读取红外避障传感器状态
 */
ir_obstacle_state_t ir_obstacle_read(void)
{
    int level = gpio_get_level(IR_OBSTACLE_GPIO);
    return (ir_obstacle_state_t)level;
}

/**
 * @brief 检测是否有障碍物
 */
bool ir_obstacle_is_detected(void)
{
    return (ir_obstacle_read() == IR_OBSTACLE_DETECTED);
}

/**
 * @brief 打印传感器状态（调试用）
 */
void ir_obstacle_print_status(void)
{
    ir_obstacle_state_t state = ir_obstacle_read();
    
    if (state == IR_OBSTACLE_DETECTED) {
        ESP_LOGI(TAG, "状态: 检测到障碍物 [需要避障]");
    } else {
        ESP_LOGI(TAG, "状态: 无障碍物 [可以通行]");
    }
}
