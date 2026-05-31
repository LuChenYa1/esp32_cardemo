/**
 * @file display_task.c
 * @brief 数码管轮播显示任务实现
 * 
 * 显示内容：
 * - 温湿度（DHT11）：格式 TTHH（温度+湿度）
 * - 超声波测距（HC-SR04）：单位 cm
 * - 红外避障状态：0=无障碍，1=有障碍
 */

#include "display_task.h"
#include "esp_log.h"
#include "driver/gpio.h"

// 外设驱动头文件
#include "tm1637.h"
#include "dht11.h"
#include "hc_sr04.h"
#include "ir_obstacle.h"
#include "pin_definitions.h"

/* ==================== 日志标签 ==================== */
static const char *TAG = "DISPLAY_TASK";

/* ==================== 内部函数声明 ==================== */

/**
 * @brief 读取超声波距离数据（带范围限制）
 * @return 距离值（cm），范围0-9999
 */
static uint16_t read_ultrasonic_distance_safe(void);

/* ==================== 内部函数实现 ==================== */

/**
 * @brief 读取超声波距离数据（带范围限制）
 */
static uint16_t read_ultrasonic_distance_safe(void)
{
    float dist_cm = hc_sr04_task();
    
    // 限制范围在0-9999
    if (dist_cm < 0.0f) {
        dist_cm = 0.0f;
    }
    if (dist_cm > 9999.0f) {
        dist_cm = 9999.0f;
    }
    
    return (uint16_t)dist_cm;
}

/* ==================== 对外接口实现 ==================== */

/**
 * @brief 数码管轮播显示任务入口函数
 */
