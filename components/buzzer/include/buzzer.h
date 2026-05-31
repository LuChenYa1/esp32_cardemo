#ifndef BUZZER_H
#define BUZZER_H

#include "driver/gpio.h"
#include "pin_definitions.h"

// 蜂鸣器引脚定义在 pin_definitions.h 中
// #define BUZZER_GPIO GPIO_NUM_26

// 初始化蜂鸣器
void buzzer_init(void);

// 打开蜂鸣器
void buzzer_on(void);

// 关闭蜂鸣器
void buzzer_off(void);

// 蜂鸣器鸣叫一段时间（阻塞）
void buzzer_beep(int duration_ms);

#endif // BUZZER_H