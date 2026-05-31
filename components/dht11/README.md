# DHT11温湿度传感器 (DHT11 Temperature & Humidity Sensor)

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: 未使用

---

## 组件简介

DHT11温湿度传感器组件用于测量环境温度和相对湿度。DHT11是一款低成本的数字温湿度传感器，采用单总线通信协议，只需一根数据线即可完成数据传输。

本组件基于ESP-IDF实现，参考了esp-idf-lib的dht组件，简化为仅支持DHT11型号。组件提供整数和浮点两种数据格式，满足不同应用需求。

### 主要特性

- 单总线数字信号输出，只需一根数据线
- 温度测量范围：0-50°C，精度±2°C
- 湿度测量范围：20-90%RH，精度±5%RH
- 支持整数格式（0.1精度）和浮点格式输出
- 自动校验和验证，确保数据可靠性
- 临界区保护，保证时序准确性
- 开漏输出模式，支持长线传输

### 适用场景

- 环境监测系统
- 智能家居温湿度控制
- 气象站数据采集
- 温室大棚监控
- 仓储环境监测

---

## 硬件连接

### 引脚分配

| 功能 | GPIO引脚 | 接口标识 | 说明 |
|------|---------|---------|------|
| DHT11数据线 | GPIO38 | SSA1 | 单总线数字信号，开漏输出 |

### 接线说明

1. 将DHT11的数据引脚（DATA）连接到GPIO38
2. 将DHT11的VCC连接到3.3V或5V电源（DHT11支持3.3-5.5V）
3. 将DHT11的GND与ESP32共地
4. 在数据线和VCC之间连接一个4.7kΩ-10kΩ的上拉电阻（部分DHT11模块已内置）

### 接线图

```
DHT11模块          ESP32-S3
  VCC  ----------- 3.3V/5V
  DATA ----------- GPIO38 (SSA1)
  GND  ----------- GND
  
  上拉电阻（4.7kΩ-10kΩ）
  DATA ----/\/\/\---- VCC
```

### 注意事项

- ⚠️ **上拉电阻必需**：DHT11使用开漏输出，必须外接上拉电阻，否则无法正常通信
- ⚠️ **读取间隔**：DHT11响应速度较慢，两次读取之间至少间隔2秒，否则可能读取失败
- ⚠️ **GPIO38限制**：GPIO38是仅输入引脚，但DHT11需要双向通信，使用开漏模式可以实现
- ⚠️ **线缆长度**：数据线长度不宜超过20米，过长会导致信号衰减和通信失败

---

## 功能说明

### 核心功能

#### 功能1：单总线通信协议

DHT11采用单总线通信协议，通过一根数据线完成双向通信。通信过程包括：
1. 主机发送起始信号（拉低至少18ms）
2. DHT11响应信号（拉低80us，拉高80us）
3. DHT11发送40位数据（5字节）
4. 数据包含湿度、温度和校验和

#### 功能2：数据格式转换

组件提供两种数据格式：
- **整数格式**：以0.1为单位，例如625表示62.5%湿度，244表示24.4°C温度
- **浮点格式**：直接返回浮点数，例如62.5%和24.4°C

#### 功能3：校验和验证

DHT11发送的数据包含校验和字节，组件自动验证数据完整性。如果校验失败，返回`ESP_ERR_INVALID_CRC`错误。

#### 功能4：临界区保护

DHT11的时序要求严格（微秒级），组件在读取数据时进入临界区，禁止任务切换，确保时序准确性。

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| GPIO引脚 | GPIO38 | GPIO0-48 | DHT11数据线引脚，定义在pin_definitions.h |
| 读取间隔 | 2秒 | ≥2秒 | 两次读取之间的最小间隔 |
| 起始信号时长 | 20ms | ≥18ms | 主机拉低信号的时长 |
| 超时时间 | 40-88us | - | 各阶段的超时时间 |

---

## API接口

### 数据读取函数（整数格式）

```c
/**
 * @brief 读取 DHT11 传感器数据（整数格式）
 * 
 * 湿度和温度以整数形式返回，单位为 0.1
 * 例如：humidity=625 表示 62.5%，temperature=244 表示 24.4°C
 * 
 * @param pin GPIO 引脚编号
 * @param[out] humidity 湿度值（单位：0.1%），可为 NULL
 * @param[out] temperature 温度值（单位：0.1°C），可为 NULL
 * @return 
 *     - ESP_OK 成功
 *     - ESP_ERR_INVALID_ARG 参数错误
 *     - ESP_ERR_TIMEOUT 通信超时
 *     - ESP_ERR_INVALID_CRC 校验和错误
 */
esp_err_t dht11_read_data(gpio_num_t pin, int16_t *humidity, int16_t *temperature);
```

