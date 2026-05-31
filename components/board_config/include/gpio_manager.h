/**
 * @file gpio_manager.h
 * @brief GPIO资源分配和冲突检测模块
 * 
 * 本模块负责管理所有GPIO引脚的分配，防止多个功能模块使用同一个GPIO引脚。
 * 在系统初始化时，各模块应调用gpio_manager_register()注册其使用的GPIO。
 * 如果检测到冲突，系统将报告错误并停止启动。
 * 
 * 注意：本文件保留用于向后兼容，新代码请使用 board_config.h
 *       所有类型和函数定义都在 board_config.h 中
 */

#ifndef GPIO_MANAGER_H
#define GPIO_MANAGER_H

// 直接包含 board_config.h，所有定义都在那里
#include "board_config.h"

// 为了向后兼容，不需要重复定义任何内容
// 所有的类型、函数声明都已经在 board_config.h 中定义

#endif // GPIO_MANAGER_H
