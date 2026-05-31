# Wireless 模块快速使用指南

## 快速开始

### 1. 硬件连接

```
主控ESP32-S3 (GPIO35/36) <--UART1--> ESP32-AT模块 (RX/TX)
波特率: 115200
```

### 2. 在项目中使用

#### 方法1：修改main.c

```c
#include "wireless.h"

void app_main(void) {
    // 初始化
    wireless_init();
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 测试AT
    if (wireless_test()) {
        ESP_LOGI("APP", "模块正常");
    }
}
```

#### 方法2：使用测试文件

1. 打开 `main/main_wireless_test.c`
2. 修改配置区域的WiFi信息：
   ```c
   #define WIFI_SSID "YourSSID"
   #define WIFI_PASSWORD "YourPassword"
   ```
3. 选择要运行的测试：
   ```c
   int test_mode = 1;  // 1-7选择不同测试
   ```
4. 编译上传

### 3. 编译项目

```bash
idf.py build
idf.py flash monitor
```

## 测试项目说明

| 测试编号 | 测试内容 | 说明 |
|---------|---------|------|
| 1 | 基础AT指令 | 测试模块响应、查询版本 |
| 2 | WiFi连接 | 连接WiFi、查询IP、Ping测试 |
| 3 | TCP客户端 | 连接服务器、发送接收数据 |
| 4 | TCP透传 | 透传模式数据收发 |
| 5 | 蓝牙扫描 | 扫描周围BLE设备 |
| 6 | BLE SPP服务端 | 作为服务端等待连接 |
| 7 | BLE SPP客户端 | 连接服务端进行通信 |

## 常用代码片段

### WiFi连接

```c
wireless_init();
vTaskDelay(pdMS_TO_TICKS(2000));

wireless_set_wifi_mode(1);
if (wireless_connect_wifi("SSID", "Password")) {
    ESP_LOGI("WIFI", "连接成功");
    wireless_query_ip();
}
```

### TCP通信

```c
// 连接服务器
wireless_set_single_connection();
wireless_connect_tcp("192.168.1.100", 5001);

// 发送数据
const char *msg = "Hello!";
wireless_send_data((uint8_t *)msg, strlen(msg));

// 接收数据
uint8_t buf[128];
uint16_t len = wireless_get_received_data(buf, sizeof(buf));
```

### BLE扫描

```c
wireless_ble_init(1);  // Client模式
wireless_ble_scan(1, 5);  // 扫描5秒
vTaskDelay(pdMS_TO_TICKS(6000));
```

## 注意事项

1. **初始化后等待2秒**，让模块稳定
2. **WiFi连接超时设置20秒**，信号差时需要更长时间
3. **透传模式退出**需要前后各1秒间隔
4. **BLE连接超时设置30秒**，蓝牙连接较慢
5. **查看日志**了解模块响应，标签为"WIRELESS"

## 故障排查

### 模块无响应
- 检查TX/RX是否交叉连接
- 检查波特率是否为115200
- 检查ESP32-AT模块供电

### WiFi连接失败
- 检查SSID和密码
- 检查WiFi信号强度
- 增加超时时间

### TCP连接失败
- 先确保WiFi已连接
- 检查服务器IP和端口
- 确保服务器正在运行

## 更多信息

详细文档请参考：[README.md](README.md)
