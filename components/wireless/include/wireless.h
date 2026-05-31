/**
 * @file wireless.h
 * @brief ESP32无线通信模块驱动（基于AT指令）
 * 
 * 通过UART1控制外部ESP32-AT模块，提供WiFi、TCP、BLE等功能
 * 适用于为主控ESP32扩展无线通信能力
 */

#ifndef WIRELESS_H
#define WIRELESS_H

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// ========================================
// 配置参数
// ========================================

#define WIRELESS_RX_BUF_SIZE 512        // 接收缓冲区大小
#define AT_TIMEOUT_MS 5000              // AT指令默认超时时间（毫秒）

// ========================================
// 枚举定义
// ========================================

/**
 * @brief AT指令响应状态
 */
typedef enum {
    AT_RESP_OK = 0,      // 收到OK响应
    AT_RESP_ERROR,       // 收到ERROR响应
    AT_RESP_TIMEOUT,     // 超时
    AT_RESP_WAITING      // 等待响应中
} at_response_t;

/**
 * @brief WiFi连接状态
 */
typedef enum {
    WIFI_DISCONNECTED = 0,
    WIFI_CONNECTING,
    WIFI_CONNECTED
} wifi_status_t;

// ========================================
// 基础函数
// ========================================

/**
 * @brief 初始化无线模块
 * 
 * 配置UART1（GPIO35/36，115200波特率），创建接收任务
 * 必须在使用其他函数前调用
 */
void wireless_init(void);

/**
 * @brief 发送字符串到无线模块
 * 
 * @param str 要发送的字符串
 */
void wireless_send_string(const char *str);

/**
 * @brief 发送AT指令
 * 
 * 自动添加\r\n结尾
 * 
 * @param cmd AT指令字符串（不含\r\n）
 */
void wireless_send_at_command(const char *cmd);

/**
 * @brief 等待AT指令响应
 * 
 * @param timeout_ms 超时时间（毫秒）
 * @return AT响应状态
 */
at_response_t wireless_wait_response(uint32_t timeout_ms);

/**
 * @brief 获取接收到的数据
 * 
 * @param buf 数据缓冲区
 * @param max_len 缓冲区最大长度
 * @return 实际读取的数据长度
 */
uint16_t wireless_get_received_data(uint8_t *buf, uint16_t max_len);

// ========================================
// AT指令封装
// ========================================

/**
 * @brief 测试AT指令
 * 
 * 发送AT指令测试模块响应
 * 
 * @return 1=成功，0=失败
 */
uint8_t wireless_test(void);

/**
 * @brief 复位模块
 * 
 * 发送AT+RST指令复位模块
 * 
 * @return 1=成功，0=失败
 */
uint8_t wireless_reset(void);

// ========================================
// WiFi功能
// ========================================

/**
 * @brief 设置WiFi模式
 * 
 * @param mode 1=Station, 2=AP, 3=混合模式
 * @return 1=成功，0=失败
 */
uint8_t wireless_set_wifi_mode(uint8_t mode);

/**
 * @brief 连接WiFi
 * 
 * @param ssid WiFi名称
 * @param password WiFi密码
 * @return 1=成功，0=失败
 */
uint8_t wireless_connect_wifi(const char *ssid, const char *password);

/**
 * @brief 断开WiFi连接
 * 
 * @return 1=成功，0=失败
 */
uint8_t wireless_disconnect_wifi(void);

/**
 * @brief 查询IP地址
 * 
 * @return 1=成功，0=失败
 */
uint8_t wireless_query_ip(void);

/**
 * @brief Ping测试
 * 
 * @param host 目标主机（IP或域名）
 * @return 1=成功，0=失败
 */
uint8_t wireless_ping(const char *host);

// ========================================
// TCP功能
// ========================================

/**
 * @brief 设置单连接模式
 * 
 * @return 1=成功，0=失败
 */
uint8_t wireless_set_single_connection(void);

/**
 * @brief 建立TCP连接
 * 
 * @param host 服务器IP或域名
 * @param port 服务器端口
 * @return 1=成功，0=失败
 */
uint8_t wireless_connect_tcp(const char *host, uint16_t port);

/**
 * @brief 断开TCP连接
 * 
 * @return 1=成功，0=失败
 */
uint8_t wireless_disconnect_tcp(void);

/**
 * @brief 发送数据（普通模式）
 * 
 * @param data 要发送的数据
 * @param len 数据长度
 * @return 1=成功，0=失败
 */
uint8_t wireless_send_data(const uint8_t *data, uint16_t len);

