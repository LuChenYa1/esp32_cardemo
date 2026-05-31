# Camera Protocol（摄像头通信协议）

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: ✅ main.c使用中

---

## 组件简介

Camera Protocol 是一个通过 RS485 与视觉识别摄像头通信的协议组件，支持颜色、人脸、数字、标签、二维码等多种识别功能。该组件封装了完整的 RS485 通信协议，提供简洁的 API 接口用于摄像头控制和数据读取。

### 主要特性

- 支持多种识别模式：颜色、人脸、数字、标签、二维码、20类物品、深度学习
- 支持指定和非指定两种识别方式
- 基于 RS485 总线通信，支持多设备寻址
- 完整的错误检测机制（超时检测、校验和验证）
- 事件驱动的接收机制，避免 CPU 忙等待
- 互斥锁保护，支持多任务安全访问
- 可配置的超时时间和日志级别

### 适用场景

- 智能小车视觉识别系统
- 颜色分拣系统
- 人脸识别门禁
- 数字/标签识别系统
- 二维码扫描应用

---

## 硬件连接

### 引脚分配

| 功能 | GPIO引脚 | 接口标识 | 说明 |
|------|---------|---------|------|
| UART TX | GPIO43 | - | RS485 发送数据 |
| UART RX | GPIO44 | - | RS485 接收数据 |
| RS485 方向控制 | GPIO19 | - | 控制 RS485 收发方向 |

### 接线说明

1. 将摄像头的 RS485-A 连接到 ESP32 的 RS485 转换器 A 端
2. 将摄像头的 RS485-B 连接到 ESP32 的 RS485 转换器 B 端
3. 确保摄像头和 ESP32 共地（GND 连接）
4. 摄像头供电电压根据设备要求（通常 5V 或 12V）

### 注意事项

- ⚠️ RS485 总线需要使用双绞线，减少干扰
- ⚠️ 长距离通信时需要在总线两端添加 120Ω 终端电阻
- ⚠️ 确保 RS485 方向控制引脚（GPIO19）正确配置
- ⚠️ UART0 与 RS485 舵机共用，通过互斥锁保护

---

## 功能说明

### 核心功能

#### 功能1：模式切换

支持在不同识别模式之间切换，包括颜色识别、人脸识别、数字识别、标签识别、二维码识别等。

#### 功能2：指定识别

检测摄像头是否识别到指定的目标（如指定颜色、指定人脸 ID、指定数字等），返回是否检测到的结果。

#### 功能3：非指定识别

检测摄像头识别到的任意目标，返回具体的识别结果（如检测到的颜色值、人脸 ID、数字值等）。

#### 功能4：设备控制

控制摄像头的各种参数，如亮度、对比度、曝光、白平衡、饱和度、锐化等。

#### 功能5：设备重启

远程重启摄像头设备。

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| CAMERA_UART_PORT | UART_NUM_0 | UART_NUM_0/1/2 | 使用的 UART 端口 |
| CAMERA_RX_TIMEOUT_MS | 100 | 10-1000 | 接收超时时间（毫秒） |
| CAMERA_LOG_LEVEL | ESP_LOG_INFO | ESP_LOG_* | 日志级别 |
| RS485_DIR_GPIO | GPIO19 | GPIO_NUM_* | RS485 方向控制引脚 |

---

## API接口

### 初始化函数

```c
/**
 * @brief 初始化摄像头协议
 * 
 * @note 会自动调用 servo485_init() 初始化 UART 和 RS485 方向控制引脚
 */
void Camera_Init(void);
```

**参数说明**：
- 无

**返回值**：
- 无

**使用说明**：
在使用摄像头功能前必须先调用此函数进行初始化。该函数会自动初始化 UART0 和 RS485 方向控制引脚。

---

### 模式切换函数

```c
/**
 * @brief 模式切换
 * 
 * @param id 摄像头设备 ID（0x01-0xFE）
 * @param func 功能码（FUNC_COLOR, FUNC_FACE 等）
 * @param result 输出参数，存储切换结果（0x01=成功，0x00=失败）
 * @return CAMERA_OK 成功，CAMERA_TIMEOUT 超时，CAMERA_CHECKSUM_ERR 校验错误
 */
uint8_t Camera_SetMode(uint8_t id, uint8_t func, uint8_t *result);
```

**参数说明**：
- `id`: 摄像头设备 ID，通常为 0x01
- `func`: 功能码，可选值：
  - `FUNC_COLOR` (0x01): 颜色识别
  - `FUNC_FACE` (0x03): 人脸识别
  - `FUNC_NUMBER` (0x05): 数字识别
  - `FUNC_LABEL` (0x06): 标签识别
  - `FUNC_QRCODE` (0x04): 二维码识别
