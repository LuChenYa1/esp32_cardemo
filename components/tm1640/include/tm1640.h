#ifndef TM1640_H
#define TM1640_H

#include <stdint.h>
#include "driver/gpio.h"
#include "pin_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TM1640 点阵屏配置 */
#define TM1640_ROWS    8U   // 8行
#define TM1640_COLS    16U  // 16列

/* 函数声明 */
void tm1640_init(void);                                      // 初始化 TM1640
void tm1640_clear(void);                                     // 清空显示缓存
void tm1640_set_led(uint8_t row, uint8_t col, uint8_t on);  // 设置单个像素点
void tm1640_refresh(void);                                   // 刷新显示
void tm1640_set_brightness(uint8_t level);                   // 设置亮度（0-7）
void tm1640_show_gstem_scroll_step(uint16_t step_delay_ms);  // GSTEM 滚动动画

#ifdef __cplusplus
}
#endif

#endif /* TM1640_H */
