/**
 * @file vl53l0_i2c.c
 * @brief VL53L0X激光测距传感器的软件I2C驱动
 * 
 * 使用GPIO模拟I2C协议，用于VL53L0X传感器通信
 */

#include "vl53l0_i2c.h"
#include "pin_definitions.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "VL53L0_I2C";

/**
 * @brief 初始化VL53L0X的GPIO引脚
 * 
 * 配置SCL和SDA引脚为开漏输出模式，适用于软件I2C通信
 */
void vl53l0_i2c_init(void)
{
    gpio_config_t io_conf;
    
    // 配置SCL和SDA为开漏输出模式
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT_OD;  // 开漏输出模式
    io_conf.pin_bit_mask = (1ULL << VL53L0X_SCL_GPIO) | (1ULL << VL53L0X_SDA_GPIO);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;  // 使能内部上拉电阻
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "VL53L0X I2C初始化完成: SCL=GPIO%d, SDA=GPIO%d", 
             VL53L0X_SCL_GPIO, VL53L0X_SDA_GPIO);
}

/**
 * @brief 将SDA引脚设置为输入模式
 * 
 * 用于接收来自I2C设备的数据
 */
void vl53l0_sda_set_input(void)
{
    gpio_config_t io_conf;
    
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << VL53L0X_SDA_GPIO);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
}

/**
 * @brief 将SDA引脚设置为输出模式
 * 
 * 用于向I2C设备发送数据
 */
void vl53l0_sda_set_output(void)
{
    gpio_config_t io_conf;
    
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT_OD;
    io_conf.pin_bit_mask = (1ULL << VL53L0X_SDA_GPIO);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
}

/**
 * @brief 设置SCL引脚为高电平
 */
void vl53l0_scl_high(void)
{
    gpio_set_level(VL53L0X_SCL_GPIO, 1);
}

/**
 * @brief 设置SCL引脚为低电平
 */
void vl53l0_scl_low(void)
{
    gpio_set_level(VL53L0X_SCL_GPIO, 0);
}

/**
 * @brief 设置SDA引脚为高电平
 */
void vl53l0_sda_high(void)
{
    gpio_set_level(VL53L0X_SDA_GPIO, 1);
}

/**
 * @brief 设置SDA引脚为低电平
 */
void vl53l0_sda_low(void)
{
    gpio_set_level(VL53L0X_SDA_GPIO, 0);
}

/**
 * @brief 读取SDA引脚电平
 * 
 * @return 1=高电平，0=低电平
 */
uint8_t vl53l0_sda_read(void)
{
    return gpio_get_level(VL53L0X_SDA_GPIO);
}
