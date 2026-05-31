/**
 * @file uart.c
 * @brief UART统一驱动模块（包含UART0/UART1和RS485支持）
 * 
 * 本模块提供完整的UART功能：
 * - UART0/UART1初始化、发送、接收
 * - RS485方向控制和互斥锁管理
 * - 事件队列支持（中断接收）
 */

#include "uart.h"
#include "pin_definitions.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static const char *TAG = "UART";

// UART0 RS485互斥锁（用于保护摄像头和舵机的RS485通信）
static SemaphoreHandle_t uart0_rs485_mutex = NULL;

// UART0 事件队列句柄（用于中断接收）
static QueueHandle_t uart0_event_queue = NULL;

// RS485初始化标志
static bool rs485_initialized = false;

// ========================================
// UART0 初始化
// ========================================

/**
 * @brief 初始化UART0（支持事件队列中断接收）
 * 
 * @param baud_rate 波特率
 */
void uart0_init(uint32_t baud_rate)
{
    // 配置UART参数
    const uart_config_t uart_config = {
        .baud_rate = (int)baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // 配置UART参数
    uart_param_config(UART0_PORT, &uart_config);
    
    // 设置引脚（从pin_definitions.h获取）
    uart_set_pin(UART0_PORT, UART0_TX_GPIO, UART0_RX_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    
    // 安装UART驱动并创建事件队列
    uart_driver_install(UART0_PORT, 
                        UART0_RX_BUFFER_SIZE,
                        UART0_TX_BUFFER_SIZE,
                        20,                      // 事件队列大小
                        &uart0_event_queue,
                        0);
    
    ESP_LOGI(TAG, "UART0初始化完成: 波特率=%lu, TX=GPIO%d, RX=GPIO%d", 
             baud_rate, UART0_TX_GPIO, UART0_RX_GPIO);
}

/**
 * @brief 初始化UART1
 * 
 * @param baud_rate 波特率
 */
void uart1_init(uint32_t baud_rate)
{
    // 配置UART参数
    const uart_config_t uart_config = {
        .baud_rate = (int)baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // 配置UART参数
    uart_param_config(UART1_PORT, &uart_config);
    
    // 设置引脚（从pin_definitions.h获取）
    uart_set_pin(UART1_PORT, UART1_TX_GPIO, UART1_RX_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    
    // 安装UART驱动
    uart_driver_install(UART1_PORT, UART1_RX_BUFFER_SIZE, UART1_TX_BUFFER_SIZE, 0, NULL, 0);
    
    ESP_LOGI(TAG, "UART1初始化完成: 波特率=%lu, TX=GPIO%d, RX=GPIO%d", 
             baud_rate, UART1_TX_GPIO, UART1_RX_GPIO);
}

// ========================================
// UART 发送
// ========================================

/**
 * @brief 通过UART0发送数据
 */
int uart0_send(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0) {
        return 0;
    }
    return uart_write_bytes(UART0_PORT, data, len);
}

/**
 * @brief 通过UART1发送数据
 */
int uart1_send(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0) {
        return 0;
    }
    return uart_write_bytes(UART1_PORT, data, len);
}

// ========================================
// UART 接收
// ========================================

/**
 * @brief 从UART0接收数据
 */
int uart0_recv(uint8_t *data, size_t len, TickType_t timeout)
{
    if (data == NULL || len == 0) {
        return 0;
    }
    
    int ret = uart_read_bytes(UART0_PORT, data, len, timeout);
    
    if (ret > 0) {
        ESP_LOGI(TAG, "UART0接收: %d字节", ret);
        ESP_LOG_BUFFER_HEXDUMP(TAG, data, ret, ESP_LOG_DEBUG);
    }
    
    return ret;
}

/**
 * @brief 从UART1接收数据
 */
int uart1_recv(uint8_t *data, size_t len, TickType_t timeout)
{
    if (data == NULL || len == 0) {
        return 0;
    }
    
    int ret = uart_read_bytes(UART1_PORT, data, len, timeout);
    
    if (ret > 0) {
        ESP_LOGI(TAG, "UART1接收: %d字节", ret);
        ESP_LOG_BUFFER_HEXDUMP(TAG, data, ret, ESP_LOG_DEBUG);
    }
    
    return ret;
}

// ========================================
// UART 反初始化
// ========================================

/**
 * @brief 反初始化UART0
 */
void uart0_deinit(void)
{
    uart_driver_delete(UART0_PORT);
    ESP_LOGI(TAG, "UART0已反初始化");
}

/**
 * @brief 反初始化UART1
 */
void uart1_deinit(void)
{
    uart_driver_delete(UART1_PORT);
    ESP_LOGI(TAG, "UART1已反初始化");
}

// ========================================
// RS485 控制
// ========================================

/**
 * @brief 初始化RS485方向控制GPIO
 */
void rs485_init(void)
{
    if (rs485_initialized) {
        ESP_LOGW(TAG, "RS485已经初始化");
        return;
    }
    
    // 配置RS485方向控制引脚
    gpio_config_t dir_cfg = {
        .pin_bit_mask = 1ULL << RS485_DIR_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&dir_cfg);
    
    // 默认设置为接收模式
    gpio_set_level(RS485_DIR_GPIO, RS485_DIR_RX_LEVEL);
    
    rs485_initialized = true;
    ESP_LOGI(TAG, "RS485初始化完成: DIR=GPIO%d, TX电平=%d, RX电平=%d", 
             RS485_DIR_GPIO, RS485_DIR_TX_LEVEL, RS485_DIR_RX_LEVEL);
}

/**
 * @brief 设置RS485为发送模式
 */
void rs485_set_tx_mode(void)
{
    gpio_set_level(RS485_DIR_GPIO, RS485_DIR_TX_LEVEL);
}

/**
 * @brief 设置RS485为接收模式
 */
void rs485_set_rx_mode(void)
{
    gpio_set_level(RS485_DIR_GPIO, RS485_DIR_RX_LEVEL);
}

/**
 * @brief RS485发送数据（自动控制方向）
 * 
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际发送的字节数
 */
int rs485_send(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0) {
        return 0;
    }
    
    // 切换到发送模式
    rs485_set_tx_mode();
    
    // 发送数据
    int ret = uart_write_bytes(UART0_PORT, data, len);
    
    // 等待发送完成
    uart_wait_tx_done(UART0_PORT, pdMS_TO_TICKS(100));
    
    // 短暂延时确保数据发送完毕
    vTaskDelay(pdMS_TO_TICKS(2));
    
    // 切换回接收模式
    rs485_set_rx_mode();
    
    return ret;
}

/**
 * @brief RS485发送数据（带互斥锁保护）
 * 
 * 用于多任务环境，防止摄像头和舵机同时访问RS485总线
 * 
 * @param data 数据指针
 * @param len 数据长度
 * @param timeout_ms 获取互斥锁的超时时间（毫秒）
 * @return 实际发送的字节数，-1表示获取互斥锁失败
 */
int rs485_send_protected(const uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
    if (uart0_rs485_mutex == NULL) {
        ESP_LOGE(TAG, "RS485互斥锁未初始化");
        return -1;
    }
    
    // 尝试获取互斥锁
    if (xSemaphoreTake(uart0_rs485_mutex, pdMS_TO_TICKS(timeout_ms)) != pdTRUE) {
        ESP_LOGW(TAG, "获取RS485互斥锁超时");
        return -1;
    }
    
    // 发送数据
    int ret = rs485_send(data, len);
    
    // 释放互斥锁
    xSemaphoreGive(uart0_rs485_mutex);
    
    return ret;
}

// ========================================
// RS485 互斥锁管理
// ========================================

/**
 * @brief 初始化UART0 RS485互斥锁
 */
int uart0_rs485_mutex_init(void)
{
    if (uart0_rs485_mutex != NULL) {
        ESP_LOGW(TAG, "UART0 RS485互斥锁已经初始化");
        return 0;
    }
    
    uart0_rs485_mutex = xSemaphoreCreateMutex();
    if (uart0_rs485_mutex == NULL) {
        ESP_LOGE(TAG, "创建UART0 RS485互斥锁失败");
        return -1;
    }
    
    ESP_LOGI(TAG, "UART0 RS485互斥锁初始化成功");
    return 0;
}

/**
 * @brief 获取UART0 RS485互斥锁句柄
 */
SemaphoreHandle_t uart0_get_rs485_mutex(void)
{
    return uart0_rs485_mutex;
}

// ========================================
// 事件队列
// ========================================

/**
 * @brief 获取UART0事件队列句柄
 */
QueueHandle_t uart0_get_event_queue(void)
{
    return uart0_event_queue;
}
