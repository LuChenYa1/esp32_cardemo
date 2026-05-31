#ifndef SSDX_H
#define SSDX_H

#include <stdint.h>

// ADC 初始化函数
void ssdx_adc_init(void);

// 读取ADC电压函数，channel: 0 for GPIO1, 1 for GPIO2
int ssdx_read_voltage(int channel);

#endif // SSDX_H