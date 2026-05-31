# 无线通信模块测试程序使用说明

## 概述

`main_wireless_test.c` 是ESP32无线通信模块的综合测试程序，包含WiFi、TCP、蓝牙BLE、SPP透传等7个测试项目。

**注意：** 该测试程序当前已被注释，需要取消注释后才能使用。

## 硬件连接

### 无线模块接线

| 信号线 | ESP32 GPIO | 说明 |
|--------|-----------|------|
| TX（模块发送） | GPIO16 (RX) | UART接收 |
| RX（模块接收） | GPIO17 (TX) | UART发送 |
| VCC | 3.3V | 电源正极 |
| GND | GND | 电源负极 |

### 注意事项
- 无线模块使用3.3V供电，不要接5V
- 确保TX/RX交叉连接（模块TX接ESP32 RX）
- 建议使用独立电源供电，避免电流不足

## 测试内容

### 测试1：基础AT指令
- 测试AT指令响应
- 查询固件版本
- 查询IP地址
- 验证模块通信正常

### 测试2：WiFi连接
- 设置Station模式
- 连接指定WiFi网络
- 查询IP地址
- Ping测试网络连通性

### 测试3：TCP客户端
- 设置单连接模式
- 连接TCP服务器
- 发送测试数据
- 接收服务器响应
- 断开连接

### 测试4：TCP透传
- 连接TCP服务器
- 进入透传模式
- 发送多条测试数据
- 退出透传模式
- 恢复普通模式

### 测试5：蓝牙扫描
- 初始化蓝牙（Client模式）
- 查询蓝牙地址
- 扫描周围蓝牙设备
- 关闭蓝牙

### 测试6：BLE SPP服务端
- 初始化蓝牙（Server模式）
- 创建GATT服务
- 启动GATT服务
- 设置设备名称和广播参数
- 开始广播
- 等待客户端连接
- 配置SPP参数
- 进入SPP透传
- 发送测试数据

### 测试7：BLE SPP客户端
- 初始化蓝牙（Client模式）
- 扫描蓝牙设备
- 连接服务端
- 配置SPP参数
- 进入SPP透传
- 发送测试数据
- 保存透传配置（自动重连）

## 配置参数

在运行测试前，需要修改代码中的配置参数：

### WiFi配置（测试2-4）

```c
#define WIFI_SSID "YourSSID"          // 修改为你的WiFi名称
#define WIFI_PASSWORD "YourPassword"  // 修改为你的WiFi密码
```

### TCP服务器配置（测试3-4）

```c
#define TCP_SERVER_IP "192.168.1.100"  // 修改为TCP服务器IP
#define TCP_SERVER_PORT 5001           // 修改为TCP服务器端口
```

### BLE配置（测试6-7）

```c
// 选择设备角色
#define BLE_DEVICE_ROLE_SERVER 1  // 1=服务端, 0=客户端

// 服务端配置
#define BLE_SERVER_NAME "ESP32_Server"

// 客户端配置（填写服务端MAC地址）
#define BLE_SERVER_MAC "24:0a:c4:d6:e4:46"  // 修改为实际MAC地址
```

### 选择测试项目

在 `app_main()` 函数中修改：

```c
int test_mode = 1;  // 修改为1-7选择测试项目
```

## 编译和运行

### 1. 取消代码注释

编辑 `main/main_wireless_test.c`，删除文件开头和结尾的注释符号：

```c
// 删除开头的 //
// 删除结尾的 //
```

### 2. 修改CMakeLists.txt

编辑 `main/CMakeLists.txt`：

```cmake
idf_component_register(
    SRCS "main_wireless_test.c"
    INCLUDE_DIRS "."
    REQUIRES wireless
)
```

### 3. 配置参数

根据你的测试需求，修改代码中的配置参数（WiFi、TCP、BLE）。

### 4. 选择测试项目

修改 `app_main()` 中的 `test_mode` 变量（1-7）。

### 5. 编译烧录

```bash
idf.py build flash monitor
```

## 预期输出

### 测试1：基础AT指令

```
I (1234) WIRELESS_TEST: ================================================
I (1234) WIRELESS_TEST:        ESP32无线通信模块测试程序
I (1234) WIRELESS_TEST: ================================================
I (1234) WIRELESS_TEST: 运行测试 1...

I (1234) WIRELESS_TEST: ========== 测试1：基础AT指令 ==========
I (1234) WIRELESS_TEST: 1. 测试AT指令...
I (1234) WIRELESS_TEST:    [OK] 模块响应正常
I (1234) WIRELESS_TEST: 2. 查询固件版本...
I (1234) WIRELESS: AT+GMR
I (1234) WIRELESS: OK
I (1234) WIRELESS: AT version:2.2.0.0
I (1234) WIRELESS_TEST: 3. 查询IP地址...
I (1234) WIRELESS_TEST: 测试完成
```

### 测试2：WiFi连接

```
I (2000) WIRELESS_TEST: ========== 测试2：WiFi连接 ==========
I (2000) WIRELESS_TEST: 1. 设置Station模式...
I (2000) WIRELESS_TEST: 2. 连接WiFi: YourSSID...
I (2000) WIRELESS_TEST:    (等待10-20秒)
I (15000) WIRELESS_TEST:    [OK] WiFi连接成功
I (15000) WIRELESS_TEST: 3. 查询IP地址...
I (15000) WIRELESS: +CIFSR:STAIP,"192.168.1.100"
I (15000) WIRELESS_TEST: 4. Ping测试: baidu.com...
I (16000) WIRELESS_TEST:    [OK]
I (16000) WIRELESS_TEST: 测试完成
```

### 测试3：TCP客户端

