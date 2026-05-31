/**
 * @file wireless.c
 * @brief ESP32无线通信模块驱动实现
 * 
 * 通过UART1控制外部ESP32-AT模块
 */

#include "wireless.h"
#include "uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "WIRELESS";

// ========================================
// 内部变量
// ========================================

// 环形缓冲区
static uint8_t rx_buf[WIRELESS_RX_BUF_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

// 响应状态
static volatile at_response_t response_status = AT_RESP_WAITING;

// 接收任务句柄
static TaskHandle_t rx_task_handle = NULL;

// ========================================
// 内部函数声明
// ========================================

static void wireless_rx_task(void *pvParameters);
static void process_line(const char *line);

// ========================================
// 基础函数实现
// ========================================

/**
 * @brief 初始化无线模块
 */
void wireless_init(void)
{
    ESP_LOGI(TAG, "初始化无线模块...");
    
    // 初始化UART1，波特率115200
    uart1_init(115200);
    
    // 清空缓冲区
    rx_head = 0;
    rx_tail = 0;
    response_status = AT_RESP_WAITING;
    
    // 创建接收任务
    xTaskCreate(wireless_rx_task, "wireless_rx", 4096, NULL, 10, &rx_task_handle);
    
    ESP_LOGI(TAG, "无线模块初始化完成");
}

/**
 * @brief 发送字符串到无线模块
 */
void wireless_send_string(const char *str)
{
    uart1_send((uint8_t *)str, strlen(str));
}

/**
 * @brief 发送AT指令
 */
void wireless_send_at_command(const char *cmd)
{
    wireless_send_string(cmd);
    uart1_send((uint8_t *)"\r\n", 2);
    response_status = AT_RESP_WAITING;
    
    ESP_LOGI(TAG, ">> %s", cmd);
}

/**
 * @brief 等待AT指令响应
 */
at_response_t wireless_wait_response(uint32_t timeout_ms)
{
    uint32_t elapsed = 0;
    const uint32_t check_interval = 10;  // 每10ms检查一次
    
    while (elapsed < timeout_ms) {
        if (response_status == AT_RESP_OK || response_status == AT_RESP_ERROR) {
            return response_status;
        }
        
        vTaskDelay(pdMS_TO_TICKS(check_interval));
        elapsed += check_interval;
    }
    
    ESP_LOGW(TAG, "等待响应超时");
    return AT_RESP_TIMEOUT;
}

/**
 * @brief 获取接收到的数据
 */
uint16_t wireless_get_received_data(uint8_t *buf, uint16_t max_len)
{
    uint16_t len = 0;
    
    while (rx_head != rx_tail && len < max_len) {
        buf[len++] = rx_buf[rx_tail];
        rx_tail = (rx_tail + 1) % WIRELESS_RX_BUF_SIZE;
    }
    
    return len;
}

// ========================================
// 接收任务
// ========================================

/**
 * @brief 接收任务（使用UART事件队列）
 */
static void wireless_rx_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t *temp_buf = (uint8_t *)malloc(256);
    static char line_buf[128];
    static uint8_t line_idx = 0;
    
    if (temp_buf == NULL) {
        ESP_LOGE(TAG, "接收任务内存分配失败");
        vTaskDelete(NULL);
        return;
    }
    
    ESP_LOGI(TAG, "接收任务启动");
    
    while (1) {
        // 从UART1读取数据（阻塞等待）
        int len = uart1_recv(temp_buf, 256, portMAX_DELAY);
        
        if (len > 0) {
            // 将数据存入环形缓冲区
            for (int i = 0; i < len; i++) {
                uint8_t byte = temp_buf[i];
                
                // 存入环形缓冲区
                uint16_t next_head = (rx_head + 1) % WIRELESS_RX_BUF_SIZE;
                if (next_head != rx_tail) {
                    rx_buf[rx_head] = byte;
                    rx_head = next_head;
                }
                
                // 解析行数据
                if (byte == '\n' || byte == '\r') {
                    if (line_idx > 0) {
                        line_buf[line_idx] = '\0';
                        process_line(line_buf);
                        line_idx = 0;
                    }
                } else if (line_idx < sizeof(line_buf) - 1) {
                    line_buf[line_idx++] = byte;
                }
            }
        }
    }
    
    free(temp_buf);
    vTaskDelete(NULL);
}

