# UDP（UDP网络通信）

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: ⏸️ 未使用

---

## 组件简介

UDP 是一个基于 ESP-IDF lwIP 协议栈的 UDP 网络通信组件，提供 UDP 数据报收发功能。支持单播、广播、回调接收等特性，适合低延迟、无连接的数据传输场景。

### 主要特性

- UDP 数据报收发
- 支持单播和广播
- 异步接收任务
- 回调函数机制
- 无连接、低延迟

### 适用场景

- 实时数据传输
- 局域网设备发现
- 广播消息
- 低延迟通信

---

## 硬件连接

### 引脚分配

UDP 通信基于 WiFi，无需额外引脚连接。

### 注意事项

- ⚠️ 需要先初始化 WiFi 并连接到网络
- ⚠️ UDP 不保证数据可靠性，可能丢包

---

## 功能说明

### 核心功能

#### 功能1：UDP 接收

绑定本地端口，接收来自任意主机的 UDP 数据报。

#### 功能2：UDP 发送

向指定主机和端口发送 UDP 数据报。

#### 功能3：广播发送

向局域网内所有设备发送广播消息。

#### 功能4：回调接收

注册回调函数，异步处理接收到的数据。

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| 本地端口 | 用户指定 | 1-65535 | UDP绑定端口 |
| 接收缓冲区 | 1024 | 128-4096 | 接收缓冲区大小 |

---

## API接口

### 初始化和接收

```c
/**
 * @brief 启动UDP接收（绑定本地端口）
 * 
 * @param port 本地端口
 * @return 0=成功，-1=失败
 */
int udp_start(uint16_t port);

/**
 * @brief 设置接收回调函数
 * 
 * @param cb 回调函数指针，NULL取消回调
 */
void udp_set_recv_callback(udp_recv_cb_t cb);
```

### 发送函数

```c
/**
 * @brief 发送到指定主机和端口
 * 
 * @param host 目标IP地址（点分十进制）
 * @param port 目标端口
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际发送的字节数，-1=失败
 */
int udp_send_to(const char *host, uint16_t port, const char *data, int len);

/**
 * @brief 发送广播到指定端口
 * 
 * @param port 目标端口
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际发送的字节数，-1=失败
 */
int udp_send_broadcast(uint16_t port, const char *data, int len);
```

---

## 使用示例

### UDP 接收示例

```c
#include "udp.h"
#include "wifi.h"

// 接收回调函数
void udp_recv_callback(const char *data, int len, const char *addr, uint16_t port)
{
    ESP_LOGI(TAG, "收到来自 %s:%d 的数据，长度 %d", addr, port, len);
    ESP_LOG_BUFFER_HEXDUMP(TAG, data, len, ESP_LOG_INFO);
}

void udp_recv_example(void)
{
    // 1. 初始化WiFi
    wifi_init_sta();
    
    // 2. 启动UDP接收（端口5000）
    if (udp_start(5000) == 0) {
        ESP_LOGI(TAG, "UDP接收启动成功");
        
        // 3. 设置接收回调
        udp_set_recv_callback(udp_recv_callback);
    }
}
```

### UDP 发送示例

```c
void udp_send_example(void)
{
    // 1. 初始化WiFi
    wifi_init_sta();
    
    // 2. 发送到指定主机
    const char *msg = "Hello UDP";
    int sent = udp_send_to("192.168.1.100", 5000, msg, strlen(msg));
    ESP_LOGI(TAG, "发送了 %d 字节", sent);
}
```

### UDP 广播示例

```c
void udp_broadcast_example(void)
{
    // 1. 初始化WiFi
    wifi_init_sta();
    
    // 2. 发送广播消息
    const char *msg = "Broadcast Message";
    int sent = udp_send_broadcast(5000, msg, strlen(msg));
    ESP_LOGI(TAG, "广播了 %d 字节", sent);
}
```

---

## 依赖

### 依赖的其他组件

- `wifi`: WiFi 连接

### ESP-IDF 组件

- `lwip`: TCP/IP 协议栈
- `freertos`: 任务管理
- `esp_log`: 日志输出

---

## 注意事项

### 硬件限制

- ⚠️ 需要 WiFi 连接

### 软件限制

- ⚠️ 必须先初始化 WiFi
- ⚠️ UDP 不保证数据可靠性，可能丢包或乱序

### 线程安全

- ⚠️ 发送函数不是线程安全的，多任务环境需要加锁

### 性能考虑

- UDP 比 TCP 延迟更低，但不保证可靠性
- 适合实时性要求高、可容忍少量丢包的场景

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [WiFi组件文档](../wifi/README.md)
- [ESP-IDF Socket 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/lwip.html)

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/udp/`  
**文档类型**: 组件使用说明
