/**
 * @file 485servo.c
 * @brief RS485总线舵机驱动实现
 * 
 * 功能：
 * - 控制RS485总线舵机
 * - 设置舵机位置和速度
 * - 支持单个和批量控制
 * 
 * 通信协议：
 * - 帧头：0xFF 0xFF
 * - 数据格式：ID + 指令 + 参数 + 校验和
 * 
 * 注意：
 * - 需要先初始化UART0和RS485方向控制
 * - 使用UART0的RS485互斥锁保护通信
 */

#include "485servo.h"
#include "pin_definitions.h"
#include "esp_log.h"
#include "uart.h"

#include <stdio.h>


// 舵机指令定义
static uint8_t Servo_play[] = {
    0xFF, 0xFF, // 帧头
    0x00,       // 舵机ID（0xFE代表选中所有舵机）
    0x07, 0x03, 0x1E, // 舵机运动指令
    0x00, 0x00, // 舵机位置设置（0-1024，低位在前）
    0x00, 0x00, // 舵机运动速度（0x01最小，0x64最大，低位在前）
    0x00        // 校验位
};

// 舵机位置读取指令定义
static uint8_t Servo_read_pos[] = {
    0xFF, 0xFF, // 帧头
    0x00,       // 舵机ID（0xFE代表选中所有舵机）
    0x04,
    0x02,
    0x24,
    0x02,
    0x00        // 校验位
};

/**
 * @brief 计算校验和
 * 
 * @param data 数据包指针，指向需要计算校验和的数据
 * @return uint8_t 返回计算得到的校验和值
 */
static uint8_t Parity_Check(uint8_t *data)
{
    // 初始化校验和变量为0
    uint8_t checksum = 0;
    // 循环遍历数据包中的第2到第9个字节（索引2-9），对它们求和
    for (uint8_t i = 2; i < 10; i++) {
        // 将当前字节加到校验和中
        checksum += data[i];
    }
    // 对总和进行按位取反操作，得到校验和
    checksum = (uint8_t)(~checksum);
    // 返回计算出的校验和
    return checksum;
}



/**
 * @brief 初始化RS485通信控制GPIO
 * 
 * 配置RS485方向控制引脚并设置初始状态
 */
void servo485_init(void)
{
    // 定义GPIO配置结构体，用于配置RS485方向控制引脚
    gpio_config_t dir_cfg = {
        // 设置要配置的GPIO引脚掩码，对应RS485_DIR_GPIO引脚
        .pin_bit_mask = 1ULL << RS485_DIR_GPIO,
        // 设置GPIO工作模式为输出模式
        .mode = GPIO_MODE_OUTPUT,
        // 禁用上拉电阻
        .pull_up_en = GPIO_PULLUP_DISABLE,
        // 禁用下拉电阻
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        // 禁用中断功能
        .intr_type = GPIO_INTR_DISABLE,
    };
    // 应用GPIO配置
    gpio_config(&dir_cfg);
    // 设置GPIO引脚电平为低电平（接收模式）
    gpio_set_level(RS485_DIR_GPIO, RS485_DIR_RX_LEVEL);
    
    // 初始化UART（使用正确的函数名），波特率为1000000
    uart0_init(1000000);
}

/**
 * @brief 设置舵机的位置
 * 
 * 向指定舵机发送位置控制命令，包括舵机ID、目标位置和移动速度
 * 使用互斥锁保护RS485总线访问，防止与摄像头通信冲突
 * 
 * @param id 舵机ID (0x00-0xFE, 0xFE表示所有舵机)
 * @param position 目标位置值 (0-1024)
 * @param speed 舵机移动速度 (0x01最小 - 0x64最大)
 */
void Set_Servo_position(uint8_t id, uint16_t position, uint8_t speed)
{
    // 获取RS485互斥锁
    SemaphoreHandle_t mutex = uart0_get_rs485_mutex();
    if (mutex == NULL) {
        ESP_LOGE("SERVO", "RS485互斥锁未初始化");
        return;
    }
    
    // 尝试获取互斥锁，最多等待200ms
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        ESP_LOGW("SERVO", "获取RS485互斥锁超时，舵机ID=0x%02X", id);
        return;
    }
    
    // 提取目标位置的低字节（LSB）
    uint8_t position_L = (uint8_t)position;
    // 提取目标位置的高字节（MSB）
    uint8_t position_H = (uint8_t)(position >> 8);

    // 设置舵机ID到指令数组的第3个字节
    Servo_play[2] = id;
    // 设置位置低字节到指令数组的第7个字节
    Servo_play[6] = position_L;
    // 设置位置高字节到指令数组的第8个字节
    Servo_play[7] = position_H;
    // 设置速度值到指令数组的第9个字节
    Servo_play[8] = speed;
    // 计算校验和并写入到指令数组的最后一个字节（第11个字节）
    Servo_play[10] = Parity_Check(Servo_play);

    // 设置485方向为发送模式
    gpio_set_level(RS485_DIR_GPIO, RS485_DIR_TX_LEVEL);
    
    // 通过UART发送完整的舵机控制指令（11字节）
    uart0_send(Servo_play, 11);
    
    // 发送完成后短暂延时确保数据发送完毕
    vTaskDelay(pdMS_TO_TICKS(10));  // 增加延时以确保数据发送完整
    // 设置485方向为接收模式
    gpio_set_level(RS485_DIR_GPIO, RS485_DIR_RX_LEVEL);
    
    // 释放互斥锁
    xSemaphoreGive(mutex);
}

/**
 * @brief 读取舵机当前位置（发送读取指令）
 * 使用互斥锁保护RS485总线访问，防止与摄像头通信冲突
 *
 * @param id 舵机ID (0x00-0xFE, 0xFE表示所有舵机)
 */
void Read_Servo_position(uint8_t id)
{
    // 获取RS485互斥锁
    SemaphoreHandle_t mutex = uart0_get_rs485_mutex();
    if (mutex == NULL) {
        ESP_LOGE("SERVO", "RS485互斥锁未初始化");
        return;
    }
    
    // 尝试获取互斥锁，最多等待200ms
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        ESP_LOGW("SERVO", "获取RS485互斥锁超时，舵机ID=0x%02X", id);
        return;
    }
    
    // 将传入的舵机ID写入到读取指令数组的第3个字节位置（索引为2）
    Servo_read_pos[2] = id;

    // 定义一个变量存储校验和，初始化为0
    uint8_t checksum = 0;
    // 循环计算从第3个字节到第7个字节（不包括第8个字节校验位）的累加和
    for (uint8_t i = 2; i < 7; i++) {
        // 将当前字节添加到校验和中
        checksum += Servo_read_pos[i];
    }
    // 对累加和进行按位取反运算，得到校验和并写入到指令数组的第8个字节（索引为7）
    Servo_read_pos[7] = (uint8_t)(~checksum);

    // 设置485方向为发送模式
    gpio_set_level(RS485_DIR_GPIO, RS485_DIR_TX_LEVEL);
    // 通过UART发送整个读取位置指令，共8个字节
    uart0_send(Servo_read_pos, 8);
    // 发送完成后短暂延时确保数据发送完毕
    vTaskDelay(pdMS_TO_TICKS(1));
    // 设置485方向为接收模式
    gpio_set_level(RS485_DIR_GPIO, RS485_DIR_RX_LEVEL);
    
    // 释放互斥锁
    xSemaphoreGive(mutex);
}

/**
 * @brief 反初始化485伺服模块
 * 
 * 释放485通信相关的资源，包括GPIO和UART，
 * 确保模块安全关闭。
 */
void servo485_deinit(void)
{
    uart0_deinit();
}