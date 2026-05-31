/**
 * @file five_way_gray_i2c.c
 * @brief 五路灰度传感器I2C驱动实现
 * 
 * 功能：
 * - 通过I2C读取5路灰度传感器数据
 * - 后台任务2ms周期自动更新
 * - 原子变量保护共享数据
 * 
 * 通信：
 * - I2C地址：0x4F
 * - I2C频率：400kHz
 * - 数据格式：5字节（每路1字节）
 * 
 * 注意：
 * - 与二路ADC灰度传感器互斥使用（共用GPIO18/20）
 * - 使用独立I2C总线（I2C_NUM_1）
 */

#include "include/five_way_gray_i2c.h"
#include "board_config.h"
#include "timer_system.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdatomic.h>
#include <string.h>

static const char *TAG = "FIVE_GRAY_I2C";

// I2C总线和设备句柄
static i2c_master_bus_handle_t i2c_bus_handle = NULL;
static i2c_master_dev_handle_t i2c_dev_handle = NULL;

// 使用原子变量保护共享数据（C11标准）
static _Atomic uint16_t g_gray_values[FIVE_WAY_GRAY_SENSOR_COUNT] = {0};
static _Atomic uint32_t g_read_count = 0;
static _Atomic uint32_t g_error_count = 0;
static _Atomic bool g_data_ready = false;

// 后台扫描任务句柄
static TaskHandle_t scan_task_handle = NULL;

// 校准数据（每个传感器独立校准）
static five_way_gray_calibration_t calibration_data[FIVE_WAY_GRAY_SENSOR_COUNT] = {
    {.white_value = 1000, .black_value = 512, .is_calibrated = false},  // 传感器0默认值
    {.white_value = 1000, .black_value = 512, .is_calibrated = false},  // 传感器1默认值
    {.white_value = 1000, .black_value = 512, .is_calibrated = false},  // 传感器2默认值
    {.white_value = 1000, .black_value = 512, .is_calibrated = false},  // 传感器3默认值
    {.white_value = 1000, .black_value = 512, .is_calibrated = false}   // 传感器4默认值
};

/**
 * @brief 安全模式函数声明（在timer_system中实现）
 */
extern void enter_safe_mode(void);

/**
 * @brief 从传感器读取原始数据
 * 
 * 通过I2C读取10字节数据（5个传感器 × 2字节）
 * 数据格式：小端序，每个传感器2字节（低字节在前）
 * 
 * @param values 5个uint16_t的数组，用于存储读取的数据
 * @return 
 *     - ESP_OK 读取成功
 *     - 其他值 读取失败
 */
static esp_err_t five_way_gray_i2c_read_raw(uint16_t values[FIVE_WAY_GRAY_SENSOR_COUNT])
{
    if (i2c_dev_handle == NULL) {
        ESP_LOGE(TAG, "I2C设备未初始化");
        return ESP_FAIL;
    }

    // 读取10字节原始数据
    uint8_t raw_data[FIVE_WAY_GRAY_SENSOR_COUNT * 2];
    esp_err_t ret = i2c_master_receive(i2c_dev_handle, raw_data, sizeof(raw_data), -1);
    
    if (ret != ESP_OK) {
        return ret;
    }

    // 解析数据：小端序，低字节在前
    for (int i = 0; i < FIVE_WAY_GRAY_SENSOR_COUNT; i++) {
        values[i] = raw_data[i * 2] + (raw_data[i * 2 + 1] * 256);
    }

    return ESP_OK;
}

/**
 * @brief 后台扫描任务
 * 
 * 以2ms周期运行，读取传感器数据并更新共享变量
 * 实现错误处理：读取失败时保持上次值，连续失败100次进入安全模式
 * 
 * @param pvParameters 任务参数（未使用）
 */
