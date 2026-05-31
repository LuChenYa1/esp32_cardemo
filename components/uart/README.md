# UART（串口通信驱动）

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: ⏸️ 未使用（被其他组件依赖）

---

## 组件简介

UART 是一个统一的串口通信驱动模块，提供 UART0/UART1 的完整功能接口，包括初始化、发送、接收、RS485 方向控制和互斥锁管理。该组件是其他通信组件（如 camera_protocol、voice_module、485servo）的基础依赖。

### 主要特性

- 支持 UART0 和 UART1 双串口
- RS485 方向自动控制
- 互斥锁保护，支持多任务安全访问
- 事件队列支持，实现中断接收模式
- 可配置波特率、缓冲区大小
- 完整的发送/接收 API

### 适用场景

- RS485 总线通信（摄像头、舵机）
- 串口外设通信（语音模块、蓝牙模块）
- 多任务环境下的串口共享
- 需要中断接收的高实时性应用

---

## 硬件连接

### 引脚分配

| 功能 | GPIO引脚 | 接口标识 | 说明 |
|------|---------|---------|------|
| UART0 TX | GPIO43 | - | 串口0发送（RS485） |
| UART0 RX | GPIO44 | - | 串口0接收（RS485） |
| RS485 方向控制 | GPIO19 | - | 控制RS485收发方向 |
| UART1 TX | GPIO17 | - | 串口1发送 |
| UART1 RX | GPIO18 | - | 串口1接收 |

### 接线说明

**UART0（RS485模式）：**
1. 将 GPIO43 连接到 RS485 转换器的 TXD
2. 将 GPIO44 连接到 RS485 转换器的 RXD
3. 将 GPIO19 连接到 RS485 转换器的 DE/RE（方向控制）
4. RS485-A 和 RS485-B 连接到总线

**UART1（普通串口）：**
1. 将 GPIO17 连接到外设的 RX
2. 将 GPIO18 连接到外设的 TX
3. 确保共地（GND 连接）

### 注意事项

- ⚠️ UART0 被 RS485 设备共用（摄像头、舵机），必须使用互斥锁保护
- ⚠️ RS485 方向控制引脚（GPIO19）必须正确配置，否则无法通信
- ⚠️ UART1 可用于普通串口外设，不需要方向控制

---

## 功能说明

### 核心功能

#### 功能1：UART 初始化

支持 UART0 和 UART1 的独立初始化，可配置波特率、引脚、缓冲区大小。UART0 支持事件队列，用于中断接收模式。

#### 功能2：数据发送

提供同步阻塞式发送接口，支持任意长度数据发送。

#### 功能3：数据接收

提供带超时的接收接口，支持轮询和中断两种模式。

#### 功能4：RS485 方向控制

自动控制 RS485 收发方向，发送时切换到发送模式，发送完成后切换回接收模式。

#### 功能5：互斥锁保护

提供互斥锁机制，防止多任务同时访问 RS485 总线，确保通信可靠性。

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| UART0_PORT | UART_NUM_0 | UART_NUM_0/1/2 | UART0 端口号 |
| UART1_PORT | UART_NUM_1 | UART_NUM_0/1/2 | UART1 端口号 |
| UART0_RX_BUFFER_SIZE | 1024 | 128-4096 | UART0 接收缓冲区大小 |
| UART0_TX_BUFFER_SIZE | 1024 | 128-4096 | UART0 发送缓冲区大小 |
| UART1_RX_BUFFER_SIZE | 1024 | 128-4096 | UART1 接收缓冲区大小 |
| UART1_TX_BUFFER_SIZE | 1024 | 128-4096 | UART1 发送缓冲区大小 |
| RS485_DIR_TX_LEVEL | 1 | 0/1 | RS485 发送模式电平 |
| RS485_DIR_RX_LEVEL | 0 | 0/1 | RS485 接收模式电平 |

---

## API接口

### 初始化函数

