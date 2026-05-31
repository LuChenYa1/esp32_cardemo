#include "pwm.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "PWM";

/**
 * @brief 初始化指定的LEDC通道配置
 * 
 * 该函数用于初始化一个LEDC（LED PWM控制器）通道，设置其工作模式、定时器选择、GPIO引脚等参数。
 * 初始化完成后，通过调用`ledc_channel_config`函数应用配置。
 * 
 * @param channel 要初始化的LEDC通道号，类型为`ledc_channel_t`
 * @param gpio_num 与该通道关联的GPIO引脚编号
 */
static void ledc_channel_init(ledc_channel_t channel, int gpio_num)
{
    // 配置LEDC通道的参数结构体
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,          // 设置LEDC的工作模式
        .channel = channel,               // 指定要配置的通道
        .timer_sel = LEDC_TIMER,          // 选择使用的定时器
        .intr_type = LEDC_INTR_DISABLE,   // 禁用中断
        .gpio_num = gpio_num,             // 关联的GPIO引脚编号
        .duty = 0,                        // 初始占空比设为0
        .hpoint = 0,                      // 初始高电平时间点设为0
    };

    // 应用LEDC通道配置，并检查是否出错
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

/**
 * @brief 设置电机正转和反转通道的占空比，以控制电机速度
 *
 * 该函数通过配置LEDC（LED PWM控制器）通道的占空比来控制电机的正转和反转速度
 * 直接使用硬件分辨率值，无需映射转换
 * 
 * @param forward_channel  正转通道的LEDC通道号
 * @param reverse_channel  反转通道的LEDC通道号
 * @param forward_duty     正转占空比值（0-1023，直接对应硬件分辨率）
 * @param reverse_duty     反转占空比值（0-1023，直接对应硬件分辨率）
 */
static void set_motor_speed_channels(ledc_channel_t forward_channel,
                                     ledc_channel_t reverse_channel,
                                     uint16_t forward_duty,
                                     uint16_t reverse_duty)
{
    // 限制输入值在0-1023范围内（硬件最大值）
    if (forward_duty > PWM_MAX_DUTY) forward_duty = PWM_MAX_DUTY;
    if (reverse_duty > PWM_MAX_DUTY) reverse_duty = PWM_MAX_DUTY;

    // 直接设置占空比，无需映射转换
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, forward_channel, forward_duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, forward_channel));

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, reverse_channel, reverse_duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, reverse_channel));
}

/**
 * @brief 初始化LEDC（LED Control）外设
 * 
 * 该函数配置LEDC定时器和8个LEDC通道，用于4个电机的双PWM口输出
 * 配置包括：定时器参数设置、通道1~通道8配置
 * 
 * @param void 无输入参数
 * @return void 无返回值
 */
void ledc_init(void)  // 定义ledc初始化函数
{
    // 配置LEDC定时器参数
    ledc_timer_config_t ledc_timer = {  // 创建LEDC定时器配置结构体实例
        .speed_mode = LEDC_MODE,        // 设置速度模式（高速/低速模式）
        .timer_num = LEDC_TIMER,        // 选择要配置的定时器编号
        .duty_resolution = LEDC_DUTY_RES, // 设定占空比分辨率（影响PWM精度）
        .freq_hz = LEDC_FREQUENCY,      // 设置PWM输出频率（单位Hz）
        .clk_cfg = LEDC_AUTO_CLK,       // 选择时钟源（自动选择最佳时钟源）
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));  // 应用定时器配置，并检查错误

    // 电机1（IO21/26）
    ledc_channel_init(MOTOR_CHANNEL_1, LEDC_OUTPUT_IO_1);
    ledc_channel_init(MOTOR_CHANNEL_2, LEDC_OUTPUT_IO_2);

    // 电机2（IO4/6）
    ledc_channel_init(MOTOR2_CHANNEL_1, LEDC_OUTPUT_IO_3);
    ledc_channel_init(MOTOR2_CHANNEL_2, LEDC_OUTPUT_IO_4);

    // 电机3（IO19/20）
    ledc_channel_init(MOTOR3_CHANNEL_1, LEDC_OUTPUT_IO_5);
    ledc_channel_init(MOTOR3_CHANNEL_2, LEDC_OUTPUT_IO_6);

    // 电机4（IO15/16）
    ledc_channel_init(MOTOR4_CHANNEL_1, LEDC_OUTPUT_IO_7);
    ledc_channel_init(MOTOR4_CHANNEL_2, LEDC_OUTPUT_IO_8);

} 

/**
 * @brief 设置单电机正/反向PWM占空比
 * 
 * 该函数通过LED控制器设置单电机正向/反向PWM占空比，从而控制电机转速
 * 
 * @param motor1_duty 正向占空比 (0-1023，直接对应硬件分辨率)
 * @param motor2_duty 反向占空比 (0-1023，直接对应硬件分辨率)
 * 
 * @return 无返回值
 */