static void five_way_gray_scan_task(void *pvParameters)
{
    uint16_t values[FIVE_WAY_GRAY_SENSOR_COUNT];
    uint32_t consecutive_errors = 0;

    ESP_LOGI(TAG, "五路灰度扫描任务已启动，周期: 2ms");

    while (1) {
        // 尝试读取传感器数据
        esp_err_t ret = five_way_gray_i2c_read_raw(values);

        if (ret == ESP_OK) {
            // 读取成功，更新缓存
            for (int i = 0; i < FIVE_WAY_GRAY_SENSOR_COUNT; i++) {
                atomic_store(&g_gray_values[i], values[i]);
            }
            
            // 标记数据就绪
            atomic_store(&g_data_ready, true);
            
            // 增加读取计数
            atomic_fetch_add(&g_read_count, 1);
            
            // 重置连续错误计数
            if (consecutive_errors > 0) {
                consecutive_errors = 0;
            }
        } else {
            // 读取失败，保持上次有效值（缓存值不变）
            consecutive_errors++;
            atomic_fetch_add(&g_error_count, 1);

            ESP_LOGW(TAG, "I2C读取失败 (连续错误: %lu): %s",
                     consecutive_errors, esp_err_to_name(ret));

            // 连续失败超过100次，进入安全模式
            if (consecutive_errors >= 100) {
                ESP_LOGE(TAG, "I2C连续失败超过100次，进入安全模式");
                enter_safe_mode();
                // enter_safe_mode()会进入死循环，不会返回
            }
        }

        // 2ms周期延时
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

/**
 * @brief 初始化五路灰度传感器
 */
esp_err_t five_way_gray_i2c_init(void)
{
    ESP_LOGI(TAG, "初始化五路灰度传感器（I2C模式）...");

    // 1. 注册GPIO到gpio_manager（检测冲突）
    esp_err_t ret = gpio_manager_register(FIVE_WAY_GRAY_I2C_SCL_GPIO, GPIO_FUNC_I2C_SCL,
                                          "five_way_gray_i2c", "五路灰度传感器SCL");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO%d注册失败，可能与其他模块冲突", FIVE_WAY_GRAY_I2C_SCL_GPIO);
        return ret;
    }

    ret = gpio_manager_register(FIVE_WAY_GRAY_I2C_SDA_GPIO, GPIO_FUNC_I2C_SDA,
                                 "five_way_gray_i2c", "五路灰度传感器SDA");
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO%d注册失败，可能与其他模块冲突", FIVE_WAY_GRAY_I2C_SDA_GPIO);
        return ret;
    }

    // 2. 配置I2C主总线参数
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = FIVE_WAY_GRAY_I2C_PORT,
        .scl_io_num = FIVE_WAY_GRAY_I2C_SCL_GPIO,
        .sda_io_num = FIVE_WAY_GRAY_I2C_SDA_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,  // 启用内部上拉电阻
    };

    // 3. 创建I2C主总线实例
    ret = i2c_new_master_bus(&bus_config, &i2c_bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C总线初始化失败: %s", esp_err_to_name(ret));
        return ret;
    }

    // 4. 配置传感器设备参数
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = FIVE_WAY_GRAY_I2C_ADDR,
        .scl_speed_hz = FIVE_WAY_GRAY_I2C_FREQ_HZ,
    };

    // 5. 将传感器添加到I2C总线上
    ret = i2c_master_bus_add_device(i2c_bus_handle, &dev_config, &i2c_dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "添加I2C设备失败: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "I2C总线初始化成功: SCL=GPIO%d, SDA=GPIO%d, 地址=0x%02X, 频率=%dkHz",
             FIVE_WAY_GRAY_I2C_SCL_GPIO, FIVE_WAY_GRAY_I2C_SDA_GPIO,
             FIVE_WAY_GRAY_I2C_ADDR, FIVE_WAY_GRAY_I2C_FREQ_HZ / 1000);

    // 6. 创建后台扫描任务
    BaseType_t task_ret = xTaskCreate(
        five_way_gray_scan_task,    // 任务函数
        "five_gray_scan",           // 任务名称
        2048,                       // 栈大小（字节）
        NULL,                       // 任务参数
        6,                          // 优先级（6，与二路灰度扫描任务相同）
        &scan_task_handle           // 任务句柄
    );

    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "创建扫描任务失败");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "五路灰度传感器初始化完成，扫描任务已启动（优先级: 6，周期: 2ms）");
    
    return ESP_OK;
}

/**
 * @brief 获取缓存的传感器值（中断安全）
 */
void five_way_gray_i2c_get_values(uint16_t values[FIVE_WAY_GRAY_SENSOR_COUNT])
{
    if (values == NULL) {
        return;
    }

    // 使用原子读取操作，中断安全
    for (int i = 0; i < FIVE_WAY_GRAY_SENSOR_COUNT; i++) {
        values[i] = atomic_load(&g_gray_values[i]);
    }
}

/**
 * @brief 获取单个传感器的值
 */
uint16_t five_way_gray_i2c_get_value(uint8_t index)
{
    if (index >= FIVE_WAY_GRAY_SENSOR_COUNT) {
        ESP_LOGE(TAG, "无效的传感器索引: %d", index);
        return 0;
    }

    return atomic_load(&g_gray_values[index]);
}