/**
 * @brief 设置透传模式
 * 
 * @param enable 1=透传模式，0=普通模式
 * @return 1=成功，0=失败
 */
uint8_t wireless_set_transparent_mode(uint8_t enable);

/**
 * @brief 进入透传模式
 * 
 * @return 1=成功，0=失败
 */
uint8_t wireless_enter_transparent(void);

/**
 * @brief 退出透传模式
 * 
 * 发送+++退出透传（前后需要1秒间隔）
 */
void wireless_exit_transparent(void);

// ========================================
// 蓝牙基础功能
// ========================================

/**
 * @brief 初始化蓝牙
 * 
 * @param mode 0=关闭, 1=Client, 2=Server
 * @return 1=成功，0=失败
 */
uint8_t wireless_ble_init(uint8_t mode);

/**
 * @brief 设置蓝牙设备名称
 * 
 * @param name 设备名称（最大32字节）
 * @return 1=成功，0=失败
 */
uint8_t wireless_ble_set_name(const char *name);

/**
 * @brief 扫描蓝牙设备
 * 
 * @param enable 1=开始扫描，0=停止扫描
 * @param duration 扫描时长（秒），仅在enable=1时有效
 * @return 1=成功，0=失败
 */
uint8_t wireless_ble_scan(uint8_t enable, uint8_t duration);

/**
 * @brief 连接蓝牙设备
 * 
 * @param conn_index 连接索引（0-2）
 * @param mac 目标设备MAC地址
 * @return 1=成功，0=失败
 */
uint8_t wireless_ble_connect(uint8_t conn_index, const char *mac);

/**
 * @brief 断开蓝牙连接
 * 
 * @param conn_index 连接索引（0-2）
 * @return 1=成功，0=失败
 */
uint8_t wireless_ble_disconnect(uint8_t conn_index);

// ========================================
// 蓝牙SPP透传功能
// ========================================

/**
 * @brief 创建GATT服务（服务端）
 * 
 * @return 1=成功，0=失败
 */
uint8_t wireless_ble_create_gatt_service(void);

/**
 * @brief 启动GATT服务（服务端）
 * 
 * @return 1=成功，0=失败
 */
uint8_t wireless_ble_start_gatt_service(void);

/**
 * @brief 设置广播参数
 * 
 * @param adv_int_min 最小广播间隔（单位：0.625ms）
 * @param adv_int_max 最大广播间隔（单位：0.625ms）
 * @return 1=成功，0=失败
 */
uint8_t wireless_ble_set_adv_param(uint16_t adv_int_min, uint16_t adv_int_max);

/**
 * @brief 设置广播数据
 * 
 * @param adv_data 广播数据（十六进制字符串）
 * @return 1=成功，0=失败
 */
uint8_t wireless_ble_set_adv_data(const char *adv_data);

/**
 * @brief 开始广播
 * 
 * @return 1=成功，0=失败
 */
uint8_t wireless_ble_start_adv(void);

/**
 * @brief 停止广播
 * 
 * @return 1=成功，0=失败
 */
uint8_t wireless_ble_stop_adv(void);

/**
 * @brief 配置SPP参数
 * 
 * @param tx_srv 发送服务索引
 * @param tx_char 发送特征索引
 * @param rx_srv 接收服务索引
 * @param rx_char 接收特征索引
 * @param mtu MTU大小
 * @return 1=成功，0=失败
 */
uint8_t wireless_ble_config_spp(uint8_t tx_srv, uint8_t tx_char, 
                                 uint8_t rx_srv, uint8_t rx_char, uint8_t mtu);

/**
 * @brief 进入SPP透传模式
 * 
 * @return 1=成功，0=失败
 */
uint8_t wireless_ble_enter_spp(void);

/**
 * @brief 退出SPP透传模式
 * 
 * 发送+++退出透传（前后需要1秒间隔）
 */
void wireless_ble_exit_spp(void);

/**
 * @brief 保存透传配置到NVS（断电自动重连）
 * 
 * @param role 角色（1=客户端，2=服务端）
 * @param tx_srv 发送服务索引
 * @param tx_char 发送特征索引
 * @param rx_srv 接收服务索引
 * @param rx_char 接收特征索引
 * @param peer_addr 对端设备MAC地址
 * @return 1=成功，0=失败
 */
uint8_t wireless_ble_save_trans_link(uint8_t role, uint8_t tx_srv, uint8_t tx_char,
                                      uint8_t rx_srv, uint8_t rx_char, const char *peer_addr);

#ifdef __cplusplus
}
#endif

#endif // WIRELESS_H
