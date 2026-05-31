/**
 * @file mpu_i2c.c
 * @brief MPU6050陀螺仪的硬件I2C驱动
 * 
 * 使用ESP32的硬件I2C主控制器与MPU6050通信
 */

#include "mpu_i2c.h"
#include "pin_definitions.h"
#include "esp_log.h"

// MPU6050专用的I2C句柄
static i2c_master_bus_handle_t mpu_bus_handle = NULL;
static i2c_master_dev_handle_t mpu_dev_handle = NULL;

static const char *TAG = "MPU_I2C";

/**
 * @brief MPU6050的I2C总线初始化函数
 * 
 * @return ESP_OK表示初始化成功，其他值表示失败
 */
esp_err_t mpu_i2c_init(void)
{
    esp_err_t err;

    // 配置I2C主总线参数（使用pin_definitions.h中的定义）
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_PORT,
        .scl_io_num = I2C_MASTER_SCL_GPIO,
        .sda_io_num = I2C_MASTER_SDA_GPIO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    
    // 创建I2C主总线实例
    err = i2c_new_master_bus(&i2c_mst_config, &mpu_bus_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C总线初始化失败: %s", esp_err_to_name(err));
        return err;
    }
    
    // 配置MPU6050设备参数
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = MPU6050_I2C_ADDR,
        .scl_speed_hz = MPU6050_SCL_SPEED,
        .scl_wait_us = 10,
    };
    
    // 将MPU6050添加到I2C总线上
    err = i2c_master_bus_add_device(mpu_bus_handle, &dev_cfg, &mpu_dev_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "添加MPU6050设备失败: %s", esp_err_to_name(err));
        return err;
    }
    
    ESP_LOGI(TAG, "MPU6050 I2C初始化成功: SCL=GPIO%d, SDA=GPIO%d, 地址=0x%02X", 
             I2C_MASTER_SCL_GPIO, I2C_MASTER_SDA_GPIO, MPU6050_I2C_ADDR);
    return ESP_OK;
}

/**
 * @brief 向MPU6050的指定寄存器写入单个字节数据
 * 
 * @param reg 目标寄存器地址
 * @param byte 要写入的数据字节
 * @return esp_err_t 写入操作的状态码
 */
esp_err_t mpu_write_byte(uint8_t reg, uint8_t byte)
{
    uint8_t data[2] = {reg, byte};
    esp_err_t err = i2c_master_transmit(mpu_dev_handle, data, 2, -1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "写入寄存器0x%02X失败: %s", reg, esp_err_to_name(err));
    }
    return err;
}

/**
 * @brief 向MPU6050的指定寄存器写入多个字节数据
 * 
 * @param reg 起始寄存器地址
 * @param write_len 要写入的数据长度
 * @param buf 包含待写入数据的缓冲区指针
 * @return esp_err_t 写入操作的状态码
 */
esp_err_t mpu_write_buf(uint8_t reg, uint16_t write_len, uint8_t *buf)
{
    // 创建发送缓冲区
    uint8_t write_buf[write_len + 1];
    write_buf[0] = reg;
    
    // 复制数据
    for (uint16_t i = 0; i < write_len; i++) {
        write_buf[i + 1] = buf[i];
    }
    
    // 执行I2C传输
    esp_err_t err = i2c_master_transmit(mpu_dev_handle, write_buf, write_len + 1, -1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "批量写入寄存器0x%02X失败: %s", reg, esp_err_to_name(err));
    }
    return err;
}

/**
 * @brief 从MPU6050的指定寄存器读取单个字节数据
 * 
 * @param reg 要读取的寄存器地址
 * @return 读取到的数据字节，如果读取失败则返回0
 */
uint8_t mpu_read_byte(uint8_t reg)
{
    uint8_t read_byte = 0;
    
    esp_err_t err = i2c_master_transmit_receive(mpu_dev_handle, &reg, 1, &read_byte, 1, -1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "读取寄存器0x%02X失败: %s", reg, esp_err_to_name(err));
        return 0;
    }
    return read_byte;
}

/**
 * @brief 从MPU6050的指定寄存器开始读取多个字节数据
 * 
 * @param reg 起始寄存器地址
 * @param read_len 要读取的数据长度
 * @param read_buf 用于存储读取数据的缓冲区指针
 * @return esp_err_t 读取操作的状态码
 */
esp_err_t mpu_read_buf(uint8_t reg, uint16_t read_len, uint8_t *read_buf)
{
    esp_err_t err = i2c_master_transmit_receive(mpu_dev_handle, &reg, 1, read_buf, read_len, -1);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "批量读取寄存器0x%02X失败: %s", reg, esp_err_to_name(err));
    }
    return err;
}