/**
 * @brief 检测是否有任意传感器检测到黑线
 */
bool five_way_gray_i2c_any_black_detected(uint16_t threshold)
{
    // 使用默认阈值
    if (threshold == 0) {
        threshold = FIVE_WAY_GRAY_BLACK_THRESHOLD;
    }

    // 检查所有传感器
    for (int i = 0; i < FIVE_WAY_GRAY_SENSOR_COUNT; i++) {
        uint16_t value = atomic_load(&g_gray_values[i]);
        if (value < threshold) {
            return true;  // 检测到黑线
        }
    }

    return false;  // 所有传感器都在白色区域
}

/**
 * @brief 获取统计信息
 */
void five_way_gray_i2c_get_stats(uint32_t *read_count, uint32_t *error_count)
{
    if (read_count != NULL) {
        *read_count = atomic_load(&g_read_count);
    }

    if (error_count != NULL) {
        *error_count = atomic_load(&g_error_count);
    }
}

/**
 * @brief 打印传感器状态（调试用）
 */
void five_way_gray_i2c_print_status(void)
{
    uint16_t values[FIVE_WAY_GRAY_SENSOR_COUNT];
    five_way_gray_i2c_get_values(values);

    bool data_ready = atomic_load(&g_data_ready);
    uint32_t read_count = atomic_load(&g_read_count);
    uint32_t error_count = atomic_load(&g_error_count);

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "五路灰度传感器状态");
    ESP_LOGI(TAG, "========================================");
    
    if (!data_ready) {
        ESP_LOGW(TAG, "数据未就绪");
        return;
    }

    // 打印传感器值
    ESP_LOGI(TAG, "传感器值: [%4d, %4d, %4d, %4d, %4d]",
             values[0], values[1], values[2], values[3], values[4]);

    // 打印黑线检测状态
    ESP_LOGI(TAG, "黑线检测: [%s, %s, %s, %s, %s] (阈值=%d)",
             values[0] < FIVE_WAY_GRAY_BLACK_THRESHOLD ? "黑" : "白",
             values[1] < FIVE_WAY_GRAY_BLACK_THRESHOLD ? "黑" : "白",
             values[2] < FIVE_WAY_GRAY_BLACK_THRESHOLD ? "黑" : "白",
             values[3] < FIVE_WAY_GRAY_BLACK_THRESHOLD ? "黑" : "白",
             values[4] < FIVE_WAY_GRAY_BLACK_THRESHOLD ? "黑" : "白",
             FIVE_WAY_GRAY_BLACK_THRESHOLD);

    // 打印统计信息
    ESP_LOGI(TAG, "统计信息: 读取次数=%lu, 错误次数=%lu, 成功率=%.2f%%",
             read_count, error_count,
             read_count > 0 ? (100.0 * (read_count - error_count) / read_count) : 0.0);
    
    ESP_LOGI(TAG, "========================================");
}

/**
 * @brief 传感器校准 - 交互式校准
 */
