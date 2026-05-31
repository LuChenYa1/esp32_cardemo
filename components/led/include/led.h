#ifndef LED_H
#define LED_H

#include "driver/gpio.h"
#include "pin_definitions.h"

// LED引脚定义在 pin_definitions.h 中
// #define LED_GPIO GPIO_NUM_0

// 函数声明
void led_init(void);
void led_on(void);
void led_off(void);
void led_toggle(void);

#endif // LED_H