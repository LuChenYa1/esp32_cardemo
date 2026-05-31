# Camera Protocol 使用说明

## 概述

这是一个通过 RS485 与视觉识别摄像头通信的协议组件，支持颜色、人脸、数字、标签、二维码等多种识别功能。

## 初始化

在使用摄像头功能前，需要先调用初始化函数：

```c
#include "camera_protocol.h"

void app_main(void)
{
    // 初始化摄像头协议（会自动初始化 UART 和 RS485 方向控制引脚）
    Camera_Init();
    
    // 后续可以使用摄像头功能...
}
```

## 日志控制

调试完成后，可以修改头文件中的日志级别宏来关闭调试日志：

```c
// camera_protocol.h
#define CAMERA_LOG_LEVEL  ESP_LOG_WARN  // 只打印警告和错误
```

或者在运行时动态设置：

```c
esp_log_level_set("CAMERA", ESP_LOG_WARN);
```

## 函数接口说明

所有函数都采用直接返回值的设计：
- 成功：返回实际数据值（0x01-0xFE）
- 失败：返回 0x00（超时或校验错误）

### 1. 模式切换

```c
uint8_t result = Camera_SetMode(0x01, FUNC_COLOR);
if (result == 0x01) {
    ESP_LOGI("APP", "模式切换成功");
} else {
    ESP_LOGW("APP", "模式切换失败");
}
```

### 2. 颜色识别

#### 指定颜色识别

```c
uint8_t result = Camera_ReadColorSpec(0x01, COLOR_RED);
if (result == 0x01) {
    ESP_LOGI("APP", "检测到红色");
} else if (result == 0x00) {
    ESP_LOGI("APP", "未检测到红色或超时");
}
```

#### 非指定颜色识别

```c
uint8_t detected_color = Camera_ReadColorNonSpec(0x01);
if (detected_color != 0x00) {
    switch (detected_color) {
        case COLOR_RED:    ESP_LOGI("APP", "检测到红色"); break;
        case COLOR_GREEN:  ESP_LOGI("APP", "检测到绿色"); break;
        case COLOR_BLUE:   ESP_LOGI("APP", "检测到蓝色"); break;
        case COLOR_YELLOW: ESP_LOGI("APP", "检测到黄色"); break;
        // ...
    }
} else {
    ESP_LOGW("APP", "未检测到颜色或超时");
}
```

### 3. 人脸识别

```c
// 指定人脸识别
uint8_t result = Camera_ReadFaceSpec(0x01, 0x01);
if (result == 0x01) {
    ESP_LOGI("APP", "检测到指定人脸");
}

// 非指定人脸识别
uint8_t detected_id = Camera_ReadFaceNonSpec(0x01);
if (detected_id != 0x00) {
    ESP_LOGI("APP", "检测到人脸 ID: 0x%02X", detected_id);
}
```

### 4. 数字识别

```c
// 指定数字识别
uint8_t result = Camera_ReadNumberSpec(0x01, NUMBER_5);
if (result == 0x01) {
    ESP_LOGI("APP", "检测到数字 5");
}

// 非指定数字识别
uint8_t detected_number = Camera_ReadNumberNonSpec(0x01);
if (detected_number != 0x00) {
    ESP_LOGI("APP", "检测到数字: 0x%02X", detected_number);
}
```

### 5. 标签识别

```c
// 指定标签识别
uint8_t result = Camera_ReadLabelSpec(0x01, LABEL_1);
if (result == 0x01) {
    ESP_LOGI("APP", "检测到标签 1");
}

// 非指定标签识别
uint8_t detected_label = Camera_ReadLabelNonSpec(0x01);
if (detected_label != 0x00) {
    ESP_LOGI("APP", "检测到标签: 0x%02X", detected_label);
}
```

### 6. 二维码识别

```c
uint8_t qr_buf[64];
uint8_t buf_len = sizeof(qr_buf);
uint8_t result = Camera_ReadQR_Code(0x01, qr_buf, &buf_len);
if (result == 0x01) {
    ESP_LOGI("APP", "检测到二维码");
}
// 注意：当前版本仅返回检测结果，二维码数据解析功能待实现
```

### 7. 设备控制

```c
// 设备重启
uint8_t result = Camera_Reset(0x01);
if (result == 0x01) {
    ESP_LOGI("APP", "重启成功");
}

// 设备控制（如调整亮度、对比度等）
result = Camera_DeviceCTRL(0x01, CTRL_BRIGHTNESS, 50);
if (result == 0x01) {
    ESP_LOGI("APP", "亮度设置成功");
}
```

## 返回值说明

所有函数返回值含义：
- `0x00`: 失败（超时、校验错误或未检测到）
- `0x01`: 成功（对于指定识别）或检测到（对于非指定识别）
- `0x02-0xFE`: 具体的识别结果（如颜色值、人脸 ID、数字值等）

## 常见问题

### 1. 超时问题
- 检查 RS485 连接是否正常
- 确认摄像头设备 ID 是否正确
- 检查波特率设置（默认 1000000）

### 2. 校验错误
- 检查 RS485 线路质量
- 确认接地是否良好
- 尝试降低波特率

### 3. 日志过多
- 修改 `CAMERA_LOG_LEVEL` 为 `ESP_LOG_WARN`
- 或在运行时调用 `esp_log_level_set("CAMERA", ESP_LOG_WARN)`

### 4. 返回值为 0
可能的原因：
- 摄像头未响应（超时）
- 数据校验失败
- 未检测到目标对象

建议查看日志输出，会有详细的错误信息。

## 依赖组件

- `uart`: UART 驱动
- `485servo`: RS485 发送函数（uart0_send）
- `driver`: ESP-IDF GPIO 和 UART 驱动

## 硬件配置

- UART0: TX=GPIO43, RX=GPIO44
- RS485 方向控制: GPIO19
- 波特率: 1000000

## 性能说明

- 调用频率：低频调用（秒级），适合偶尔查询的场景
- 超时时间：100ms（可在头文件中修改 `CAMERA_RX_TIMEOUT_MS`）
- 通信方式：同步阻塞，调用时会等待摄像头响应