void five_way_gray_i2c_calibrate(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "开始五路灰度传感器校准...");
    ESP_LOGI(TAG, "========================================");

    uint16_t white_values[FIVE_WAY_GRAY_SENSOR_COUNT] = {0};
    uint16_t black_values[FIVE_WAY_GRAY_SENSOR_COUNT];
    
    // 初始化黑线值为最大值（10位ADC）
    for (int i = 0; i < FIVE_WAY_GRAY_SENSOR_COUNT; i++) {
        black_values[i] = 1023;
    }

    // 校准白色区域
    ESP_LOGI(TAG, "请将传感器放置在白色区域上");
    ESP_LOGI(TAG, "5秒后开始采集白色数据...");
    vTaskDelay(pdMS_TO_TICKS(5000));

    ESP_LOGI(TAG, "正在采集白色区域数据...");
    for (int i = 0; i < 100; i++) {
        uint16_t values[FIVE_WAY_GRAY_SENSOR_COUNT];
        five_way_gray_i2c_get_values(values);
        
        // 记录每个传感器的最大值
        for (int j = 0; j < FIVE_WAY_GRAY_SENSOR_COUNT; j++) {
            if (values[j] > white_values[j]) {
                white_values[j] = values[j];
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGI(TAG, "白色区域采集完成: [%d, %d, %d, %d, %d]",
             white_values[0], white_values[1], white_values[2], 
             white_values[3], white_values[4]);

    // 校准黑线区域
    ESP_LOGI(TAG, "请将传感器放置在黑线上");
    ESP_LOGI(TAG, "5秒后开始采集黑线数据...");
    vTaskDelay(pdMS_TO_TICKS(5000));

    ESP_LOGI(TAG, "正在采集黑线数据...");
    for (int i = 0; i < 100; i++) {
        uint16_t values[FIVE_WAY_GRAY_SENSOR_COUNT];
        five_way_gray_i2c_get_values(values);
        
        // 记录每个传感器的最小值
        for (int j = 0; j < FIVE_WAY_GRAY_SENSOR_COUNT; j++) {
            if (values[j] < black_values[j]) {
                black_values[j] = values[j];
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGI(TAG, "黑线区域采集完成: [%d, %d, %d, %d, %d]",
             black_values[0], black_values[1], black_values[2], 
             black_values[3], black_values[4]);

    // 保存校准数据
    for (int i = 0; i < FIVE_WAY_GRAY_SENSOR_COUNT; i++) {
        calibration_data[i].white_value = white_values[i];
        calibration_data[i].black_value = black_values[i];
        calibration_data[i].is_calibrated = true;
    }

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "校准完成！");
    ESP_LOGI(TAG, "传感器0: 白色=%d, 黑线=%d", white_values[0], black_values[0]);
    ESP_LOGI(TAG, "传感器1: 白色=%d, 黑线=%d", white_values[1], black_values[1]);
    ESP_LOGI(TAG, "传感器2: 白色=%d, 黑线=%d", white_values[2], black_values[2]);
    ESP_LOGI(TAG, "传感器3: 白色=%d, 黑线=%d", white_values[3], black_values[3]);
    ESP_LOGI(TAG, "传感器4: 白色=%d, 黑线=%d", white_values[4], black_values[4]);
    ESP_LOGI(TAG, "========================================");
}

/**
 * @brief 手动设置校准参数
 */
void five_way_gray_i2c_set_calibration(uint8_t index, uint16_t white_value, uint16_t black_value)
{
    if (index >= FIVE_WAY_GRAY_SENSOR_COUNT) {
        ESP_LOGE(TAG, "无效的传感器索引: %d", index);
        return;
    }

    calibration_data[index].white_value = white_value;
    calibration_data[index].black_value = black_value;
    calibration_data[index].is_calibrated = true;

    ESP_LOGI(TAG, "传感器%d校准参数已设置: 白色=%d, 黑线=%d",
             index, white_value, black_value);
}

/**
 * @brief 获取校准参数
 */
const five_way_gray_calibration_t* five_way_gray_i2c_get_calibration(uint8_t index)
{
    if (index >= FIVE_WAY_GRAY_SENSOR_COUNT) {
        return NULL;
    }
    return &calibration_data[index];
}

/**
 * @brief 读取归一化后的传感器值
 */
float five_way_gray_i2c_read_normalized(uint8_t index)
{
    if (index >= FIVE_WAY_GRAY_SENSOR_COUNT) {
        ESP_LOGE(TAG, "无效的传感器索引: %d", index);
        return -1.0f;
    }

    // 检查是否已校准
    if (!calibration_data[index].is_calibrated) {
        ESP_LOGW(TAG, "传感器%d未校准，使用默认参数", index);
    }

    // 读取原始值
    uint16_t raw = five_way_gray_i2c_get_value(index);

    // 归一化处理 (0=黑线, 1=白色)
    uint16_t white = calibration_data[index].white_value;
    uint16_t black = calibration_data[index].black_value;
    
    if (white <= black) {
        ESP_LOGW(TAG, "传感器%d校准数据异常: 白色=%d, 黑线=%d", index, white, black);
        return -1.0f;
    }

    float normalized = (float)(raw - black) / (float)(white - black);

    // 限制在0-1范围内
    if (normalized < 0.0f) normalized = 0.0f;
    if (normalized > 1.0f) normalized = 1.0f;

    return normalized;
}

/**
 * @brief 读取所有传感器的归一化值
 */
void five_way_gray_i2c_read_all_normalized(float normalized[FIVE_WAY_GRAY_SENSOR_COUNT])
{
    if (normalized == NULL) {
        return;
    }

    for (int i = 0; i < FIVE_WAY_GRAY_SENSOR_COUNT; i++) {
        normalized[i] = five_way_gray_i2c_read_normalized(i);
    }
}
