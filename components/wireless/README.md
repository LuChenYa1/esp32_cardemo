# Wireless（无线通信模块驱动）

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: ⏸️ 未使用

---

## 组件简介

Wireless 是一个基于 AT 指令的无线通信模块驱动，通过 UART1 控制外部 ESP32-AT 模块，为主控 ESP32 扩展 WiFi、TCP、BLE 等无线通信能力。该组件封装了完整的 AT 指令集，提供简洁的 API 接口。

### 主要特性

- 支持 WiFi 连接（Station/AP/混合模式）
- 支持 TCP 客户端/服务器
- 支持 BLE 客户端/服务器
- 支持透传模式（WiFi 和 BLE）
- 完整的 AT 指令封装
- 异步接收机制

### 适用场景

- 为主控 ESP32 扩展无线通信能力
- 需要同时使用多个无线协议
- 需要独立的无线通信模块
- 远程控制和数据传输

---

## 硬件连接

### 引脚分配

| 功能 | GPIO引脚 | 接口标识 | 说明 |
|------|---------|---------|------|
| UART1 TX | GPIO17 | - | 发送AT指令到模块 |
| UART1 RX | GPIO18 | - | 接收模块响应 |

### 接线说明

1. 将 ESP32-AT 模块的 RX 连接到主控的 GPIO17（UART1 TX）
2. 将 ESP32-AT 模块的 TX 连接到主控的 GPIO18（UART1 RX）
3. 确保两个 ESP32 共地（GND 连接）
4. ESP32-AT 模块独立供电（3.3V 或 5V，根据模块要求）

### 注意事项

- ⚠️ ESP32-AT 模块需要独立供电，电流需求较大
- ⚠️ 确保 AT 固件版本兼容（推荐 ESP-AT v2.x）
- ⚠️ UART1 波特率默认 115200，需与模块配置一致

---

## 功能说明

### 核心功能

#### 功能1：WiFi 连接

支持 Station、AP、混合三种模式，可连接到 WiFi 网络或创建热点。

#### 功能2：TCP 通信

支持 TCP 客户端和服务器模式，可建立 TCP 连接并收发数据。

#### 功能3：BLE 通信

支持 BLE 客户端和服务器模式，可扫描、连接、收发 BLE 数据。

#### 功能4：透传模式

支持 WiFi 和 BLE 透传模式，简化数据收发流程。

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| WIRELESS_RX_BUF_SIZE | 512 | 128-2048 | 接收缓冲区大小 |
| AT_TIMEOUT_MS | 5000 | 1000-30000 | AT指令超时时间（毫秒） |
| UART1 波特率 | 115200 | 9600-921600 | 串口波特率 |

---

## API接口

### 初始化函数

```c
/**
 * @brief 初始化无线模块
 */
void wireless_init(void);
```

### WiFi 功能

```c
// 设置WiFi模式（1=Station, 2=AP, 3=混合）
uint8_t wireless_set_wifi_mode(uint8_t mode);

// 连接WiFi
uint8_t wireless_connect_wifi(const char *ssid, const char *password);

// 断开WiFi
uint8_t wireless_disconnect_wifi(void);

// 查询IP地址
uint8_t wireless_query_ip(void);
```

### TCP 功能

```c
// 建立TCP连接
uint8_t wireless_connect_tcp(const char *host, uint16_t port);

// 发送数据
uint8_t wireless_send_data(const uint8_t *data, uint16_t len);

// 断开TCP连接
uint8_t wireless_disconnect_tcp(void);

// 进入/退出透传模式
uint8_t wireless_enter_transparent(void);
void wireless_exit_transparent(void);
```

### BLE 功能

```c
// 初始化蓝牙（0=关闭, 1=Client, 2=Server）
uint8_t wireless_ble_init(uint8_t mode);

// 设置设备名称
uint8_t wireless_ble_set_name(const char *name);

// 扫描设备
uint8_t wireless_ble_scan(uint8_t enable, uint8_t duration);

// 连接设备
uint8_t wireless_ble_connect(uint8_t conn_index, const char *mac);

// 进入/退出SPP透传
uint8_t wireless_ble_enter_spp(void);
void wireless_ble_exit_spp(void);
```

---

## 使用示例

### WiFi 连接示例

```c
#include "wireless.h"

void wifi_example(void)
{
    // 初始化
    wireless_init();
    
    // 设置为Station模式
    wireless_set_wifi_mode(1);
    
    // 连接WiFi
    if (wireless_connect_wifi("MyWiFi", "password123")) {
        ESP_LOGI(TAG, "WiFi连接成功");
        
        // 查询IP地址
        wireless_query_ip();
    }
}
```

### TCP 通信示例

```c
void tcp_example(void)
{
    // 建立TCP连接
    if (wireless_connect_tcp("192.168.1.100", 8080)) {
        ESP_LOGI(TAG, "TCP连接成功");
        
        // 发送数据
        const char *msg = "Hello Server";
        wireless_send_data((uint8_t *)msg, strlen(msg));
    }
}
```

---

## 依赖

### 依赖的其他组件

- `uart`: UART1 驱动

### ESP-IDF 组件

- `driver`: UART 驱动
- `freertos`: 任务管理
- `esp_log`: 日志输出

---

## 注意事项

### 硬件限制

- ⚠️ 需要外部 ESP32-AT 模块
- ⚠️ 模块需要独立供电

### 软件限制

- ⚠️ AT 指令为同步阻塞调用
- ⚠️ 透传模式下无法发送 AT 指令

### 线程安全

- ⚠️ 不是线程安全的，多任务环境需要加锁

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [UART组件文档](../uart/README.md)
- [ESP-AT 用户指南](https://docs.espressif.com/projects/esp-at/)

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/wireless/`  
**文档类型**: 组件使用说明
