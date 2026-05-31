/**
 * @file uart.h
 * @brief UART统一驱动模块头文件
 * 
 * 提供UART0/UART1和RS485的完整功能接口
 * 所有引脚定义来自 pin_definitions.h
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stddef.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

// ========================================
// UART 初始化
// ========================================

/**
 * @brief 初始化UART0
 * 
 * 配置UART0的波特率、引脚、缓冲区，并创建事件队列
 * 注意：UART0也用于RS485通信
 * 
 * @param baud_rate 波特率（例如：115200, 1000000）
 */
void uart0_init(uint32_t baud_rate);

/**
 * @brief 初始化UART1
 * 
 * 配置UART1的波特率、引脚、缓冲区
 * 用于外接串口模块（语音、蓝牙等）
 * 
 * @param baud_rate 波特率（例如：115200, 9600）
 */
void uart1_init(uint32_t baud_rate);

// ========================================
// UART 发送
// ========================================

/**
 * @brief 通过UART0发送数据
 * 
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际发送的字节数
 */
int uart0_send(const uint8_t *data, uint16_t len);

/**
 * @brief 通过UART1发送数据
 * 
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际发送的字节数
 */
int uart1_send(const uint8_t *data, uint16_t len);

// ========================================
// UART 接收
// ========================================

/**
 * @brief 从UART0接收数据
 * 
 * @param data 接收缓冲区指针
 * @param len 最大接收长度
 * @param timeout 超时时间（FreeRTOS ticks）
 * @return 实际接收的字节数
 */
int uart0_recv(uint8_t *data, size_t len, TickType_t timeout);

/**
 * @brief 从UART1接收数据
 * 
 * @param data 接收缓冲区指针
 * @param len 最大接收长度
 * @param timeout 超时时间（FreeRTOS ticks）
 * @return 实际接收的字节数
 */
int uart1_recv(uint8_t *data, size_t len, TickType_t timeout);

// ========================================
// UART 反初始化
// ========================================

/**
 * @brief 反初始化UART0
 * 
 * 释放UART0资源
 */
void uart0_deinit(void);

/**
 * @brief 反初始化UART1
 * 
 * 释放UART1资源
 */
void uart1_deinit(void);

// ========================================
// RS485 控制
// ========================================

/**
 * @brief 初始化RS485方向控制GPIO
 * 
 * 配置RS485方向控制引脚，默认设置为接收模式
 */
void rs485_init(void);

/**
 * @brief 设置RS485为发送模式
 */
void rs485_set_tx_mode(void);

/**
 * @brief 设置RS485为接收模式
 */
void rs485_set_rx_mode(void);

/**
 * @brief RS485发送数据（自动控制方向）
 * 
 * 自动切换到发送模式，发送数据后切换回接收模式
 * 
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际发送的字节数
 */
int rs485_send(const uint8_t *data, uint16_t len);

/**
 * @brief RS485发送数据（带互斥锁保护）
 * 
 * 用于多任务环境，防止摄像头和舵机同时访问RS485总线
 * 推荐在多任务环境中使用此函数
 * 
 * @param data 数据指针
 * @param len 数据长度
 * @param timeout_ms 获取互斥锁的超时时间（毫秒）
 * @return 实际发送的字节数，-1表示获取互斥锁失败
 */
int rs485_send_protected(const uint8_t *data, uint16_t len, uint32_t timeout_ms);

// ========================================
// RS485 互斥锁管理
// ========================================

/**
 * @brief 初始化UART0 RS485互斥锁
 * 
 * 创建用于保护RS485总线访问的互斥锁
 * 必须在使用 rs485_send_protected() 之前调用
 * 
 * @return 0=成功，-1=失败
 */
int uart0_rs485_mutex_init(void);

/**
 * @brief 获取UART0 RS485互斥锁句柄
 * 
 * @return 互斥锁句柄，如果未初始化则返回NULL
 */
SemaphoreHandle_t uart0_get_rs485_mutex(void);

// ========================================
// 事件队列
// ========================================

/**
 * @brief 获取UART0事件队列句柄
 * 
 * 用于中断接收模式
 * 
 * @return 事件队列句柄，如果未初始化则返回NULL
 */
QueueHandle_t uart0_get_event_queue(void);

#ifdef __cplusplus
}
#endif

#endif // UART_H