/**
 * @brief 处理接收到的一行数据
 */
static void process_line(const char *line)
{
    // 过滤ESP32的日志信息（以I、W、E开头的调试信息）
    uint8_t is_log = 0;
    if (strlen(line) > 2 && 
        (line[0] == 'I' || line[0] == 'W' || line[0] == 'E') && 
        line[1] == ' ' && line[2] == '(') {
        is_log = 1;  // 这是日志信息，不打印
    }
    
    // 打印接收到的响应（过滤日志）
    if (!is_log && strlen(line) > 0) {
        ESP_LOGI(TAG, "<< %s", line);
    }
    
    // 解析响应
    if (strcmp(line, "OK") == 0) {
        response_status = AT_RESP_OK;
    } else if (strcmp(line, "ERROR") == 0 || strcmp(line, "FAIL") == 0) {
        response_status = AT_RESP_ERROR;
    } else if (strstr(line, "WIFI CONNECTED") != NULL) {
        response_status = AT_RESP_OK;
    } else if (strstr(line, "WIFI DISCONNECT") != NULL) {
        // WiFi断开
    } else if (strstr(line, "CONNECT") != NULL) {
        response_status = AT_RESP_OK;
    } else if (strstr(line, "CLOSED") != NULL) {
        // TCP连接关闭
    } else if (strstr(line, "+IPD") != NULL) {
        // 接收到数据：+IPD,<len>:<data>
    } else if (strstr(line, "SEND OK") != NULL) {
        response_status = AT_RESP_OK;
    } else if (strstr(line, ">") != NULL) {
        response_status = AT_RESP_OK;
    }
}

// ========================================
// AT指令封装
// ========================================

/**
 * @brief 测试AT指令
 */
