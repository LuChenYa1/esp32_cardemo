/**
 * @file voice_module.h
 * @brief 语音控制模块头文件
 * 
 * 功能：
 * - 通过UART1接收语音模块命令（波特率115200）
 * - 解析串口数据为命令帧（协议：0xAA 0x55 [CMD] 0x55 0xAA）
 * - 处理摄像头控制命令（识别模式、亮度调节）
 * - 处理运行模式命令（巡线/命令模式）
 * - 处理速度设置命令
 * - 非阻塞式串口数据接收
 * - 命令处理时间不超过10毫秒
 */

#ifndef VOICE_MODULE_H
#define VOICE_MODULE_H

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "pin_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== 配置参数 ==================== */

// 任务优先级（低于Timer中断）
#define VOICE_TASK_PRIORITY       4

// 任务栈大小（增加到8KB以防止栈溢出）
#define VOICE_TASK_STACK_SIZE     8192

// UART配置（使用UART1，波特率115200）
// UART引脚定义在 pin_definitions.h 中
#define VOICE_UART_PORT           UART_NUM_1
#define VOICE_UART_BAUD_RATE      115200
// #define UART1_TX_GPIO        GPIO_NUM_35
// #define UART1_RX_GPIO        GPIO_NUM_36
#define VOICE_UART_RX_BUFFER_SIZE 2056

// 命令帧配置（协议格式：0xAA 0x55 [CMD] 0x55 0xAA）
#define VOICE_CMD_FRAME_HEADER1   0xAA    // 帧头第一字节
#define VOICE_CMD_FRAME_HEADER2   0x55    // 帧头第二字节
#define VOICE_CMD_FRAME_TAIL1     0x55    // 帧尾第一字节
#define VOICE_CMD_FRAME_TAIL2     0xAA    // 帧尾第二字节
#define VOICE_CMD_FRAME_LEN       5       // 帧长度（0xAA 0x55 [CMD] 0x55 0xAA）

// 命令定义（摄像头控制）
#define VOICE_CMD_COLOR           0x01    // 颜色识别模式
#define VOICE_CMD_BLOCK           0x02    // 色块识别模式
#define VOICE_CMD_FACE            0x03    // 人脸识别模式
#define VOICE_CMD_QRCODE          0x04    // 二维码识别模式
#define VOICE_CMD_NUMBER          0x05    // 数字识别模式
#define VOICE_CMD_LABEL           0x06    // 标签识别模式
#define VOICE_CMD_20CLASS         0x07    // 20类物体识别模式
#define VOICE_CMD_CAPTURE         0x09    // 拍照模式
#define VOICE_CMD_RESET           0x0B    // 摄像头复位
#define VOICE_CMD_BRIGHTNESS_UP   0x0C    // 增加亮度
#define VOICE_CMD_BRIGHTNESS_DOWN 0x0D    // 降低亮度
#define VOICE_CMD_SPEED_HIGH      0x0E    // 速度设置为2（高速）
#define VOICE_CMD_SPEED_LOW       0x0F    // 速度设置为1（低速）
#define VOICE_CMD_RUN_MODE        0x11    // 巡线模式 - 启动电机
#define VOICE_CMD_CMD_MODE        0x12    // 命令模式 - 停止电机

/* ==================== 命令帧结构 ==================== */

/**
 * @brief 语音命令帧结构
 * 
 * 帧格式：[0xAA] [0x55] [CMD] [0x55] [0xAA]
 */
typedef struct {
    uint8_t header1;     // 帧头第一字节 0xAA
    uint8_t header2;     // 帧头第二字节 0x55
    uint8_t command;     // 命令字节
    uint8_t tail1;       // 帧尾第一字节 0x55
    uint8_t tail2;       // 帧尾第二字节 0xAA
} VoiceCommandFrame_t;

/* ==================== 全局标志变量 ==================== */

/**
 * @brief 摄像头控制标志（外部可访问，线程安全）
 * 
 * 这些标志使用原子变量（_Atomic）实现，可以在多任务环境中安全访问
 * 建议使用atomic_load()和atomic_store()进行访问
 */
extern _Atomic uint8_t Flag_Color;
extern _Atomic uint8_t Flag_Block;
extern _Atomic uint8_t Flag_Face;
extern _Atomic uint8_t Flag_QRCODE;
extern _Atomic uint8_t Flag_NUMBER;
extern _Atomic uint8_t Flag_LABEL;
extern _Atomic uint8_t Flag_20CLASS;
extern _Atomic uint8_t Flag_Speed;
extern _Atomic uint8_t Flag_RunMode;

/* ==================== 对外接口 ==================== */

/**
 * @brief 初始化语音模块
 * 
 * 配置UART用于语音模块通信，初始化运行模式标志
 * 
 * @return ESP_OK 成功，ESP_FAIL 失败
 */
esp_err_t voice_module_init(void);

/**
 * @brief 创建语音模块UART接收任务
 * 
 * 创建FreeRTOS任务，实现非阻塞式串口数据接收和命令解析
 * 
 * @return ESP_OK 成功，ESP_FAIL 失败
 */
esp_err_t voice_module_task_create(void);

/**
 * @brief 删除语音模块任务
 * 
 * 停止语音模块任务并释放相关资源
 */
void voice_module_task_delete(void);

/**
 * @brief 获取运行模式标志（线程安全）
 * 
 * @return uint8_t 运行模式标志（0=停止，1=运行）
 */
uint8_t voice_module_get_run_mode(void);

/**
 * @brief 设置运行模式标志（线程安全）
 * 
 * @param mode 运行模式（0=停止，1=运行）
 */
void voice_module_set_run_mode(uint8_t mode);

/**
 * @brief 语音模块任务入口函数（内部使用）
 * 
 * @param pvParameters 任务参数（未使用）
 */
void voice_module_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* VOICE_MODULE_H */