**参数说明**：
- `pin`: GPIO引脚编号，通常使用`DHT11_DATA_GPIO`宏
- `humidity`: 指向int16_t的指针，用于接收湿度值（单位：0.1%），可传NULL
- `temperature`: 指向int16_t的指针，用于接收温度值（单位：0.1°C），可传NULL

**返回值**：
- `ESP_OK`: 读取成功
- `ESP_ERR_INVALID_ARG`: 参数错误（humidity和temperature都为NULL）
- `ESP_ERR_TIMEOUT`: 通信超时（传感器无响应或时序错误）
- `ESP_ERR_INVALID_CRC`: 校验和错误（数据传输错误）

**使用说明**：
此函数会阻塞执行约20ms，不应在中断或高优先级任务中调用。两次调用之间至少间隔2秒。

---

### 数据读取函数（浮点格式）

```c
/**
 * @brief 读取 DHT11 传感器数据（浮点格式）
 * 
 * 湿度和温度以浮点数形式返回
 * 
 * @param pin GPIO 引脚编号
 * @param[out] humidity 湿度值（单位：%），可为 NULL
 * @param[out] temperature 温度值（单位：°C），可为 NULL
 * @return 
 *     - ESP_OK 成功
 *     - ESP_ERR_INVALID_ARG 参数错误
 *     - ESP_ERR_TIMEOUT 通信超时
 *     - ESP_ERR_INVALID_CRC 校验和错误
 */
esp_err_t dht11_read_float_data(gpio_num_t pin, float *humidity, float *temperature);
```

**参数说明**：
- `pin`: GPIO引脚编号，通常使用`DHT11_DATA_GPIO`宏
- `humidity`: 指向float的指针，用于接收湿度值（单位：%），可传NULL
- `temperature`: 指向float的指针，用于接收温度值（单位：°C），可传NULL

**返回值**：
- 同`dht11_read_data()`

**使用说明**：
此函数内部调用`dht11_read_data()`并转换为浮点格式，使用限制相同。

---

## 使用示例

### 基本使用示例（整数格式）

```c
#include "dht11.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "EXAMPLE";

void example_basic_usage(void)
{
    int16_t humidity, temperature;
    
    // 读取DHT11数据（整数格式）
    esp_err_t ret = dht11_read_data(DHT11_DATA_GPIO, &humidity, &temperature);
    
    if (ret == ESP_OK) {
        // 转换为实际值（除以10）
        float hum = humidity / 10.0;
        float temp = temperature / 10.0;
        ESP_LOGI(TAG, "温度: %.1f°C, 湿度: %.1f%%", temp, hum);
    } else if (ret == ESP_ERR_TIMEOUT) {
        ESP_LOGE(TAG, "DHT11通信超时，请检查接线");
    } else if (ret == ESP_ERR_INVALID_CRC) {
        ESP_LOGE(TAG, "DHT11数据校验失败，请重试");
    } else {
        ESP_LOGE(TAG, "DHT11读取失败: %s", esp_err_to_name(ret));
    }
}
```

### 基本使用示例（浮点格式）

```c
#include "dht11.h"
#include "esp_log.h"

static const char *TAG = "EXAMPLE";

void example_float_format(void)
{
    float humidity, temperature;
    
    // 读取DHT11数据（浮点格式）
    esp_err_t ret = dht11_read_float_data(DHT11_DATA_GPIO, &humidity, &temperature);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "温度: %.1f°C, 湿度: %.1f%%", temperature, humidity);
    } else {
        ESP_LOGE(TAG, "DHT11读取失败: %s", esp_err_to_name(ret));
    }
}
```

### 周期性读取示例

```c
#include "dht11.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "DHT11_TASK";

void dht11_monitor_task(void *pvParameters)
{
    float humidity, temperature;
    
    while (1) {
        // 读取DHT11数据
        esp_err_t ret = dht11_read_float_data(DHT11_DATA_GPIO, &humidity, &temperature);
        
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "温度: %.1f°C, 湿度: %.1f%%", temperature, humidity);
            
            // 根据温湿度执行控制逻辑
            if (temperature > 30.0) {
                ESP_LOGW(TAG, "温度过高，启动降温");
                // 控制风扇或空调
            }
            
            if (humidity < 30.0) {
                ESP_LOGW(TAG, "湿度过低，启动加湿");
                // 控制加湿器
            }
        } else {
            ESP_LOGE(TAG, "DHT11读取失败: %s", esp_err_to_name(ret));
        }
        
        // DHT11要求至少2秒读取间隔
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void example_periodic_read(void)
{
    xTaskCreate(dht11_monitor_task, "dht11_monitor", 2048, NULL, 5, NULL);
}
```

### 仅读取温度或湿度

