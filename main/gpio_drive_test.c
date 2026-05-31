

// /*
//  * ESP32-S3 引脚输出驱动能力测试
//  *
//  * 测试核心板 U26/U27 引出的所有可用 GPIO（排除串口和电源）
//  * 提供两个函数：所有引脚置高 / 所有引脚置低
//  *
//  * ⚠️ 冲突说明：
//  *   - GPIO37: U26-KEY2 与 U27-SSA2 共用，同一引脚
//  *   - GPIO38: U27-SSA1 与 U27-KEY1 共用，同一引脚
//  *   - GPIO45: U26-BEEP(PWM) 与 U27-A2(编码器) 共用，同一引脚
//  *   - GPIO19: U26-PS(485DIR) 与 U27-485DIR 共用，同一引脚
//  *   - SSA5/SSD5: 连接 PCF8574 I/O 扩展芯片，非直接 GPIO，不在本测试范围
//  *
//  * 排除的串口引脚：
//  *   - TXD1=GPIO43, RXD1=GPIO44（UART0，默认日志口）
//  *   - TXD2=GPIO17(⚠️ 与 B4/编码器4 共用), RXD2=GPIO18
//  *   - 485通信: GPIO17(TX)/GPIO18(RX)
//  *
//  * 完整引脚映射（U26 / U27）：
//  * +--------+----------+---------+  +--------+----------+---------+
//  * | 标识   | GPIO     | 备注    |  | 标识   | GPIO     | 备注    |
//  * +--------+----------+---------+  +--------+----------+---------+
//  * | KEY2   | GPIO37   | 输入键  |  | SSA1   | GPIO38   | DHT11   |
//  * | R      | GPIO0    | LED     |  | SSA2   | GPIO37   | 触摸    |
//  * | SPI_NSS| GPIO10   | SPI CS  |  | SSA3   | GPIO34   | TM_SCL  |
//  * | SPI_SCK| GPIO12   | SPI CLK |  | SSD3   | GPIO33   | TM_SDA  |
//  * | SPI_MIS| GPIO13   | SPI MISO|  | SSA4   | GPIO30   | 预留    |
//  * | SPI_MOS| GPIO11   | SPI MOSI|  | SSD4   | GPIO31   | 预留    |
//  * | BEEP   | GPIO45   | 蜂鸣器  |  | SSA6   | GPIO40   | VL_SCL  |
//  * | PWM1   | GPIO2    | 电机1+  |  | SSD6   | GPIO39   | VL_SDA  |
//  * | PWM2   | GPIO3    | 电机1-  |  | 485DIR | GPIO19   | RS485方向|
//  * | PWM3   | GPIO4    | 电机2+  |  | A1     | GPIO41   | 编码器1A|
//  * | PWM4   | GPIO5    | 电机2-  |  | B1     | GPIO42   | 编码器1B|
//  * | PWM5   | GPIO6    | 电机3+  |  | A2     | GPIO45   | 编码器2A|
//  * | PWM6   | GPIO7    | 电机3-  |  | B2     | GPIO46   | 编码器2B|
//  * | PWM7   | GPIO8    | 电机4+  |  | KEY1   | GPIO38   | 输入键  |
//  * | PWM8   | GPIO9    | 电机4-  |  +--------+----------+---------+
//  * | A3     | GPIO14   | 编码器3A|
//  * | B3     | GPIO15   | 编码器3B|
//  * | A4     | GPIO16   | 编码器4A|
//  * | B4     | GPIO17   | 编码器4B|
//  * | PS     | GPIO19   | 485DIR  |
//  * +--------+----------+---------+
//  */

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/gpio.h"
// #include "esp_log.h"

// static const char *TAG = "GPIO驱动测试";

// /* 所有待测 GPIO 列表（去重后，排除串口 GPIO43/44/17/18） */
// static const gpio_num_t test_gpios[] = {
//     GPIO_NUM_1,  // R      - LED
//     GPIO_NUM_2,  // PWM1   - 电机1+
//     GPIO_NUM_3,  // PWM2   - 电机1-
//     GPIO_NUM_4,  // PWM3   - 电机2+
//     GPIO_NUM_5,  // PWM4   - 电机2-
//     GPIO_NUM_6,  // PWM5   - 电机3+
//     GPIO_NUM_7,  // PWM6   - 电机3-
//     GPIO_NUM_8,  // PWM7   - 电机4+
//     GPIO_NUM_9,  // PWM8   - 电机4-
//     GPIO_NUM_10, // SPI_NSS
//     GPIO_NUM_11, // SPI_MOS
//     GPIO_NUM_12, // SPI_SCK
//     GPIO_NUM_13, // SPI_MIS
//     GPIO_NUM_14, // A3     - 编码器3A
//     GPIO_NUM_15, // B3     - 编码器3B
//     GPIO_NUM_16, // A4     - 编码器4A
//     GPIO_NUM_17, // A4     - 编码器4A
//     GPIO_NUM_18, // A4     - 编码器4A
//     GPIO_NUM_19, // PS / 485DIR
//     GPIO_NUM_20, // PS / 485DIR
//     GPIO_NUM_21, // PS / 485DIR
//     GPIO_NUM_26, // PS / 485DIR


//     GPIO_NUM_33, // SSD3   - TM1637 SDA

//     // GPIO_NUM_31, // SSA4   - 预留
//     // GPIO_NUM_32, // SSD4   - 预留
//     // GPIO_NUM_34, // SSA3   - TM1637 SCL
//     // GPIO_NUM_35, // SSD3   - TM1637 SDA
//     // GPIO_NUM_36, // SSD3   - TM1637 SDA
//     // GPIO_NUM_39, // SSD6   - VL53L0X SDA