//  void set_motor_speed(uint16_t motor1_duty, uint16_t motor2_duty)
// {
//     set_motor_speed_channels(MOTOR_CHANNEL_1, MOTOR_CHANNEL_2, motor2_duty, motor1_duty);
//     // ESP_LOGI(TAG, "电机1 正向: %d/1023, 反向: %d/1023", motor1_duty, motor2_duty);
// }

// void set_motor2_speed(uint16_t motor1_duty, uint16_t motor2_duty)
// {
//         set_motor_speed_channels(MOTOR3_CHANNEL_1, MOTOR3_CHANNEL_2, motor2_duty, motor1_duty);
//     // ESP_LOGI(TAG, "电机3 正向: %d/1023, 反向: %d/1023", motor1_duty, motor2_duty);
// }

// void set_motor3_speed(uint16_t motor1_duty, uint16_t motor2_duty)
// {
//     set_motor_speed_channels(MOTOR2_CHANNEL_1, MOTOR2_CHANNEL_2, motor1_duty, motor2_duty);
//     // ESP_LOGI(TAG, "电机2 正向: %d/1023, 反向: %d/1023", motor1_duty, motor2_duty);
// }
// void set_motor4_speed(uint16_t motor1_duty, uint16_t motor2_duty)
// {
//     set_motor_speed_channels(MOTOR4_CHANNEL_1, MOTOR4_CHANNEL_2, motor1_duty, motor2_duty);
//     // ESP_LOGI(TAG, "电机4 正向: %d/1023, 反向: %d/1023", motor1_duty, motor2_duty);
// } 
void set_motor3_speed(uint16_t motor1_duty, uint16_t motor2_duty)
{
    set_motor_speed_channels(MOTOR_CHANNEL_1, MOTOR_CHANNEL_2, motor2_duty, motor1_duty);
    // ESP_LOGI(TAG, "电机1 正向: %d/1023, 反向: %d/1023", motor1_duty, motor2_duty);
}

void set_motor4_speed(uint16_t motor1_duty, uint16_t motor2_duty)
{
        set_motor_speed_channels(MOTOR3_CHANNEL_1, MOTOR3_CHANNEL_2, motor2_duty, motor1_duty);
    // ESP_LOGI(TAG, "电机3 正向: %d/1023, 反向: %d/1023", motor1_duty, motor2_duty);
}

void set_motor1_speed(uint16_t motor1_duty, uint16_t motor2_duty)
{
    set_motor_speed_channels(MOTOR2_CHANNEL_1, MOTOR2_CHANNEL_2, motor1_duty, motor2_duty);
    // ESP_LOGI(TAG, "电机2 正向: %d/1023, 反向: %d/1023", motor1_duty, motor2_duty);
}
void set_motor2_speed(uint16_t motor1_duty, uint16_t motor2_duty)
{
    set_motor_speed_channels(MOTOR4_CHANNEL_1, MOTOR4_CHANNEL_2, motor1_duty, motor2_duty);
    // ESP_LOGI(TAG, "电机4 正向: %d/1023, 反向: %d/1023", motor1_duty, motor2_duty);
}

// #include "pwm.h"
// #include "driver/ledc.h"
// #include "esp_log.h"
// #include "esp_err.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// static const char *TAG = "PWM";

// /**
//  * @brief 初始化指定的LEDC通道配置
//  * 
//  * 该函数用于初始化一个LEDC（LED PWM控制器）通道，设置其工作模式、定时器选择、GPIO引脚等参数。
//  * 初始化完成后，通过调用`ledc_channel_config`函数应用配置。
//  * 
//  * @param channel 要初始化的LEDC通道号，类型为`ledc_channel_t`
//  * @param gpio_num 与该通道关联的GPIO引脚编号
//  */
// static void ledc_channel_init(ledc_channel_t channel, int gpio_num)
// {
//     // 配置LEDC通道的参数结构体
//     ledc_channel_config_t ledc_channel = {
//         .speed_mode = LEDC_MODE,          // 设置LEDC的工作模式
//         .channel = channel,               // 指定要配置的通道
//         .timer_sel = LEDC_TIMER,          // 选择使用的定时器
//         .intr_type = LEDC_INTR_DISABLE,   // 禁用中断
//         .gpio_num = gpio_num,             // 关联的GPIO引脚编号
//         .duty = 0,                        // 初始占空比设为0
//         .hpoint = 0,                      // 初始高电平时间点设为0
//     };

//     // 应用LEDC通道配置，并检查是否出错
//     ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
// }

// /**
//  * @brief 设置电机正转和反转通道的占空比，以控制电机速度。
//  *
//  * 该函数通过配置LEDC（LED PWM控制器）通道的占空比来控制电机的正转和反转速度。
//  * 正转和反转的占空比分别由传入的百分比参数计算得出，并更新到对应的LEDC通道。
//  *
//  * @param forward_channel  正转通道的LEDC通道号
//  * @param reverse_channel  反转通道的LEDC通道号
//  * @param forward_percent  正转速度的百分比（0-100）
//  * @param reverse_percent  反转速度的百分比（0-100）
//  */
// static void set_motor_speed_channels(ledc_channel_t forward_channel,
//                                      ledc_channel_t reverse_channel,
//                                      uint8_t forward_percent,
//                                      uint8_t reverse_percent)
// {
//     // 计算最大占空比值，基于LEDC分辨率
//     uint32_t max_duty = (1 << LEDC_DUTY_RES) - 1;

