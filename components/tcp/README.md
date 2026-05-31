# TCP（TCP网络通信）

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: ⏸️ 未使用

---

## 组件简介

TCP 是一个基于 ESP-IDF lwIP 协议栈的 TCP 网络通信组件，提供 TCP 客户端和服务器功能。支持可靠的数据传输、自动重连、异步接收等特性。

### 主要特性

- TCP 服务器模式（Slave）
- TCP 客户端模式（Client）
- 自动重连机制
- 异步接收任务
- 支持多客户端连接（服务器模式）

### 适用场景

- 远程控制和监控
- 数据采集和上传
- 设备间可靠通信
- 局域网通信

---

## 硬件连接

### 引脚分配

TCP 通信基于 WiFi，无需额外引脚连接。

### 注意事项

- ⚠️ 需要先初始化 WiFi 并连接到网络
- ⚠️ 确保网络连通性和防火墙设置

---

## 功能说明

### 核心功能

#### 功能1：TCP 服务器（Slave）

启动 TCP 服务器，监听指定端口，接受客户端连接并收发数据。

#### 功能2：TCP 客户端（Client）

连接到远程 TCP 服务器，发送和接收数据，支持自动重连。

#### 功能3：异步接收

后台任务持续接收数据，避免阻塞主任务。

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| 服务器端口 | 用户指定 | 1-65535 | TCP服务器监听端口 |
| 默认服务器 | 192.168.130.125:3333 | - | 客户端默认连接地址 |
| 重连间隔 | 5秒 | 1-60秒 | 客户端断线重连间隔 |

---

## API接口

### 服务器模式

```c
/**
 * @brief 启动TCP服务器
 * 
 * @param port 监听端口
 * @return 0=成功，-1=失败
 */
int tcp_slave_start(uint16_t port);

/**
 * @brief 向客户端发送数据
 * 
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际发送的字节数，-1=失败
 */
int tcp_slave_send(const char *data, int len);
```

### 客户端模式

```c
/**
 * @brief 连接到TCP服务器
 * 
 * @param host 服务器IP地址
 * @param port 服务器端口
 * @return 0=成功，-1=失败
 */
int tcp_client_connect(const char *host, uint16_t port);

/**
 * @brief 连接到默认服务器（192.168.130.125:3333）
 * 
 * @return 0=成功，-1=失败
 */
int tcp_client_connect_default(void);

/**
 * @brief 向服务器发送数据
 * 
 * @param data 数据指针
 * @param len 数据长度
 * @return 实际发送的字节数，-1=失败
 */
int tcp_client_send(const char *data, int len);

/**
 * @brief 断开客户端连接
 */
void tcp_client_disconnect(void);
```

---

## 使用示例

### TCP 服务器示例

```c
#include "tcp.h"
#include "wifi.h"

void tcp_server_example(void)
{
    // 1. 初始化WiFi
    wifi_init_sta();
    
    // 2. 启动TCP服务器（端口8080）
    if (tcp_slave_start(8080) == 0) {
        ESP_LOGI(TAG, "TCP服务器启动成功");
        
        // 3. 发送数据到客户端
        const char *msg = "Hello Client";
        tcp_slave_send(msg, strlen(msg));
    }
}
```

### TCP 客户端示例

```c
void tcp_client_example(void)
{
    // 1. 初始化WiFi
    wifi_init_sta();
    
    // 2. 连接到服务器
    if (tcp_client_connect("192.168.1.100", 8080) == 0) {
        ESP_LOGI(TAG, "TCP连接成功");
        
        // 3. 发送数据
        const char *msg = "Hello Server";
        tcp_client_send(msg, strlen(msg));
    }
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
- ⚠️ 服务器模式和客户端模式不能同时使用

### 线程安全

- ⚠️ 发送函数不是线程安全的，多任务环境需要加锁

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
**组件路径**: `components/tcp/`  
**文档类型**: 组件使用说明
