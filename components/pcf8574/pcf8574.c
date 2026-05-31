/**
 * @file pcf8574.c
 * @brief PCF8574 I2C GPIO 扩展芯片驱动实现
 * 
 * 使用 ESP-IDF 5.3+ 的新 I2C master 驱动
 * 兼容 managed_components 中的 API 接口
 */

#include "pcf8574.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "PCF8574";

// 默认 I2C 频率 (400kHz)
#define I2C_FREQ_HZ 400000

// I2C 总线管理（避免重复初始化）
typedef struct {
    i2c_port_t port;
    gpio_num_t sda_io_num;
    gpio_num_t scl_io_num;
    i2c_master_bus_handle_t bus_handle;
    int ref_count;  // 引用计数
} i2c_bus_info_t;

#define MAX_I2C_BUSES 2
static i2c_bus_info_t i2c_buses[MAX_I2C_BUSES] = {0};

/**
 * @brief 获取或创建 I2C 总线
 */
static esp_err_t get_or_create_i2c_bus(i2c_port_t port, gpio_num_t sda_gpio, 
                                       gpio_num_t scl_gpio, 
                                       i2c_master_bus_handle_t *bus_handle)
{
    // 查找是否已存在相同配置的总线
    for (int i = 0; i < MAX_I2C_BUSES; i++) {
        if (i2c_buses[i].ref_count > 0 &&
            i2c_buses[i].port == port &&
            i2c_buses[i].sda_io_num == sda_gpio &&
            i2c_buses[i].scl_io_num == scl_gpio) {
            // 找到已存在的总线，增加引用计数
            i2c_buses[i].ref_count++;
            *bus_handle = i2c_buses[i].bus_handle;
            ESP_LOGI(TAG, "复用已存在的 I2C 总线 (引用计数: %d)", i2c_buses[i].ref_count);
            return ESP_OK;
        }
    }

    // 查找空闲槽位
    int free_slot = -1;
    for (int i = 0; i < MAX_I2C_BUSES; i++) {
        if (i2c_buses[i].ref_count == 0) {
            free_slot = i;
            break;
        }
    }

    if (free_slot == -1) {
        ESP_LOGE(TAG, "I2C 总线槽位已满");
        return ESP_ERR_NO_MEM;
    }

    // 创建新的 I2C 总线
    i2c_master_bus_config_t bus_config = {
        .i2c_port = port,
        .sda_io_num = sda_gpio,
        .scl_io_num = scl_gpio,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t ret = i2c_new_master_bus(&bus_config, bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C 总线初始化失败: %s", esp_err_to_name(ret));
        return ret;
    }

    // 保存总线信息
    i2c_buses[free_slot].port = port;
    i2c_buses[free_slot].sda_io_num = sda_gpio;
    i2c_buses[free_slot].scl_io_num = scl_gpio;
    i2c_buses[free_slot].bus_handle = *bus_handle;
    i2c_buses[free_slot].ref_count = 1;

    ESP_LOGI(TAG, "创建新的 I2C 总线 (槽位: %d)", free_slot);
    return ESP_OK;
}

/**
 * @brief 释放 I2C 总线引用
 */
static esp_err_t release_i2c_bus(i2c_master_bus_handle_t bus_handle)
{
    for (int i = 0; i < MAX_I2C_BUSES; i++) {
        if (i2c_buses[i].bus_handle == bus_handle && i2c_buses[i].ref_count > 0) {
            i2c_buses[i].ref_count--;
            ESP_LOGI(TAG, "释放 I2C 总线引用 (剩余引用: %d)", i2c_buses[i].ref_count);
            
            if (i2c_buses[i].ref_count == 0) {
                // 没有引用了，删除总线
                i2c_del_master_bus(bus_handle);
                i2c_buses[i].bus_handle = NULL;
                ESP_LOGI(TAG, "删除 I2C 总线 (槽位: %d)", i);
            }
            return ESP_OK;
        }
    }
    return ESP_ERR_NOT_FOUND;
}

/**
 * @brief 初始化 PCF8574 设备描述符
 */
esp_err_t pcf8574_init_desc(i2c_dev_t *dev, uint8_t addr, i2c_port_t port, 
                            gpio_num_t sda_gpio, gpio_num_t scl_gpio)
{
    if (dev == NULL) {
        ESP_LOGE(TAG, "设备描述符为空");
        return ESP_ERR_INVALID_ARG;
    }

    // PCF8574 有效地址范围：0x20-0x27（A2-A0 配置）
    // PCF8574A 有效地址范围：0x38-0x3F（A2-A0 配置）
    // 但在扫描模式下，允许所有 7 位地址
    if (addr > 0x7F) {
        ESP_LOGE(TAG, "无效的 I2C 地址: 0x%02X (超出 7 位范围)", addr);
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "初始化 PCF8574 设备描述符");
    ESP_LOGI(TAG, "  地址: 0x%02X", addr);
    ESP_LOGI(TAG, "  端口: %d", port);
    ESP_LOGI(TAG, "  SDA: GPIO%d, SCL: GPIO%d", sda_gpio, scl_gpio);

    // 保存配置参数
    dev->addr = addr;
    dev->port = port;
    dev->sda_io_num = sda_gpio;
    dev->scl_io_num = scl_gpio;
    dev->bus_handle = NULL;
    dev->dev_handle = NULL;

    // 获取或创建 I2C 总线
    esp_err_t ret = get_or_create_i2c_bus(port, sda_gpio, scl_gpio, &dev->bus_handle);
    if (ret != ESP_OK) {
        return ret;
    }

    // 配置 I2C 设备
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = I2C_FREQ_HZ,
    };

    ret = i2c_master_bus_add_device(dev->bus_handle, &dev_config, &dev->dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "添加 I2C 设备失败: %s", esp_err_to_name(ret));
        release_i2c_bus(dev->bus_handle);
        dev->bus_handle = NULL;
        return ret;
    }

    ESP_LOGI(TAG, "PCF8574 设备描述符初始化成功");
    return ESP_OK;
}