```c
#include "dht11.h"
#include "esp_log.h"

static const char *TAG = "EXAMPLE";

void example_read_temperature_only(void)
{
    float temperature;
    
    // 只读取温度，湿度参数传NULL
    esp_err_t ret = dht11_read_float_data(DHT11_DATA_GPIO, NULL, &temperature);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "当前温度: %.1f°C", temperature);
    }
}

void example_read_humidity_only(void)
{
    float humidity;
    
    // 只读取湿度，温度参数传NULL
    esp_err_t ret = dht11_read_float_data(DHT11_DATA_GPIO, &humidity, NULL);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "当前湿度: %.1f%%", humidity);
    }
}
```

---

## 注意事项

### 硬件限制

- ⚠️ **上拉电阻必需**：DHT11使用开漏输出，必须在数据线和VCC之间连接4.7kΩ-10kΩ上拉电阻
- ⚠️ **电源电压**：DHT11支持3.3-5.5V电源，但建议使用5V以获得更好的稳定性
- ⚠️ **测量范围**：温度0-50°C，湿度20-90%RH，超出范围测量不准确
- ⚠️ **测量精度**：温度±2°C，湿度±5%RH，不适合高精度应用

### 软件限制

- ⚠️ **读取间隔**：两次读取之间必须间隔至少2秒，否则DHT11无法响应
- ⚠️ **阻塞执行**：读取函数会阻塞约20ms，不应在中断或高优先级任务中调用
- ⚠️ **临界区**：读取时会进入临界区禁止任务切换，可能影响实时性要求高的任务
- ⚠️ **错误率**：DHT11通信可靠性一般，建议添加重试机制

### 线程安全

- ❌ DHT11读取函数不是线程安全的，不应在多个任务中同时调用
- ❌ 读取时会进入临界区，禁止任务切换，可能导致其他任务延迟

### 性能考虑

- 读取一次数据需要约20ms，频繁读取会占用较多CPU时间
- 建议读取间隔为2-5秒，平衡数据实时性和系统负载
- 如需更高频率的温湿度监测，建议使用DHT22或SHT3x等更快速的传感器

---

## 故障排除

### 常见问题

#### 问题1：读取超时（ESP_ERR_TIMEOUT）

**现象**：调用`dht11_read_data()`返回`ESP_ERR_TIMEOUT`

**原因**：
1. DHT11未正确连接或电源未接通
2. 缺少上拉电阻或上拉电阻阻值不合适
3. 数据线过长导致信号衰减
4. DHT11损坏

**解决方案**：
1. 检查DHT11的VCC、GND和DATA连接
2. 确认数据线和VCC之间有4.7kΩ-10kΩ上拉电阻
3. 缩短数据线长度（建议<20cm）
4. 使用万用表测量DHT11的VCC电压，应为3.3V或5V
5. 更换DHT11传感器测试

#### 问题2：校验和错误（ESP_ERR_INVALID_CRC）

**现象**：调用`dht11_read_data()`返回`ESP_ERR_INVALID_CRC`

**原因**：
1. 数据传输过程中受到干扰
2. 上拉电阻阻值不合适
3. 读取间隔过短
4. 电源不稳定

**解决方案**：
1. 远离强电磁干扰源（如电机、继电器）
2. 调整上拉电阻阻值（推荐4.7kΩ）
3. 确保两次读取间隔至少2秒
4. 在DHT11的VCC和GND之间添加0.1uF滤波电容
5. 添加重试机制，连续失败3次才报错

#### 问题3：读取值异常（全0或全1）

**现象**：读取的温湿度值为0或异常大

**原因**：
1. DHT11未正确初始化
2. GPIO配置错误
3. DHT11损坏

**解决方案**：
1. 确认GPIO引脚配置为开漏输出模式
2. 检查`pin_definitions.h`中的引脚定义是否正确
3. 使用示波器查看数据线波形，确认通信时序
4. 更换DHT11传感器测试

#### 问题4：读取间隔过短导致失败

**现象**：连续快速读取时，第二次读取失败

**原因**：DHT11需要时间恢复，两次读取间隔过短

**解决方案**：
1. 确保两次读取之间至少间隔2秒
2. 在应用层添加时间戳检查，防止过快读取
3. 使用FreeRTOS的`vTaskDelay()`确保间隔

```c
// 正确的读取方式
while (1) {
    dht11_read_float_data(DHT11_DATA_GPIO, &humidity, &temperature);
    vTaskDelay(pdMS_TO_TICKS(2000));  // 至少2秒间隔
}
```

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [GPIO引脚分配文档](../../docs/GPIO_PIN_ALLOCATION.md)
- [板级配置组件](../board_config/README.md)

### 数据手册

- DHT11数字温湿度传感器数据手册
- ESP32-S3技术参考手册 - GPIO章节
- esp-idf-lib DHT组件文档

### 代码示例

- 本组件暂未在main.c中使用，可参考本文档的使用示例

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，完整的API文档和使用说明 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/dht11/`  
**文档类型**: 组件使用说明