- `result`: 输出参数，存储切换结果

**返回值**：
- `CAMERA_OK` (0): 成功
- `CAMERA_TIMEOUT` (1): 超时
- `CAMERA_CHECKSUM_ERR` (2): 校验错误

**使用说明**：
在使用特定识别功能前，需要先切换到对应的模式。

---

### 颜色识别函数

```c
/**
 * @brief 读指定颜色识别
 * 
 * @param id 摄像头设备 ID
 * @param color 颜色值（COLOR_RED, COLOR_GREEN 等）
 * @return 识别结果（0x01=检测到，0x00=未检测到/超时）
 */
uint8_t Camera_ReadColorSpec(uint8_t id, uint8_t color);

/**
 * @brief 读非指定颜色识别
 * 
 * @param id 摄像头设备 ID
 * @return 检测到的颜色值（COLOR_RED, COLOR_GREEN 等），0x00=未检测到/超时
 */
uint8_t Camera_ReadColorNonSpec(uint8_t id);
```

**参数说明**：
- `id`: 摄像头设备 ID
- `color`: 颜色值，可选值：
  - `COLOR_RED` (0x01): 红色
  - `COLOR_GREEN` (0x02): 绿色
  - `COLOR_BLUE` (0x03): 蓝色
  - `COLOR_YELLOW` (0x04): 黄色
  - `COLOR_GRAY` (0x05): 灰色
  - `COLOR_PURPLE` (0x06): 紫色
  - `COLOR_WHITE` (0x07): 白色
  - `COLOR_BLACK` (0x08): 黑色

**返回值**：
- 指定识别：0x01=检测到，0x00=未检测到/超时
- 非指定识别：检测到的颜色值，0x00=未检测到/超时

**使用说明**：
- 指定识别用于判断是否检测到特定颜色
- 非指定识别用于获取检测到的任意颜色

---

### 人脸识别函数

```c
/**
 * @brief 读指定人脸识别
 * 
 * @param id 摄像头设备 ID
 * @param face_id 人脸 ID
 * @return 识别结果（0x01=检测到，0x00=未检测到/超时）
 */
uint8_t Camera_ReadFaceSpec(uint8_t id, uint8_t face_id);

/**
 * @brief 读非指定人脸识别
 * 
 * @param id 摄像头设备 ID
 * @return 检测到的人脸 ID（0x01-0xFE），0x00=未检测到/超时
 */
uint8_t Camera_ReadFaceNonSpec(uint8_t id);
```

**参数说明**：
- `id`: 摄像头设备 ID
- `face_id`: 人脸 ID（0x01-0xFE）

**返回值**：
- 指定识别：0x01=检测到，0x00=未检测到/超时
- 非指定识别：检测到的人脸 ID，0x00=未检测到/超时

---

### 数字识别函数

```c
/**
 * @brief 读指定数字识别
 * 
 * @param id 摄像头设备 ID
 * @param num_param 数字参数（NUMBER_0 ~ NUMBER_9）
 * @return 识别结果（0x01=检测到，0x00=未检测到/超时）
 */
uint8_t Camera_ReadNumberSpec(uint8_t id, uint8_t num_param);

/**
 * @brief 读非指定数字识别
 * 
 * @param id 摄像头设备 ID
 * @return 检测到的数字值（NUMBER_0 ~ NUMBER_9），0x00=未检测到/超时
 */
uint8_t Camera_ReadNumberNonSpec(uint8_t id);
```

**参数说明**：
- `id`: 摄像头设备 ID
- `num_param`: 数字参数（NUMBER_0 ~ NUMBER_9）

**返回值**：
- 指定识别：0x01=检测到，0x00=未检测到/超时
- 非指定识别：检测到的数字值，0x00=未检测到/超时

---

### 标签识别函数

```c
/**
 * @brief 读指定标签识别
 * 
 * @param id 摄像头设备 ID
 * @param label 标签值（LABEL_1 ~ LABEL_20）
 * @return 识别结果（0x01=检测到，0x00=未检测到/超时）
 */
uint8_t Camera_ReadLabelSpec(uint8_t id, uint8_t label);

/**
 * @brief 读非指定标签识别
 * 
 * @param id 摄像头设备 ID
 * @return 检测到的标签值（LABEL_1 ~ LABEL_20），0x00=未检测到/超时
 */
uint8_t Camera_ReadLabelNonSpec(uint8_t id);
```

**参数说明**：
- `id`: 摄像头设备 ID
- `label`: 标签值（LABEL_1 ~ LABEL_20）

**返回值**：
- 指定识别：0x01=检测到，0x00=未检测到/超时
- 非指定识别：检测到的标签值，0x00=未检测到/超时

