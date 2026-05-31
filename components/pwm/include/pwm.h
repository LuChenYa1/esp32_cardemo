#ifndef PWM_H
#define PWM_H

#include <stdint.h>
#include <stdio.h>
#include "driver/ledc.h"
#include "pin_definitions.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO_1        MOTOR1_FWD_GPIO
#define LEDC_OUTPUT_IO_2        MOTOR1_REV_GPIO
#define MOTOR_CHANNEL_1         LEDC_CHANNEL_0
#define MOTOR_CHANNEL_2         LEDC_CHANNEL_1
#define LEDC_OUTPUT_IO_3        MOTOR2_FWD_GPIO
#define LEDC_OUTPUT_IO_4        MOTOR2_REV_GPIO
#define LEDC_OUTPUT_IO_5        MOTOR3_FWD_GPIO
#define LEDC_OUTPUT_IO_6        MOTOR3_REV_GPIO
#define LEDC_OUTPUT_IO_7        MOTOR4_FWD_GPIO
#define LEDC_OUTPUT_IO_8        MOTOR4_REV_GPIO
#define MOTOR2_CHANNEL_1        LEDC_CHANNEL_2
#define MOTOR2_CHANNEL_2        LEDC_CHANNEL_3
#define MOTOR3_CHANNEL_1        LEDC_CHANNEL_4
#define MOTOR3_CHANNEL_2        LEDC_CHANNEL_5
#define MOTOR4_CHANNEL_1        LEDC_CHANNEL_6
#define MOTOR4_CHANNEL_2        LEDC_CHANNEL_7
#define LEDC_DUTY_RES           LEDC_TIMER_10_BIT
#define LEDC_FREQUENCY          (1000)
#define PWM_MAX_DUTY            ((1 << LEDC_DUTY_RES) - 1)

void ledc_init(void);
void set_motor1_speed(uint16_t motor1_duty, uint16_t motor2_duty);
void set_motor2_speed(uint16_t motor1_duty, uint16_t motor2_duty);
void set_motor3_speed(uint16_t motor1_duty, uint16_t motor2_duty);
void set_motor4_speed(uint16_t motor1_duty, uint16_t motor2_duty);

#endif // PWM_H
