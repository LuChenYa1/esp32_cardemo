#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include <stdio.h>
#include "driver/ledc.h"

#define LEDC_TIMER              LEDC_TIMER_0          // 定义LEDC定时器编号为定时器0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE   // 定义LEDC工作模式为低速模式
#define LEDC_OUTPUT_IO_1        (2)                   // 电机1 PWM正向输出GPIO
#define LEDC_OUTPUT_IO_2        (3)                   // 电机1 PWM反向输出GPIO
#define MOTOR_CHANNEL_1         LEDC_CHANNEL_0        // 电机1正向通道
#define MOTOR_CHANNEL_2         LEDC_CHANNEL_1        // 电机1反向通道
#define LEDC_OUTPUT_IO_3        (4)                   // 电机2 PWM正向输出GPIO
#define LEDC_OUTPUT_IO_4        (5)                   // 电机2 PWM反向输出GPIO
#define LEDC_OUTPUT_IO_5        (6)                   // 电机3 PWM正向输出GPIO
#define LEDC_OUTPUT_IO_6        (7)                   // 电机3 PWM反向输出GPIO
#define LEDC_OUTPUT_IO_7        (8)                   // 电机4 PWM正向输出GPIO
#define LEDC_OUTPUT_IO_8        (9)                   // 电机4 PWM反向输出GPIO
#define MOTOR2_CHANNEL_1        LEDC_CHANNEL_2        // 电机2正向通道
#define MOTOR2_CHANNEL_2        LEDC_CHANNEL_3        // 电机2反向通道
#define MOTOR3_CHANNEL_1        LEDC_CHANNEL_4        // 电机3正向通道
#define MOTOR3_CHANNEL_2        LEDC_CHANNEL_5        // 电机3反向通道
#define MOTOR4_CHANNEL_1        LEDC_CHANNEL_6        // 电机4正向通道
#define MOTOR4_CHANNEL_2        LEDC_CHANNEL_7        // 电机4反向通道
#define LEDC_DUTY_RES           LEDC_TIMER_10_BIT     // 定义占空比分辨率为10位（1024级调节）
#define LEDC_FREQUENCY          (1000)                // 定义PWM输出频率为1000Hz
#define PWM_MAX_DUTY            ((1 << LEDC_DUTY_RES) - 1)  // PWM最大占空比值（1023）

void ledc_init(void);

// 电机速度控制函数
// 参数范围: 0-1023 (直接对应硬件占空比，无需转换)
// motor1_duty: 正向占空比 (0-1023)
// motor2_duty: 反向占空比 (0-1023)
void set_motor1_speed(uint16_t motor1_duty, uint16_t motor2_duty);
void set_motor2_speed(uint16_t motor1_duty, uint16_t motor2_duty);
void set_motor3_speed(uint16_t motor1_duty, uint16_t motor2_duty);
void set_motor4_speed(uint16_t motor1_duty, uint16_t motor2_duty);

#endif // PWM_H

// #ifndef PWM_H
// #define PWM_H

// #include <stdint.h>
// #include <stdio.h>

// #define LEDC_TIMER              LEDC_TIMER_0          // 定义LEDC定时器编号为定时器0
// #define LEDC_MODE               LEDC_LOW_SPEED_MODE   // 定义LEDC工作模式为低速模式
// #define LEDC_OUTPUT_IO_1        (21)                  // 定义第一个PWM输出GPIO引脚为21号引脚
// #define LEDC_OUTPUT_IO_2        (26)                  // 定义第二个PWM输出GPIO引脚为26号引脚
// #define MOTOR_CHANNEL_1         LEDC_CHANNEL_0        // 定义电机1对应的LEDC通道为通道0
// #define MOTOR_CHANNEL_2         LEDC_CHANNEL_1        // 定义电机2对应的LEDC通道为通道1
// #define LEDC_OUTPUT_IO_3        (4)                   // 电机2 PWM正向输出GPIO
// #define LEDC_OUTPUT_IO_4        (6)                   // 电机2 PWM反向输出GPIO
// #define LEDC_OUTPUT_IO_5        (19)                  // 电机3 PWM正向输出GPIO
// #define LEDC_OUTPUT_IO_6        (20)                  // 电机3 PWM反向输出GPIO
// #define LEDC_OUTPUT_IO_7        (15)                  // 电机4 PWM正向输出GPIO
// #define LEDC_OUTPUT_IO_8        (16)                  // 电机4 PWM反向输出GPIO
// #define MOTOR2_CHANNEL_1        LEDC_CHANNEL_2        // 电机2正向通道
// #define MOTOR2_CHANNEL_2        LEDC_CHANNEL_3        // 电机2反向通道
// #define MOTOR3_CHANNEL_1        LEDC_CHANNEL_4        // 电机3正向通道
// #define MOTOR3_CHANNEL_2        LEDC_CHANNEL_5        // 电机3反向通道
// #define MOTOR4_CHANNEL_1        LEDC_CHANNEL_6        // 电机4正向通道
// #define MOTOR4_CHANNEL_2        LEDC_CHANNEL_7        // 电机4反向通道
// #define LEDC_DUTY_RES           LEDC_TIMER_11_BIT     // 定义占空比分辨率为11位（2048级调节）
// #define LEDC_FREQUENCY          (1000)                // 定义PWM输出频率为1000Hz

// void ledc_init(void);
// // 说明：单电机双PWM口，分别控制正向/反向占空比（宏名称不变）
// void set_motor_speed(uint8_t motor1_percent, uint8_t motor2_percent);
// void set_motor2_speed(uint8_t motor1_percent, uint8_t motor2_percent);
// void set_motor3_speed(uint8_t motor1_percent, uint8_t motor2_percent);
// void set_motor4_speed(uint8_t motor1_percent, uint8_t motor2_percent);

// #endif // PWM_H