---

### 二维码识别函数

```c
/**
 * @brief 读取二维码数据
 * 
 * @param id 摄像头设备 ID
 * @param qr_buf 用于存放二维码数据的缓冲区（预留，当前未使用）
 * @param buf_len 输入时为缓冲区最大长度，输出时为实际接收到的数据长度（预留，当前未使用）
 * @return 检测结果（0x01=检测到，0x00=未检测到/超时）
 */
uint8_t Camera_ReadQR_Code(uint8_t id, uint8_t *qr_buf, uint8_t *buf_len);
```

**参数说明**：
- `id`: 摄像头设备 ID
- `qr_buf`: 二维码数据缓冲区（预留）
- `buf_len`: 缓冲区长度（预留）

**返回值**：
- 0x01=检测到，0x00=未检测到/超时

**使用说明**：
当前版本仅返回检测结果，二维码数据解析功能待实现。

---

### 设备控制函数

```c
/**
 * @brief 设备控制
 * 
 * @param id 摄像头设备 ID
 * @param dat1 控制项
 * @param dat2 控制参数
 * @param result 输出参数，存储控制结果（0x01=成功，0x00=失败）
 * @return CAMERA_OK 成功，CAMERA_TIMEOUT 超时，CAMERA_CHECKSUM_ERR 校验错误
 */
uint8_t Camera_DeviceCTRL(uint8_t id, uint8_t dat1, uint8_t dat2, uint8_t *result);

/**
 * @brief 设备重启
 * 
 * @param id 摄像头设备 ID
 * @return 重启结果（0x01=成功，0x00=失败/超时）
 */
uint8_t Camera_Reset(uint8_t id);
```

**参数说明**：
- `id`: 摄像头设备 ID
- `dat1`: 控制项，可选值：
  - `CTRL_BRIGHTNESS` (0x02): 亮度
  - `CTRL_CONTRAST` (0x03): 对比度
  - `CTRL_EXPOSURE` (0x04): 曝光
  - `CTRL_WB` (0x05): 白平衡
  - `CTRL_SATURATION` (0x06): 饱和度
  - `CTRL_SHARPEN` (0x07): 锐化
- `dat2`: 控制参数值（0-100）
- `result`: 输出参数，存储控制结果

**返回值**：
- `CAMERA_OK` (0): 成功
- `CAMERA_TIMEOUT` (1): 超时
- `CAMERA_CHECKSUM_ERR` (2): 校验错误

---

## 使用示例

### 基本使用示例

```c
#include "camera_protocol.h"
#include "esp_log.h"

static const char *TAG = "CAMERA_EXAMPLE";

void camera_example_basic(void)
{
    // 1. 初始化摄像头协议
    Camera_Init();
    ESP_LOGI(TAG, "摄像头协议初始化完成");
    
    // 2. 切换到颜色识别模式
    uint8_t result;
    uint8_t ret = Camera_SetMode(0x01, FUNC_COLOR, &result);
    if (ret == CAMERA_OK && result == 0x01) {
        ESP_LOGI(TAG, "切换到颜色识别模式成功");
    } else {
        ESP_LOGW(TAG, "模式切换失败");
        return;
    }
    
    // 3. 读取非指定颜色识别
    uint8_t detected_color = Camera_ReadColorNonSpec(0x01);
    if (detected_color != 0x00) {
        switch (detected_color) {
            case COLOR_RED:
                ESP_LOGI(TAG, "检测到红色");
                break;
            case COLOR_GREEN:
                ESP_LOGI(TAG, "检测到绿色");
                break;
            case COLOR_BLUE:
                ESP_LOGI(TAG, "检测到蓝色");
                break;
            default:
                ESP_LOGI(TAG, "检测到其他颜色: 0x%02X", detected_color);
                break;
        }
    } else {
        ESP_LOGI(TAG, "未检测到颜色");
    }
}
```

### 指定颜色识别示例

```c
void camera_example_color_spec(void)
{
    // 检测是否有红色
    uint8_t result = Camera_ReadColorSpec(0x01, COLOR_RED);
    if (result == 0x01) {
        ESP_LOGI(TAG, "检测到红色");
        // 执行相应动作
    } else {
        ESP_LOGI(TAG, "未检测到红色");
    }
}
```

### 数字识别示例

```c
void camera_example_number(void)
{
    // 1. 切换到数字识别模式
    uint8_t result;
    Camera_SetMode(0x01, FUNC_NUMBER, &result);
    
    // 2. 读取非指定数字识别
    uint8_t detected_number = Camera_ReadNumberNonSpec(0x01);
    if (detected_number != 0x00) {
        ESP_LOGI(TAG, "检测到数字: %d", detected_number);
    } else {
        ESP_LOGI(TAG, "未检测到数字");
    }
}
```