```c
/**
 * @brief 初始化UART0
 * 
 * @param baud_rate 波特率（例如：115200, 1000000）
 */
void uart0_init(uint32_t baud_rate);

/**
 * @brief 初始化UART1
 * 
 * @param baud_rate 波特率（例如：115200, 9600）
 */
void uart1_init(uint32_t baud_rate);
```

**参数说明**：
- `baud_rate`: 波特率，常用值：9600, 115200, 1000000

**返回值**：
- 无

**使用说明**：
在使用 UART 功能前必须先调用初始化函数。UART0 会自动创建事件队列，支持中断接收。

---

### 发送函数

```c
/**
 * @brief 通过UART0发送数据
 * 
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际发送的字节数
 */
int uart0_send(const uint8_t *data, uint16_t len);

/**
 * @brief 通过UART1发送数据
 * 
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际发送的字节数
 */
int uart1_send(const uint8_t *data, uint16_t len);
```

**参数说明**：
- `data`: 要发送的数据指针
- `len`: 数据长度（字节）

**返回值**：
- 实际发送的字节数

**使用说明**：
同步阻塞发送，函数返回时数据已写入 UART 发送缓冲区。

---

### 接收函数

```c
/**
 * @brief 从UART0接收数据
 * 
 * @param data 接收缓冲区指针
 * @param len 最大接收长度
 * @param timeout 超时时间（FreeRTOS ticks）
 * @return 实际接收的字节数
 */
int uart0_recv(uint8_t *data, size_t len, TickType_t timeout);

/**
 * @brief 从UART1接收数据
 * 
 * @param data 接收缓冲区指针
 * @param len 最大接收长度
 * @param timeout 超时时间（FreeRTOS ticks）
 * @return 实际接收的字节数
 */
int uart1_recv(uint8_t *data, size_t len, TickType_t timeout);
```

**参数说明**：
- `data`: 接收缓冲区指针
- `len`: 最大接收长度（字节）
- `timeout`: 超时时间，使用 `pdMS_TO_TICKS(ms)` 转换

**返回值**：
- 实际接收的字节数，0 表示超时

**使用说明**：
阻塞接收，直到接收到数据或超时。

---

### RS485 控制函数

```c
/**
 * @brief 初始化RS485方向控制GPIO
 */
void rs485_init(void);

/**
 * @brief 设置RS485为发送模式
 */
void rs485_set_tx_mode(void);

/**
 * @brief 设置RS485为接收模式
 */
void rs485_set_rx_mode(void);

/**
 * @brief RS485发送数据（自动控制方向）
 * 
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际发送的字节数
 */
int rs485_send(const uint8_t *data, uint16_t len);

/**
 * @brief RS485发送数据（带互斥锁保护）
 * 
 * @param data 数据指针
 * @param len 数据长度
 * @param timeout_ms 获取互斥锁的超时时间（毫秒）
 * @return 实际发送的字节数，-1表示获取互斥锁失败
 */
int rs485_send_protected(const uint8_t *data, uint16_t len, uint32_t timeout_ms);
```

**参数说明**：
- `data`: 要发送的数据指针
- `len`: 数据长度（字节）
- `timeout_ms`: 互斥锁超时时间（毫秒）

**返回值**：
- 实际发送的字节数
- -1 表示获取互斥锁失败

**使用说明**：
- `rs485_send()`: 自动控制方向，适合单任务环境
- `rs485_send_protected()`: 带互斥锁保护，适合多任务环境（推荐）

---

### 互斥锁管理函数

```c
/**
 * @brief 初始化UART0 RS485互斥锁
 * 
 * @return 0=成功，-1=失败
 */
int uart0_rs485_mutex_init(void);

/**
 * @brief 获取UART0 RS485互斥锁句柄
 * 
 * @return 互斥锁句柄，如果未初始化则返回NULL
 */
SemaphoreHandle_t uart0_get_rs485_mutex(void);
```

**参数说明**：
- 无

**返回值**：
- `uart0_rs485_mutex_init()`: 0=成功，-1=失败
- `uart0_get_rs485_mutex()`: 互斥锁句柄

