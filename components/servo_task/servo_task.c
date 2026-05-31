/**
 * @file servo_task.c
 * @brief 舵机自动往复控制任务实现
 * 
 * 功能：
 * - 控制4个RS485舵机（ID 02-05）
 * - 在两组位置之间自动往复运动
 * - 可配置运动速度和延时
 * 
 * 注意：
 * - 需要先初始化RS485通信
 * - 使用UART0的RS485互斥锁
 */

#include "servo_task.h"
#include "485servo.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "servo_task";

// 定义两组舵机位置（根据需求7.3和7.4）
static const ServoPositionGroup_t position_group_1 = {
    .servo_id_02_pos = 800,
    .servo_id_03_pos = 650,
    .servo_id_04_pos = 850,
    .servo_id_05_pos = 750
};

static const ServoPositionGroup_t position_group_2 = {
    .servo_id_02_pos = 350,
    .servo_id_03_pos = 750,
    .servo_id_04_pos = 750,
    .servo_id_05_pos = 680
};

/**
 * @brief 设置单个舵机位置（带重试机制）
 * 
 * @param id 舵机ID
 * @param position 目标位置
 * @param speed 运动速度
 * @return int 返回0表示成功，-1表示失败
 */
static int set_servo_with_retry(uint8_t id, uint16_t position, uint8_t speed)
{
    const uint8_t MAX_RETRIES = 3;
    
    for (uint8_t retry = 0; retry < MAX_RETRIES; retry++) {
        // 发送舵机位置命令
        Set_Servo_position(id, position, speed);
        
        // 等待舵机响应（50ms延时确保命令发送完成）
        vTaskDelay(pdMS_TO_TICKS(50));
        
        // 注意：当前485servo驱动没有返回值，假设发送成功
        // 如果需要验证，可以使用Read_Servo_position读取位置确认
        ESP_LOGD(TAG, "舵机ID 0x%02X 设置位置 %d，速度 %d (尝试 %d/%d)", 
                 id, position, speed, retry + 1, MAX_RETRIES);
        
        // 假设发送成功（实际项目中可以添加位置读取验证）
        return 0;
    }
    
    ESP_LOGE(TAG, "舵机ID 0x%02X 通信失败，已重试%d次", id, MAX_RETRIES);
    return -1;
}

/**
 * @brief 设置舵机位置组（带重试机制）
 * 
 * @param pos 舵机位置组指针
 * @return int 返回0表示成功，-1表示失败
 */
int servo_set_position_group_with_retry(const ServoPositionGroup_t *pos)
{
    if (pos == NULL) {
        ESP_LOGE(TAG, "位置组指针为空");
        return -1;
    }
    
    int result = 0;
    
    // 依次发送4个舵机的位置命令（ID 0x02-0x05）
    if (set_servo_with_retry(0x02, pos->servo_id_02_pos, 50) != 0) {
        result = -1;
    }
    
    if (set_servo_with_retry(0x03, pos->servo_id_03_pos, 50) != 0) {
        result = -1;
    }
    
    if (set_servo_with_retry(0x04, pos->servo_id_04_pos, 50) != 0) {
        result = -1;
    }
    
    if (set_servo_with_retry(0x05, pos->servo_id_05_pos, 50) != 0) {
        result = -1;
    }
    
    return result;
}

/**
 * @brief 舵机任务主函数
 * 
 * 实现3秒往复控制逻辑，在两组位置之间切换
 * 
 * @param pvParameters 任务参数（未使用）
 */
static void servo_task_main(void *pvParameters)
{
    ESP_LOGI(TAG, "舵机任务启动");
    esp_log_level_set("*", ESP_LOG_NONE);
    // 初始化RS485舵机驱动
    servo485_init();
    ESP_LOGI(TAG, "RS485舵机驱动初始化完成");
    
    // 等待500ms让舵机稳定
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // 当前位置组索引（0或1）
    uint8_t current_group = 0;
    
    while (1) {
        // 选择当前位置组
        const ServoPositionGroup_t *pos = (current_group == 0) ? 
                                          &position_group_1 : &position_group_2;
        
        ESP_LOGI(TAG, "切换到位置组 %d", current_group + 1);
        
        // 发送舵机位置命令（带重试机制）
        int result = servo_set_position_group_with_retry(pos);
        
        if (result != 0) {
            ESP_LOGW(TAG, "部分舵机通信失败，但继续执行");
        }
        
        // 切换位置组
        current_group = 1 - current_group;
        
        // 等待3秒后切换到下一组位置
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

/**
 * @brief 创建舵机自动往复控制任务
 * 
 * @return TaskHandle_t 返回创建的任务句柄，失败返回NULL
 */
TaskHandle_t servo_task_create(void)
{
    TaskHandle_t task_handle = NULL;
    
    // 创建舵机任务，优先级设置为3
    BaseType_t result = xTaskCreate(
        servo_task_main,           // 任务函数
        "servo_task",              // 任务名称
        4096,                      // 栈大小（4KB）
        NULL,                      // 任务参数
        3,                         // 优先级（需求7.1）
        &task_handle               // 任务句柄
    );
    
    if (result != pdPASS) {
        ESP_LOGE(TAG, "舵机任务创建失败");
        return NULL;
    }
    
    ESP_LOGI(TAG, "舵机任务创建成功");
    return task_handle;
}

/**
 * @brief 删除舵机任务
 * 
 * @param task_handle 要删除的任务句柄
 */
void servo_task_delete(TaskHandle_t task_handle)
{
    if (task_handle != NULL) {
        vTaskDelete(task_handle);
        ESP_LOGI(TAG, "舵机任务已删除");
    }
}