### 设备控制示例

```c
void camera_example_device_ctrl(void)
{
    uint8_t result;
    
    // 调整亮度到 50
    uint8_t ret = Camera_DeviceCTRL(0x01, CTRL_BRIGHTNESS, 50, &result);
    if (ret == CAMERA_OK && result == 0x01) {
        ESP_LOGI(TAG, "亮度设置成功");
    }
    
    // 调整对比度到 60
    ret = Camera_DeviceCTRL(0x01, CTRL_CONTRAST, 60, &result);
    if (ret == CAMERA_OK && result == 0x01) {
        ESP_LOGI(TAG, "对比度设置成功");
    }
}
```

---

## 依赖

### 依赖的其他组件

- `uart`: UART 驱动组件
- `485servo`: RS485 发送函数（uart0_send）和互斥锁
- `board_config`: GPIO 引脚定义

### ESP-IDF 组件

- `driver`: GPIO 和 UART 驱动
- `freertos`: 任务管理和互斥锁
- `esp_log`: 日志输出

---

## 注意事项

### 硬件限制

- ⚠️ UART0 与 RS485 舵机共用，通过互斥锁保护，避免冲突
- ⚠️ RS485 总线最大支持 32 个设备（理论值，实际取决于驱动器）
- ⚠️ 长距离通信（>100m）需要添加终端电阻和中继器

### 软件限制

- ⚠️ 所有函数都是同步阻塞调用，会等待摄像头响应
- ⚠️ 超时时间默认 100ms，频繁调用会影响系统响应性
- ⚠️ 不能在中断服务程序中调用这些函数
- ⚠️ 二维码数据解析功能尚未实现

### 线程安全

- ✅ 所有函数内部使用互斥锁保护，支持多任务安全访问
- ✅ 互斥锁最大等待时间 200ms，超时会返回失败
- ⚠️ 避免在高优先级任务中长时间占用 RS485 总线

### 性能考虑

- 调用频率：建议低频调用（秒级），适合偶尔查询的场景
- 超时时间：默认 100ms，可在头文件中修改 `CAMERA_RX_TIMEOUT_MS`
- 日志输出：调试完成后建议将 `CAMERA_LOG_LEVEL` 改为 `ESP_LOG_WARN`

---

## 故障排除

### 常见问题

#### 问题1：函数返回 0x00（超时）

**现象**：调用识别函数总是返回 0x00，日志显示超时

**原因**：
- RS485 连接不正常
- 摄像头设备 ID 不正确
- 摄像头未上电或未初始化
- 波特率不匹配

**解决方案**：
1. 检查 RS485 接线是否正确（A-A, B-B）
2. 确认摄像头设备 ID（默认 0x01）
3. 确认摄像头已上电且工作正常
4. 检查波特率设置（默认 1000000）

#### 问题2：校验和错误

**现象**：日志显示 "checksum error"

**原因**：
- RS485 线路质量差，信号干扰
- 接地不良
- 波特率过高

**解决方案**：
1. 使用双绞线，减少干扰
2. 确保摄像头和 ESP32 共地
3. 尝试降低波特率
4. 添加终端电阻（120Ω）

#### 问题3：日志输出过多

**现象**：串口日志刷屏，影响调试

**原因**：日志级别设置为 INFO

**解决方案**：
1. 修改头文件中的 `CAMERA_LOG_LEVEL` 为 `ESP_LOG_WARN`
2. 或在运行时调用 `esp_log_level_set("CAMERA", ESP_LOG_WARN)`

#### 问题4：获取互斥锁超时

**现象**：日志显示 "获取RS485互斥锁超时"

**原因**：
- 其他任务正在使用 UART0（如 RS485 舵机）
- 任务优先级配置不当

**解决方案**：
1. 确保 RS485 舵机和摄像头不会同时频繁调用
2. 调整任务优先级，避免低优先级任务长时间占用
3. 增加互斥锁等待时间（修改代码中的 200ms）

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [GPIO引脚分配文档](../../docs/GPIO_PIN_ALLOCATION.md)
- [UART组件文档](../uart/README.md)
- [485servo组件文档](../485servo/README.md)
- [详细使用说明](USAGE.md)

### 数据手册

- 摄像头通信协议规范（参考 USAGE.md）
- RS485 总线标准（TIA/EIA-485）

### 代码示例

- 测试程序：`main/main.c`（摄像头识别功能）
- 示例代码：`components/camera_protocol/example_usage.c`

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，完整功能实现 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/camera_protocol/`  
**文档类型**: 组件使用说明