void display_task(void *pvParameters)
{
    ESP_LOGI(TAG, "数码管轮播显示任务启动");
    
    // ========== 初始化所有外设 ==========
    
    // 1. 初始化TM1637数码管
    tm1637_init();
    ESP_LOGI(TAG, "TM1637数码管初始化完成");
    
    // 2. DHT11温湿度传感器（无需初始化）
    ESP_LOGI(TAG, "DHT11温湿度传感器就绪(GPIO%d)", DHT11_DATA_GPIO);
    
    // 3. 超声波传感器（GPIO在hc_sr04.c中配置）
    ESP_LOGI(TAG, "HC-SR04超声波传感器就绪");
    
    // 4. 初始化红外避障传感器
    ir_obstacle_init();
    ESP_LOGI(TAG, "红外避障传感器就绪", IR_OBSTACLE_GPIO);
    
    // ========== 轮播显示主循环 ==========
    
    DisplayMode_t mode = DISPLAY_MODE_TEMP_HUMI;  // 初始模式：温湿度
    uint32_t mode_ticks = 0;
    
    // 各传感器读取计数器
    uint32_t dht11_read_ticks = 0;
    uint32_t ultrasonic_read_ticks = 0;
    uint32_t ir_read_ticks = 0;
    
    // 计算每个模式持续的刷新次数：5000ms / 200ms = 25次
    const uint32_t ticks_per_mode = DISPLAY_MODE_INTERVAL_MS / DISPLAY_REFRESH_MS;
    
    // 传感器读取间隔配置（降低频率，避免阻塞中断）
    const uint32_t dht11_read_interval = 10;       // DHT11: 2秒读取一次（200ms × 10 = 2000ms，阻塞20-30ms）
    const uint32_t ultrasonic_read_interval = 5;   // HC-SR04: 1秒读取一次（200ms × 5 = 1000ms，阻塞最多100ms）
    const uint32_t ir_read_interval = 2;           // 红外避障: 400ms读取一次（200ms × 2 = 400ms，读取很快）
    
    // 缓存值（用于在两次读取之间显示）
    uint16_t cached_temp_humi_value = 2550;        // 默认 25°C, 50%
    uint16_t cached_ultrasonic_distance = 0;       // 默认 0cm
    uint16_t cached_ir_obstacle_state = 0;         // 默认 0=无障碍
    
    ESP_LOGI(TAG, "开始轮播显示（刷新间隔：%dms，模式切换间隔：%dms）", 
             DISPLAY_REFRESH_MS, DISPLAY_MODE_INTERVAL_MS);
    ESP_LOGI(TAG, "传感器读取间隔 - DHT11:2秒, HC-SR04:1秒, 红外避障:400ms");
    
    while (1) {
        uint16_t display_value = 0;
        
        // 根据当前模式读取传感器数据
        switch (mode) {
            case DISPLAY_MODE_TEMP_HUMI: {
                // 温湿度模式：格式TTHH（前2位温度，后2位湿度）
                // 优化：每2秒才读取一次DHT11，避免频繁阻塞中断
                dht11_read_ticks++;
                if (dht11_read_ticks >= dht11_read_interval) {
                    dht11_read_ticks = 0;
                    
                    // 读取DHT11（会阻塞约20-30ms）
                    int16_t temp_raw, humi_raw;
                    esp_err_t ret = dht11_read_data(DHT11_DATA_GPIO, &humi_raw, &temp_raw);
                    
                    if (ret == ESP_OK) {
                        // 读取成功，更新缓存值
                        uint8_t temp = temp_raw / 10;
                        uint8_t humi = humi_raw / 10;
                        
                        // 限制范围防止溢出
                        if (temp > 99) temp = 99;
                        if (humi > 99) humi = 99;
                        
                        cached_temp_humi_value = (uint16_t)(temp * 100 + humi);
                        ESP_LOGI(TAG, "DHT11读取成功: 温度=%d°C, 湿度=%d%%", temp, humi);
                    } else {
                        ESP_LOGW(TAG, "DHT11读取失败（错误码：0x%x），使用缓存值", ret);
                    }
                }
                
                // 使用缓存值显示（避免每次都读取）
                display_value = cached_temp_humi_value;
                break;
            }
            
            case DISPLAY_MODE_ULTRASONIC: {
                // 超声波测距模式：显示距离值（cm，范围0-9999）
                // 优化：每1秒才读取一次，避免频繁阻塞（HC-SR04最多阻塞100ms！）
                ultrasonic_read_ticks++;
                if (ultrasonic_read_ticks >= ultrasonic_read_interval) {
                    ultrasonic_read_ticks = 0;
                    cached_ultrasonic_distance = read_ultrasonic_distance_safe();
                    ESP_LOGD(TAG, "HC-SR04读取: %d cm", cached_ultrasonic_distance);
                }
                display_value = cached_ultrasonic_distance;
                break;
            }
            
            case DISPLAY_MODE_IR_OBSTACLE: {
                // 红外避障状态模式：显示0=无障碍，1=有障碍
                // 优化：每400ms读取一次
                ir_read_ticks++;
                if (ir_read_ticks >= ir_read_interval) {
                    ir_read_ticks = 0;
                    cached_ir_obstacle_state = ir_obstacle_is_detected() ? 1 : 0;
                    ESP_LOGD(TAG, "红外避障状态: %d", cached_ir_obstacle_state);
                }
                display_value = cached_ir_obstacle_state;
                break;
            }
            
            default:
                ESP_LOGE(TAG, "未知显示模式：%d", mode);
                display_value = 0;
                break;
        }
        
        // 更新数码管显示
        tm1637_disp_num_process(display_value);
        
        // 模式切换逻辑
        mode_ticks++;
        if (mode_ticks >= ticks_per_mode) {
            mode_ticks = 0;
            mode = (DisplayMode_t)((mode + 1) % DISPLAY_MODE_COUNT);
            
            // 打印模式切换信息
            const char *mode_names[] = {"温湿度", "超声波测距", "红外避障状态"};
            ESP_LOGI(TAG, "切换显示模式 -> %s", mode_names[mode]);
        }
        
        // 等待下次刷新
        vTaskDelay(pdMS_TO_TICKS(DISPLAY_REFRESH_MS));
    }
}

/**
 * @brief 创建数码管显示任务
 */
esp_err_t display_task_create(void)
{
    // 将 display_task 固定到 Core 1，避免阻塞 Core 0 的 Timer 中断
    // Core 0: Timer 中断、PD 控制、转弯检测、ADC 采样（实时任务）
    // Core 1: display_task、语音模块（非实时任务）
    BaseType_t ret = xTaskCreatePinnedToCore(
        display_task,
        "display_task",
        DISPLAY_TASK_STACK_SIZE,
        NULL,
        DISPLAY_TASK_PRIORITY,
        NULL,
        1  // 固定到 Core 1
    );
    
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "创建数码管显示任务失败");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "数码管显示任务创建成功（优先级：%d，Core 1）", DISPLAY_TASK_PRIORITY);
    return ESP_OK;
}