```
I (3000) WIRELESS_TEST: ========== 测试3：TCP客户端 ==========
I (3000) WIRELESS_TEST: 1. 设置单连接模式...
I (3000) WIRELESS_TEST: 2. 连接TCP服务器: 192.168.1.100:5001...
I (4000) WIRELESS_TEST:    [OK]
I (4000) WIRELESS_TEST: 3. 发送测试数据...
I (4000) WIRELESS_TEST:    [OK]
I (4000) WIRELESS_TEST: 4. 等待接收数据...
I (6000) WIRELESS_TEST:    接收到 17 字节: Hello from Server
I (6000) WIRELESS_TEST: 5. 断开连接...
I (6000) WIRELESS_TEST: 测试完成
```

## 测试准备

### WiFi测试准备（测试2-4）

1. 准备一个可用的WiFi网络
2. 记录WiFi的SSID和密码
3. 修改代码中的WiFi配置

### TCP测试准备（测试3-4）

1. 在电脑上运行TCP服务器：

**使用Python创建简单TCP服务器：**

```python
import socket

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(('0.0.0.0', 5001))
server.listen(1)
print("TCP服务器启动，等待连接...")

while True:
    client, addr = server.accept()
    print(f"客户端连接: {addr}")
    
    while True:
        data = client.recv(1024)
        if not data:
            break
        print(f"接收: {data.decode()}")
        client.send(b"Hello from Server")
    
    client.close()
```

2. 运行服务器：`python tcp_server.py`
3. 记录服务器IP地址和端口
4. 修改代码中的TCP配置

### BLE测试准备（测试6-7）

**测试6（服务端）：**
1. 运行测试6，记录显示的MAC地址
2. 使用手机BLE调试工具连接
3. 或者使用另一个ESP32运行测试7（客户端）

**测试7（客户端）：**
1. 先运行测试6（服务端），记录MAC地址
2. 修改代码中的 `BLE_SERVER_MAC` 为服务端MAC地址
3. 运行测试7连接服务端

## 故障排查

### 问题1：模块无响应

**可能原因：**
- UART连接错误
- 波特率不匹配
- 模块未上电

**解决方法：**
1. 检查TX/RX是否交叉连接
2. 确认波特率为115200
3. 检查3.3V供电
4. 查看串口日志是否有UART错误

### 问题2：WiFi连接失败

**可能原因：**
- SSID或密码错误
- WiFi信号弱
- 模块不支持5GHz WiFi

**解决方法：**
1. 确认SSID和密码正确
2. 靠近WiFi路由器
3. 使用2.4GHz WiFi网络
4. 检查路由器是否开启MAC过滤

### 问题3：TCP连接失败

**可能原因：**
- 服务器IP或端口错误
- 服务器未运行
- 防火墙阻止连接
- 不在同一网络

**解决方法：**
1. 确认服务器IP和端口
2. 确认服务器正在运行
3. 关闭防火墙或添加例外
4. 确保ESP32和服务器在同一局域网

### 问题4：BLE连接失败

**可能原因：**
- MAC地址错误
- 服务端未启动广播
- 距离太远
- 蓝牙干扰

**解决方法：**
1. 确认MAC地址格式正确（小写，冒号分隔）
2. 先运行服务端，再运行客户端
3. 缩短设备距离（< 10米）
4. 远离WiFi路由器和其他蓝牙设备

### 问题5：透传模式无法退出

**可能原因：**
- 退出指令未正确发送
- 模块固件问题

**解决方法：**
1. 发送 `+++` 退出透传（注意前后需要1秒延时）
2. 重启模块
3. 升级模块固件

## 性能指标

- UART波特率：115200
- WiFi连接时间：10-20秒
- TCP连接时间：1-3秒
- BLE扫描时间：3-5秒
- BLE连接时间：10-30秒
- 透传模式延迟：< 100ms

## 注意事项

1. **代码注释**：测试程序默认被注释，使用前需要取消注释
2. **配置参数**：必须修改WiFi、TCP、BLE配置参数
3. **测试顺序**：建议按顺序测试（1→2→3→...）
4. **WiFi优先**：WiFi和BLE不能同时使用，测试BLE前需断开WiFi
5. **电源要求**：无线模块功耗较大，建议使用独立电源
6. **串口冲突**：确保没有其他组件使用UART1（GPIO16/17）
7. **超时设置**：某些操作需要较长时间，注意调整超时参数
8. **错误处理**：测试失败时查看串口日志，根据错误信息排查

## 扩展测试

### 添加自定义测试

在代码中添加新的测试函数：

```c
void test_custom(void)
{
    ESP_LOGI(TAG, "========== 自定义测试 ==========");
    
    // 在这里添加你的测试代码
    
    ESP_LOGI(TAG, "测试完成\n");
}

void app_main(void)
{
    // ... 现有代码 ...
    
    case 8:
        test_custom();
        break;
}
```

### 修改测试参数

根据实际需求修改测试参数：

```c
// 修改WiFi连接超时
wireless_connect_wifi(WIFI_SSID, WIFI_PASSWORD);  // 默认20秒

// 修改TCP发送数据
const char *test_data = "Your custom data";

// 修改BLE扫描时间
wireless_ble_scan(1, 10);  // 扫描10秒
```

## 相关文档

- [无线通信组件README](../components/wireless/README.md)
- [UART组件README](../components/uart/README.md)
- [引脚定义](../components/board_config/include/pin_definitions.h)
- [测试程序索引](TESTS_INDEX.md)

---

**文档版本：** 1.0  
**创建日期：** 2024-12-XX  
**维护者：** 项目团队  
**最后更新：** 2024-12-XX