uint8_t wireless_test(void)
{
    wireless_send_at_command("AT");
    return (wireless_wait_response(1000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 复位模块
 */
uint8_t wireless_reset(void)
{
    wireless_send_at_command("AT+RST");
    return (wireless_wait_response(5000) == AT_RESP_OK) ? 1 : 0;
}

// ========================================
// WiFi功能
// ========================================

/**
 * @brief 设置WiFi模式
 */
uint8_t wireless_set_wifi_mode(uint8_t mode)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+CWMODE=%d", mode);
    wireless_send_at_command(cmd);
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 连接WiFi
 */
uint8_t wireless_connect_wifi(const char *ssid, const char *password)
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    wireless_send_at_command(cmd);
    return (wireless_wait_response(20000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 断开WiFi连接
 */
uint8_t wireless_disconnect_wifi(void)
{
    wireless_send_at_command("AT+CWQAP");
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 查询IP地址
 */
uint8_t wireless_query_ip(void)
{
    wireless_send_at_command("AT+CIFSR");
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief Ping测试
 */
uint8_t wireless_ping(const char *host)
{
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "AT+PING=\"%s\"", host);
    wireless_send_at_command(cmd);
    return (wireless_wait_response(10000) == AT_RESP_OK) ? 1 : 0;
}

// ========================================
// TCP功能
// ========================================

/**
 * @brief 设置单连接模式
 */
uint8_t wireless_set_single_connection(void)
{
    wireless_send_at_command("AT+CIPMUX=0");
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 建立TCP连接
 */
uint8_t wireless_connect_tcp(const char *host, uint16_t port)
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%d", host, port);
    wireless_send_at_command(cmd);
    return (wireless_wait_response(10000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 断开TCP连接
 */
uint8_t wireless_disconnect_tcp(void)
{
    wireless_send_at_command("AT+CIPCLOSE");
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 发送数据（普通模式）
 */
uint8_t wireless_send_data(const uint8_t *data, uint16_t len)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d", len);
    wireless_send_at_command(cmd);
    
    // 等待'>'提示符
    if (wireless_wait_response(2000) != AT_RESP_OK) {
        return 0;
    }
    
    // 发送数据
    uart1_send(data, len);
    
    return (wireless_wait_response(5000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 设置透传模式
 */
uint8_t wireless_set_transparent_mode(uint8_t enable)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+CIPMODE=%d", enable);
    wireless_send_at_command(cmd);
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 进入透传模式
 */
uint8_t wireless_enter_transparent(void)
{
    wireless_send_at_command("AT+CIPSEND");
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 退出透传模式
 */
void wireless_exit_transparent(void)
{
    // 延时1秒
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 发送+++
    wireless_send_string("+++");
    
    // 延时1秒
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// ========================================
// 蓝牙基础功能
// ========================================

/**
 * @brief 初始化蓝牙
 */
uint8_t wireless_ble_init(uint8_t mode)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+BLEINIT=%d", mode);
    wireless_send_at_command(cmd);
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 设置蓝牙设备名称
 */
uint8_t wireless_ble_set_name(const char *name)
{
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "AT+BLENAME=\"%s\"", name);
    wireless_send_at_command(cmd);
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 扫描蓝牙设备
 */
uint8_t wireless_ble_scan(uint8_t enable, uint8_t duration)
{
    char cmd[32];
    if (enable) {
        snprintf(cmd, sizeof(cmd), "AT+BLESCAN=1,%d", duration);
    } else {
        snprintf(cmd, sizeof(cmd), "AT+BLESCAN=0");
    }
    wireless_send_at_command(cmd);
    return (wireless_wait_response(duration * 1000 + 2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 连接蓝牙设备
 */
uint8_t wireless_ble_connect(uint8_t conn_index, const char *mac)
{
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "AT+BLECONN=%d,\"%s\"", conn_index, mac);
    wireless_send_at_command(cmd);
    return (wireless_wait_response(30000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 断开蓝牙连接
 */
uint8_t wireless_ble_disconnect(uint8_t conn_index)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+BLEDISCONN=%d", conn_index);
    wireless_send_at_command(cmd);
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

// ========================================
// 蓝牙SPP透传功能
// ========================================

/**
 * @brief 创建GATT服务（服务端）
 */
uint8_t wireless_ble_create_gatt_service(void)
{
    wireless_send_at_command("AT+BLEGATTSSRVCRE");
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 启动GATT服务（服务端）
 */
uint8_t wireless_ble_start_gatt_service(void)
{
    wireless_send_at_command("AT+BLEGATTSSRVSTART");
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 设置广播参数
 */
uint8_t wireless_ble_set_adv_param(uint16_t adv_int_min, uint16_t adv_int_max)
{
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "AT+BLEADVPARAM=%d,%d,0,0,7,0,,", adv_int_min, adv_int_max);
    wireless_send_at_command(cmd);
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 设置广播数据
 */
uint8_t wireless_ble_set_adv_data(const char *adv_data)
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+BLEADVDATA=\"%s\"", adv_data);
    wireless_send_at_command(cmd);
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 开始广播
 */
uint8_t wireless_ble_start_adv(void)
{
    wireless_send_at_command("AT+BLEADVSTART");
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 停止广播
 */
uint8_t wireless_ble_stop_adv(void)
{
    wireless_send_at_command("AT+BLEADVSTOP");
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 配置SPP参数
 */
uint8_t wireless_ble_config_spp(uint8_t tx_srv, uint8_t tx_char, 
                                 uint8_t rx_srv, uint8_t rx_char, uint8_t mtu)
{
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "AT+BLESPPCFG=%d,%d,%d,%d,%d", 
             tx_srv, tx_char, rx_srv, rx_char, mtu);
    wireless_send_at_command(cmd);
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 进入SPP透传模式
 */
uint8_t wireless_ble_enter_spp(void)
{
    wireless_send_at_command("AT+BLESPP");
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}

/**
 * @brief 退出SPP透传模式
 */
void wireless_ble_exit_spp(void)
{
    wireless_exit_transparent();  // 复用TCP透传的退出函数
}

/**
 * @brief 保存透传配置到NVS（断电自动重连）
 */
uint8_t wireless_ble_save_trans_link(uint8_t role, uint8_t tx_srv, uint8_t tx_char,
                                      uint8_t rx_srv, uint8_t rx_char, const char *peer_addr)
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+SAVETRANSLINK=2,%d,%d,%d,%d,%d,\"%s\"", 
             role, tx_srv, tx_char, rx_srv, rx_char, peer_addr);
    wireless_send_at_command(cmd);
    return (wireless_wait_response(2000) == AT_RESP_OK) ? 1 : 0;
}
