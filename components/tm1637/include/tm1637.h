// 防止头文件重复包含的宏定义
#ifndef TM1637_H
#define TM1637_H

// 包含必要的标准库头文件
#include <stdint.h>    // 提供标准整数类型定义，如uint8_t、uint16_t等
#include <stdbool.h>   // 提供布尔类型bool及其值true/false定义

// 数码管显示特殊值定义
#define TUBE_DISPLAY_NULL			26        // 特殊显示值，表示数码管该位不显示（空白）


// TM1637 命令字基本定义（根据TM1637数据手册）
#define CMD_DATA 0x40 // 数据命令标志: 0100 0000，用于向TM1637发送数据
#define CMD_DISP 0x80 // 显示控制命令标志: 1000 0000，用于控制显示开关和亮度
#define CMD_ADDR 0xC0 // 地址命令标志: 1100 0000，用于设置显示寄存器地址

// 基于基本命令构建的具体命令定义
#define WRT_DATA (0x00 | CMD_DATA)          // 写数据命令 = 0100 0000，普通写数据模式
#define DISP_ON  (0x08 | CMD_DISP)          // 开启显示命令 = 1000 1000，同时控制亮度
#define DISP_OFF (0x00 | CMD_DISP)          // 关闭显示命令 = 1000 0000，关闭显示但保持数据
#define SET_ADDR (0x00 | CMD_ADDR)          // 设置地址命令 = 1100 0000，从第一个显示地址开始

// 外部常量数组声明
extern const uint8_t num_code[];            // 数字字符编码表，存储0-9数字对应的7段数码管编码
extern const uint8_t u8TubeAddrTab[];       // 数码管地址映射表，存储各个数码管位对应的内部地址

// 基础硬件操作函数声明
void tm1637_init(void);                     // 初始化TM1637数码管（配置GPIO和初始设置）
void tm1637_wt_sda(bool state);             // 设置DIO数据线电平状态：true为高电平，false为低电平
void tm1637_wt_scl(bool state);             // 设置CLK时钟线电平状态：true为高电平，false为低电平
uint8_t tm1637_rd_sda();                    // 读取DIO数据线当前电平状态，返回0或1

// I2C-like通信协议函数声明
void tm1637_start();                        // 发送TM1637起始信号，拉低SDA后拉低SCL
void tm1637_stop();                         // 发送TM1637停止信号，拉高SCL后拉高SDA
uint8_t tm1637_rd_ack();                    // 读取应答信号，确认TM1637是否正确接收数据

// 数据传输函数声明
void tm1637_wt_byte(uint8_t byte);          // 向TM1637发送一个完整的字节数据
void tm1637_wt_cmd(uint8_t cmd);            // 向TM1637发送命令字节
void tm1637_wt_data(uint8_t grid, uint8_t data); // 向指定数码管位置(grid)写入显示数据(data)

// 应用层控制函数声明
void tm1637_switch(bool bstate);            // 控制整个数码管显示开关：true开启显示，false关闭显示
void tm1637_tubedisplay(uint8_t * td);      // 批量更新数码管显示内容，参数td是指向显示数据数组的指针
void tm1637_disp_num_process(uint16_t u16Data); // 将16位数值(u16Data)转换为4位数码管显示格式并输出

#endif // TM1637_H 结束头文件保护