//     // 根据输入的百分比计算正转和反转的实际占空比值
//     uint32_t duty_fwd = (max_duty * forward_percent) / 100;
//     uint32_t duty_rev = (max_duty * reverse_percent) / 100;

//     // 设置并更新正转通道的占空比
//     ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, forward_channel, duty_fwd));
//     ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, forward_channel));

//     // 设置并更新反转通道的占空比
//     ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, reverse_channel, duty_rev));
//     ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, reverse_channel));
// }

// /**
//  * @brief 初始化LEDC（LED Control）外设
//  * 
//  * 该函数配置LEDC定时器和8个LEDC通道，用于4个电机的双PWM口输出
//  * 配置包括：定时器参数设置、通道1~通道8配置
//  * 
//  * @param void 无输入参数
//  * @return void 无返回值
//  */
// void ledc_init(void)  // 定义ledc初始化函数
// {
//     // 配置LEDC定时器参数
//     ledc_timer_config_t ledc_timer = {  // 创建LEDC定时器配置结构体实例
//         .speed_mode = LEDC_MODE,        // 设置速度模式（高速/低速模式）
//         .timer_num = LEDC_TIMER,        // 选择要配置的定时器编号
//         .duty_resolution = LEDC_DUTY_RES, // 设定占空比分辨率（影响PWM精度）
//         .freq_hz = LEDC_FREQUENCY,      // 设置PWM输出频率（单位Hz）
//         .clk_cfg = LEDC_AUTO_CLK,       // 选择时钟源（自动选择最佳时钟源）
//     };
//     ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));  // 应用定时器配置，并检查错误

//     // 电机1（IO21/26）
//     ledc_channel_init(MOTOR_CHANNEL_1, LEDC_OUTPUT_IO_1);
//     ledc_channel_init(MOTOR_CHANNEL_2, LEDC_OUTPUT_IO_2);

//     // 电机2（IO4/6）
//     ledc_channel_init(MOTOR2_CHANNEL_1, LEDC_OUTPUT_IO_3);
//     ledc_channel_init(MOTOR2_CHANNEL_2, LEDC_OUTPUT_IO_4);

//     // 电机3（IO19/20）
//     ledc_channel_init(MOTOR3_CHANNEL_1, LEDC_OUTPUT_IO_5);
//     ledc_channel_init(MOTOR3_CHANNEL_2, LEDC_OUTPUT_IO_6);

//     // 电机4（IO15/16）
//     ledc_channel_init(MOTOR4_CHANNEL_1, LEDC_OUTPUT_IO_7);
//     ledc_channel_init(MOTOR4_CHANNEL_2, LEDC_OUTPUT_IO_8);

// } 

// /**
//  * @brief 设置单电机正/反向PWM占空比
//  * 
//  * 该函数通过LED控制器设置单电机正向/反向PWM占空比，从而控制电机转速
//  * 
//  * @param motor1_percent 正向转速百分比 (0-100)
//  * @param motor2_percent 反向转速百分比 (0-100)
//  * 
//  * @return 无返回值
//  */
// void set_motor_speed(uint8_t motor1_percent, uint8_t motor2_percent)
// {
//     set_motor_speed_channels(MOTOR_CHANNEL_1, MOTOR_CHANNEL_2, motor1_percent, motor2_percent);

//     // 记录当前电机占空比信息到日志
//     ESP_LOGI(TAG, "电机1 正向占空比: %d%%, 反向占空比: %d%%", motor1_percent, motor2_percent);
// }

// void set_motor2_speed(uint8_t motor1_percent, uint8_t motor2_percent)
// {
//     set_motor_speed_channels(MOTOR2_CHANNEL_1, MOTOR2_CHANNEL_2, motor1_percent, motor2_percent);
//     ESP_LOGI(TAG, "电机2 正向占空比: %d%%, 反向占空比: %d%%", motor1_percent, motor2_percent);
// }

// void set_motor3_speed(uint8_t motor1_percent, uint8_t motor2_percent)
// {
//     set_motor_speed_channels(MOTOR3_CHANNEL_1, MOTOR3_CHANNEL_2, motor1_percent, motor2_percent);
//     ESP_LOGI(TAG, "电机3 正向占空比: %d%%, 反向占空比: %d%%", motor1_percent, motor2_percent);
// }

// void set_motor4_speed(uint8_t motor1_percent, uint8_t motor2_percent)
// {
//     set_motor_speed_channels(MOTOR4_CHANNEL_1, MOTOR4_CHANNEL_2, motor1_percent, motor2_percent);
//     ESP_LOGI(TAG, "电机4 正向占空比: %d%%, 反向占空比: %d%%", motor1_percent, motor2_percent);
// }