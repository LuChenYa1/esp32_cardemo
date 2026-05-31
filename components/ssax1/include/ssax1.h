#ifndef SSAX1_H
#define SSAX1_H

#include <stdint.h>

// 初始化GPIO函数
void ssax1_gpio_init(void);

// 轮询读取并打印GPIO状态
void ssax1_poll_gpio_status(void);
void ssax1_read(void);
#endif // SSAX1_H