/**
 * @brief 释放 PCF8574 设备描述符
 */
esp_err_t pcf8574_free_desc(i2c_dev_t *dev)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "释放 PCF8574 设备描述符");

    // 删除设备
    if (dev->dev_handle != NULL) {
        i2c_master_bus_rm_device(dev->dev_handle);
        dev->dev_handle = NULL;
    }

    // 释放总线引用（如果引用计数为 0 会自动删除总线）
    if (dev->bus_handle != NULL) {
        release_i2c_bus(dev->bus_handle);
        dev->bus_handle = NULL;
    }

    return ESP_OK;
}

/**
 * @brief 读取 GPIO 端口值
 */
esp_err_t pcf8574_port_read(i2c_dev_t *dev, uint8_t *val)
{
    if (dev == NULL || val == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (dev->dev_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = i2c_master_receive(dev->dev_handle, val, 1, -1);
    // 不打印错误日志，让调用者决定是否需要记录
    return ret;
}

/**
 * @brief 写入 GPIO 端口值
 */
esp_err_t pcf8574_port_write(i2c_dev_t *dev, uint8_t value)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (dev->dev_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = i2c_master_transmit(dev->dev_handle, &value, 1, -1);
    // 不打印错误日志，让调用者决定是否需要记录
    return ret;
}

/**
 * @brief 读取单个引脚电平
 */
esp_err_t pcf8574_get_level(i2c_dev_t *dev, uint8_t pin, uint32_t *val)
{
    if (dev == NULL || val == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (pin > 7) {
        ESP_LOGE(TAG, "无效的引脚号: %d", pin);
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t raw;
    esp_err_t ret = pcf8574_port_read(dev, &raw);
    if (ret == ESP_OK) {
        *val = (raw >> pin) & 1;
    }

    return ret;
}

/**
 * @brief 设置单个引脚电平
 */
esp_err_t pcf8574_set_level(i2c_dev_t *dev, uint8_t pin, uint32_t val)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (pin > 7) {
        ESP_LOGE(TAG, "无效的引脚号: %d", pin);
        return ESP_ERR_INVALID_ARG;
    }

    // 读取当前状态
    uint8_t raw;
    esp_err_t ret = pcf8574_port_read(dev, &raw);
    if (ret != ESP_OK) {
        return ret;
    }

    // 修改指定引脚
    raw &= ~(1 << pin) | (val ? (((uint8_t)val & 1) << pin) : 0);

    // 写回
    return pcf8574_port_write(dev, raw);
}