//     GPIO_NUM_37, // KEY2 / SSA2 - 触摸（⚠️ 同一引脚）
//     GPIO_NUM_38, // SSA1 / KEY1 - DHT11（⚠️ 同一引脚）
//     GPIO_NUM_40, // SSA6   - VL53L0X SCL
//     GPIO_NUM_41, // A1     - 编码器1A
//     GPIO_NUM_42, // B1     - 编码器1B
//     GPIO_NUM_45, // BEEP / A2（⚠️ 同一引脚）
//     GPIO_NUM_46, // B2     - 编码器2B
//         GPIO_NUM_47, // BEEP / A2（⚠️ 同一引脚）
//     GPIO_NUM_48, // B2     - 编码器2B
// };


// #define TEST_GPIO_COUNT (sizeof(test_gpios) / sizeof(test_gpios[0]))

// /**
//  * @brief 初始化所有测试引脚为输出模式
//  */
// static void gpio_drive_test_init(void)
// {
//     gpio_config_t io_conf = {
//         .intr_type    = GPIO_INTR_DISABLE,
//         .mode         = GPIO_MODE_OUTPUT,
//         .pull_down_en = GPIO_PULLDOWN_DISABLE,
//         .pull_up_en   = GPIO_PULLUP_DISABLE,
//         .pin_bit_mask = 0,
//     };

//     for (int i = 0; i < TEST_GPIO_COUNT; i++) {
//         io_conf.pin_bit_mask |= (1ULL << test_gpios[i]);
//     }

//     ESP_ERROR_CHECK(gpio_config(&io_conf));
//     ESP_LOGI(TAG, "所有测试引脚已初始化为输出模式，共 %d 个", TEST_GPIO_COUNT);
// }

// // 新增：初始化特殊引脚为输入模式
// static void gpio_special_pins_input_init(void)
// {
//     // ESP32-S3的一些特殊引脚，可能不能安全用作输出，配置为输入模式
//     static const gpio_num_t input_gpios[] = {
//         // GPIO_NUM_31,  // 预留引脚，配置为输入
//         // GPIO_NUM_32,  // 预留引脚，配置为输入
        
//         GPIO_NUM_34,    // 如果该引脚用于特殊功能，配置为输入
//         GPIO_NUM_35,  // 如果该引脚用于特殊功能，配置为输入
//         GPIO_NUM_36,  // 如果该引脚用于特殊功能，配置为输入
//         GPIO_NUM_39,  // VL53L0X SDA - 根据实际情况决定是否需要配置为输入
//     };
    
//     gpio_config_t input_conf = {
//         .intr_type    = GPIO_INTR_DISABLE,
//         .mode         = GPIO_MODE_INPUT,
//         .pull_down_en = GPIO_PULLDOWN_DISABLE,
//         .pull_up_en   = GPIO_PULLUP_DISABLE,  
//         .pin_bit_mask = 0,
//     };

//     for (int i = 0; i < sizeof(input_gpios) / sizeof(input_gpios[0]); i++) {
//         input_conf.pin_bit_mask |= (1ULL << input_gpios[i]);
//     }

//     if (input_conf.pin_bit_mask != 0) {
//         ESP_ERROR_CHECK(gpio_config(&input_conf));
//         ESP_LOGI(TAG, "特殊功能引脚已初始化为输入模式（带内部上拉），共 %d 个", 
//                  (int)(sizeof(input_gpios) / sizeof(input_gpios[0])));
//     }
// }

// /**
//  * @brief 所有测试引脚输出高电平（置1）
//  */
// void gpio_drive_test_all_high(void)
// {
//     for (int i = 0; i < TEST_GPIO_COUNT; i++) {
//         gpio_set_level(test_gpios[i], 1);
//     }
//     ESP_LOGI(TAG, "所有测试引脚已置高（1）");
// }

// /**
//  * @brief 所有测试引脚输出低电平（置0）
//  */
// void gpio_drive_test_all_low(void)
// {
//     for (int i = 0; i < TEST_GPIO_COUNT; i++) {
//         gpio_set_level(test_gpios[i], 0);
//     }
//     ESP_LOGI(TAG, "所有测试引脚已置低（0）");
// }

// /**
//  * @brief GPIO 驱动测试任务
//  *        交替输出高低电平，每次间隔 1 秒
//  */
// void gpio_drive_test_task(void *pvParameters)
// {
//     gpio_drive_test_init();
//     // gpio_special_pins_input_init();  // 初始化特殊引脚为输入模式

//     ESP_LOGI(TAG, "========================================");
//     ESP_LOGI(TAG, "GPIO 输出驱动能力测试开始");
//     ESP_LOGI(TAG, "========================================");
//     gpio_drive_test_all_high();
//     vTaskDelay(pdMS_TO_TICKS(1000));
//     while (1) {
//         // gpio_drive_test_all_high();
//         // vTaskDelay(pdMS_TO_TICKS(1000));

//         // gpio_drive_test_all_low();
//         // vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }

// // ==================== app_main ====================

// void app_main(void)
// {
//     ESP_LOGI(TAG, "ESP32-S3 GPIO 驱动能力测试启动");

//     xTaskCreate(gpio_drive_test_task, "gpio_drive_test",
//                 configMINIMAL_STACK_SIZE * 2, NULL, 5, NULL);
//     while (1)
//     {
//         vTaskDelay(pdMS_TO_TICKS(1000)); // 添加延迟以避免阻塞Watchdog
//     }
// }
