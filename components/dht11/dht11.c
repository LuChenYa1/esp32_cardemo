/**
 * @file dht11.c
 * @brief DHT11 温湿度传感器驱动实现
 * 
 * 参考 esp-idf-lib 的 dht 组件，简化为仅支持 DHT11
 */

#include "dht11.h"
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <esp_log.h>
#include <esp_rom_sys.h>

static const char *TAG = "DHT11";

// DHT 定时器精度（微秒）
#define DHT_TIMER_INTERVAL 2
#define DHT_DATA_BITS 40
#define DHT_DATA_BYTES (DHT_DATA_BITS / 8)

// 临界区保护宏
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
#define PORT_ENTER_CRITICAL() portENTER_CRITICAL(&mux)
#define PORT_EXIT_CRITICAL() portEXIT_CRITICAL(&mux)

// 参数检查宏
#define CHECK_ARG(VAL) do { if (!(VAL)) return ESP_ERR_INVALID_ARG; } while (0)

// 错误检查宏（带临界区退出）
#define CHECK_LOGE(x, msg, ...) do { \
        esp_err_t __err; \
        if ((__err = x) != ESP_OK) { \
            PORT_EXIT_CRITICAL(); \
            ESP_LOGE(TAG, msg, ## __VA_ARGS__); \
            return __err; \
        } \
    } while (0)

/**
 * @brief 等待引脚变为指定状态
 * 
 * @param pin GPIO 引脚
 * @param timeout 超时时间（微秒）
 * @param expected_pin_state 期望的引脚电平（0 或 1）
 * @param duration 输出参数，记录等待时长（微秒），可为 NULL
 * @return 
 *     - ESP_OK 成功
 *     - ESP_ERR_TIMEOUT 超时
 */
static esp_err_t dht_await_pin_state(gpio_num_t pin, uint32_t timeout,
                                     int expected_pin_state, uint32_t *duration)
{
    gpio_set_direction(pin, GPIO_MODE_INPUT);
    
    for (uint32_t i = 0; i < timeout; i += DHT_TIMER_INTERVAL)
    {
        // 至少等待一个时间间隔，防止读取到抖动
        esp_rom_delay_us(DHT_TIMER_INTERVAL);
        
        if (gpio_get_level(pin) == expected_pin_state)
        {
            if (duration)
                *duration = i;
            return ESP_OK;
        }
    }

    return ESP_ERR_TIMEOUT;
}

/**
 * @brief 从 DHT11 读取原始数据
 * 
 * 此函数必须在临界区中调用，以保证时序准确性
 * 
 * @param pin GPIO 引脚
 * @param data 5 字节数据缓冲区
 * @return 
 *     - ESP_OK 成功
 *     - ESP_ERR_TIMEOUT 通信超时
 */
static inline esp_err_t dht_fetch_data(gpio_num_t pin, uint8_t data[DHT_DATA_BYTES])
{
    uint32_t low_duration;
    uint32_t high_duration;

    // 阶段 A：拉低信号至少 18ms 以启动读取序列
    gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(pin, 0);
    esp_rom_delay_us(20000);  // DHT11 需要至少 18ms
    gpio_set_level(pin, 1);

    // 阶段 B：等待 DHT11 响应（拉低），最多 40us
    CHECK_LOGE(dht_await_pin_state(pin, 40, 0, NULL),
               "初始化错误，阶段 B 失败");
    
    // 阶段 C：DHT11 拉低约 80us
    CHECK_LOGE(dht_await_pin_state(pin, 88, 1, NULL),
               "初始化错误，阶段 C 失败");
    
    // 阶段 D：DHT11 拉高约 80us
    CHECK_LOGE(dht_await_pin_state(pin, 88, 0, NULL),
               "初始化错误，阶段 D 失败");

    // 读取 40 位数据
    for (int i = 0; i < DHT_DATA_BITS; i++)
    {
        // 等待低电平结束（每位起始信号约 50us）
        CHECK_LOGE(dht_await_pin_state(pin, 65, 1, &low_duration),
                   "低电平超时");
        
        // 等待高电平结束（数据位：26-28us=0，70us=1）
        CHECK_LOGE(dht_await_pin_state(pin, 75, 0, &high_duration),
                   "高电平超时");

        uint8_t b = i / 8;
        uint8_t m = i % 8;
        if (!m)
            data[b] = 0;

        // 高电平持续时间长于低电平 = 逻辑 1
        data[b] |= (high_duration > low_duration) << (7 - m);
    }

    return ESP_OK;
}

esp_err_t dht11_read_data(gpio_num_t pin, int16_t *humidity, int16_t *temperature)
{
    CHECK_ARG(humidity || temperature);

    uint8_t data[DHT_DATA_BYTES] = { 0 };

    // 初始化引脚为开漏输出，拉高
    gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(pin, 1);

    // 进入临界区，禁止任务切换
    PORT_ENTER_CRITICAL();
    esp_err_t result = dht_fetch_data(pin, data);
    if (result == ESP_OK)
        PORT_EXIT_CRITICAL();

    // 恢复 GPIO 方向
    gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(pin, 1);

    if (result != ESP_OK)
        return result;

    // 校验和检查
    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF))
    {
        ESP_LOGE(TAG, "校验和错误，接收到无效数据");
        return ESP_ERR_INVALID_CRC;
    }

    // DHT11 数据格式：
    // data[0] = 湿度整数部分
    // data[1] = 湿度小数部分（DHT11 固定为 0）
    // data[2] = 温度整数部分
    // data[3] = 温度小数部分（DHT11 固定为 0）
    // data[4] = 校验和
    
    if (humidity)
        *humidity = data[0] * 10;  // 转换为 0.1% 单位
    
    if (temperature)
        *temperature = data[2] * 10;  // 转换为 0.1°C 单位

    ESP_LOGD(TAG, "传感器数据：湿度=%d (0.1%%), 温度=%d (0.1°C)", 
             humidity ? *humidity : 0, temperature ? *temperature : 0);

    return ESP_OK;
}

esp_err_t dht11_read_float_data(gpio_num_t pin, float *humidity, float *temperature)
{
    CHECK_ARG(humidity || temperature);

    int16_t i_humidity, i_temp;

    esp_err_t res = dht11_read_data(pin, 
                                    humidity ? &i_humidity : NULL, 
                                    temperature ? &i_temp : NULL);
    if (res != ESP_OK)
        return res;

    if (humidity)
        *humidity = i_humidity / 10.0;
    
    if (temperature)
        *temperature = i_temp / 10.0;

    return ESP_OK;
}