**使用说明**：
在多任务环境中使用 RS485 前，必须先调用 `uart0_rs485_mutex_init()` 初始化互斥锁。

---

### 事件队列函数

```c
/**
 * @brief 获取UART0事件队列句柄
 * 
 * @return 事件队列句柄，如果未初始化则返回NULL
 */
QueueHandle_t uart0_get_event_queue(void);
```

**参数说明**：
- 无

**返回值**：
- UART0 事件队列句柄

**使用说明**：
用于中断接收模式，可以监听 UART 事件（数据到达、FIFO 溢出等）。

---

### 反初始化函数

```c
/**
 * @brief 反初始化UART0
 */
void uart0_deinit(void);

/**
 * @brief 反初始化UART1
 */
void uart1_deinit(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
释放 UART 资源，通常在系统关闭或重新配置时调用。

---

## 使用示例

### 基本使用示例（UART1）

```c
#include "uart.h"
#include "esp_log.h"

static const char *TAG = "UART_EXAMPLE";

void uart1_example(void)
{
    // 1. 初始化UART1（波特率115200）
    uart1_init(115200);
    ESP_LOGI(TAG, "UART1初始化完成");
    
    // 2. 发送数据
    const char *msg = "Hello UART1\r\n";
    int sent = uart1_send((const uint8_t *)msg, strlen(msg));
    ESP_LOGI(TAG, "发送了 %d 字节", sent);
    
    // 3. 接收数据（超时1秒）
    uint8_t rx_buf[128];
    int received = uart1_recv(rx_buf, sizeof(rx_buf), pdMS_TO_TICKS(1000));
    if (received > 0) {
        ESP_LOGI(TAG, "接收了 %d 字节", received);
        ESP_LOG_BUFFER_HEXDUMP(TAG, rx_buf, received, ESP_LOG_INFO);
    } else {
        ESP_LOGI(TAG, "接收超时");
    }
}
```

### RS485 通信示例（单任务）

```c
void rs485_example_single_task(void)
{
    // 1. 初始化UART0
    uart0_init(1000000);  // 波特率1Mbps
    
    // 2. 初始化RS485方向控制
    rs485_init();
    
    // 3. 发送数据（自动控制方向）
    uint8_t tx_data[] = {0xFF, 0xFF, 0xA1, 0x01, 0x02, 0x03};
    int sent = rs485_send(tx_data, sizeof(tx_data));
    ESP_LOGI(TAG, "RS485发送了 %d 字节", sent);
    
    // 4. 接收数据
    uint8_t rx_buf[64];
    int received = uart0_recv(rx_buf, sizeof(rx_buf), pdMS_TO_TICKS(100));
    if (received > 0) {
        ESP_LOGI(TAG, "RS485接收了 %d 字节", received);
    }
}
```

### RS485 通信示例（多任务，推荐）

```c
void rs485_example_multi_task(void)
{
    // 1. 初始化UART0
    uart0_init(1000000);
    
    // 2. 初始化RS485方向控制
    rs485_init();
    
    // 3. 初始化RS485互斥锁
    if (uart0_rs485_mutex_init() != 0) {
        ESP_LOGE(TAG, "互斥锁初始化失败");
        return;
    }
    
    // 4. 发送数据（带互斥锁保护）
    uint8_t tx_data[] = {0xFF, 0xFF, 0xA1, 0x01, 0x02, 0x03};
    int sent = rs485_send_protected(tx_data, sizeof(tx_data), 200);
    if (sent < 0) {
        ESP_LOGW(TAG, "获取互斥锁失败");
    } else {
        ESP_LOGI(TAG, "RS485发送了 %d 字节", sent);
    }
}
```

### 中断接收示例

```c
void uart0_event_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t *rx_buf = (uint8_t *)malloc(1024);
    
    QueueHandle_t uart_queue = uart0_get_event_queue();
    
    while (1) {
        // 等待UART事件
        if (xQueueReceive(uart_queue, &event, portMAX_DELAY)) {
            switch (event.type) {
                case UART_DATA:
                    // 有数据到达
                    int len = uart_read_bytes(UART_NUM_0, rx_buf, event.size, pdMS_TO_TICKS(100));
                    ESP_LOGI(TAG, "接收到 %d 字节", len);
                    break;
                    
                case UART_FIFO_OVF:
                    ESP_LOGW(TAG, "UART FIFO溢出");
                    uart_flush_input(UART_NUM_0);
                    break;
                    
                case UART_BUFFER_FULL:
                    ESP_LOGW(TAG, "UART缓冲区满");
                    uart_flush_input(UART_NUM_0);
                    break;
                    
                default:
                    break;
            }
        }
    }
    
    free(rx_buf);
}
```

---

## 依赖

### 依赖的其他组件

- `board_config`: GPIO 引脚定义（pin_definitions.h）

### ESP-IDF 组件

- `driver`: UART 和 GPIO 驱动
- `freertos`: 任务管理、互斥锁、队列
- `esp_log`: 日志输出

---

## 注意事项

### 硬件限制

- ⚠️ UART0 与 RS485 设备共用，必须使用互斥锁保护
- ⚠️ RS485 总线最大支持 32 个设备（理论值）
- ⚠️ 长距离通信需要添加终端电阻（120Ω）

### 软件限制

- ⚠️ 发送和接收函数都是同步阻塞调用
- ⚠️ 不能在中断服务程序中调用这些函数
- ⚠️ 多任务环境必须使用互斥锁保护 RS485 通信

### 线程安全

- ✅ `rs485_send_protected()` 使用互斥锁保护，线程安全
- ⚠️ `uart0_send()` 和 `rs485_send()` 不是线程安全的
- ⚠️ 多任务环境推荐使用 `rs485_send_protected()`

### 性能考虑

- 波特率越高，CPU 占用越高
- 接收缓冲区大小影响最大接收数据量
- 事件队列模式比轮询模式更高效

---

## 故障排除

### 常见问题

#### 问题1：UART 无法发送数据

**现象**：调用发送函数返回 0

**原因**：
- UART 未初始化
- 引脚配置错误
- 数据指针为 NULL 或长度为 0

**解决方案**：
1. 确认已调用 `uart0_init()` 或 `uart1_init()`
2. 检查引脚定义是否正确
3. 检查数据指针和长度参数

#### 问题2：RS485 通信失败

**现象**：发送数据后无响应

**原因**：
- RS485 方向控制引脚未初始化
- 方向控制电平配置错误
- RS485 接线错误

**解决方案**：
1. 确认已调用 `rs485_init()`
2. 检查 `RS485_DIR_TX_LEVEL` 和 `RS485_DIR_RX_LEVEL` 配置
3. 检查 RS485-A 和 RS485-B 接线

#### 问题3：多任务环境下通信冲突

**现象**：RS485 通信偶尔失败或数据错乱

**原因**：多个任务同时访问 RS485 总线

**解决方案**：
1. 调用 `uart0_rs485_mutex_init()` 初始化互斥锁
2. 使用 `rs485_send_protected()` 代替 `rs485_send()`
3. 确保所有任务都使用互斥锁保护

#### 问题4：接收超时

**现象**：`uart0_recv()` 总是返回 0

**原因**：
- 没有数据到达
- 超时时间设置过短
- 接收引脚配置错误

**解决方案**：
1. 确认发送端正常工作
2. 增加超时时间
3. 检查接收引脚配置

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [GPIO引脚分配文档](../../docs/GPIO_PIN_ALLOCATION.md)
- [Camera Protocol组件文档](../camera_protocol/README.md)
- [485servo组件文档](../485servo/README.md)
- [Voice Module组件文档](../voice_module/README.md)

### 数据手册

- ESP32-S3 技术参考手册 - UART 章节
- RS485 总线标准（TIA/EIA-485）

### 代码示例

- 测试程序：`main/main.c`（UART 和 RS485 通信）

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，完整功能实现 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/uart/`  
**文档类型**: 组件使用说明
