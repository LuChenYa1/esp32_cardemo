# Camera Protocol 修改记录

## 2024 年优化版本

### 主要改进

#### 1. 接口设计优化
- ✅ 所有函数改为直接返回数据值的设计（性能优化）
- ✅ 返回 0x00 表示失败/超时，0x01-0xFE 表示成功/数据
- ✅ 避免使用指针参数，减少内存操作开销
- ✅ 接口简洁直观，易于使用

#### 2. 日志系统改进
- ✅ 使用 ESP-IDF 标准日志系统（ESP_LOGI/ESP_LOGW/ESP_LOGE）
- ✅ 添加日志级别控制宏 `CAMERA_LOG_LEVEL`
- ✅ 支持运行时动态调整日志级别
- ✅ 替换所有 `printf()` 为结构化日志

#### 3. 代码清理
- ✅ 删除废弃的 `Camera_DeviceControl()` 函数
- ✅ 删除注释掉的无用代码
- ✅ 添加中文注释说明预留功能（环形缓冲区）
- ✅ 统一代码风格和注释格式

#### 4. 魔法数字优化
- ✅ 定义 `CAMERA_FRAME_MAX_SIZE` 宏（16）
- ✅ 定义 `CAMERA_DATA_OFFSET` 宏（2）
- ✅ 所有硬编码数字改为宏定义

#### 5. 初始化函数
- ✅ 添加 `Camera_Init()` 函数
- ✅ 自动调用 `servo485_init()` 初始化 UART 和 GPIO
- ✅ 自动设置日志级别

#### 6. 依赖关系明确
- ✅ CMakeLists.txt 添加对 `uart` 和 `485servo` 的依赖
- ✅ 删除空的 `example_usage.c` 文件
- ✅ 头文件添加依赖说明注释

#### 7. 错误处理改进
- ✅ 所有函数都有校验和检查
- ✅ 超时和校验错误都有日志输出
- ✅ 返回值统一为 0x00 表示失败

#### 8. 文档完善
- ✅ 创建 `USAGE.md` 使用说明文档
- ✅ 创建 `CHANGELOG.md` 修改记录
- ✅ 所有函数添加完整的 Doxygen 注释

### 函数接口设计

**最终采用的设计（直接返回值）：**
```c
uint8_t Camera_ReadColorSpec(uint8_t id, uint8_t color);
// 返回值：0x00=失败/超时，0x01=检测到，0x02-0xFE=其他数据
```

**优点：**
- 性能好，无指针操作开销
- 接口简洁，易于使用
- 适合低频调用场景

### 修改的函数列表

1. `Camera_Init()` - 新增初始化函数
2. `Camera_Reset()` - 直接返回结果值
3. `Camera_SetMode()` - 直接返回结果值
4. `Camera_DeviceCTRL()` - 直接返回结果值
5. `Camera_ReadColorSpec()` - 直接返回结果值
6. `Camera_ReadColorNonSpec()` - 直接返回检测到的颜色
7. `Camera_ReadFaceSpec()` - 直接返回结果值
8. `Camera_ReadFaceNonSpec()` - 直接返回检测到的人脸 ID
9. `Camera_ReadNumberSpec()` - 直接返回结果值
10. `Camera_ReadNumberNonSpec()` - 直接返回检测到的数字
11. `Camera_ReadLabelSpec()` - 直接返回结果值
12. `Camera_ReadLabelNonSpec()` - 直接返回检测到的标签
13. `Camera_ReadQR_Code()` - 直接返回检测结果

### 删除的函数

- `Camera_DeviceControl()` - 旧版本函数，已被 `Camera_DeviceCTRL()` 替代

### 内部函数改进

- `SendFrame()` - 改为 static，添加完整注释
- `RS485_WaitFrameById()` - 改为 static，添加完整注释
- `RS485_GetFrame()` - 改为 static，添加预留说明
- `GetByteFromRxBuf()` - 添加预留说明
- `CalcChecksum()` - 添加完整注释

### 使用示例

```c
// 初始化
Camera_Init();

// 模式切换
uint8_t result = Camera_SetMode(0x01, FUNC_COLOR);
if (result == 0x01) {
    ESP_LOGI("APP", "模式切换成功");
}

// 颜色识别
uint8_t color = Camera_ReadColorNonSpec(0x01);
if (color == COLOR_RED) {
    ESP_LOGI("APP", "检测到红色");
} else if (color == 0x00) {
    ESP_LOGW("APP", "未检测到或超时");
}
```

### 性能说明

- 调用频率：低频调用（秒级）
- 通信方式：同步阻塞
- 超时时间：100ms
- 适用场景：偶尔查询的应用

### 待实现功能

- [ ] 二维码数据解析（`Camera_ReadQR_Code` 中的 qr_buf 参数）
- [ ] 中断接收模式（环形缓冲区相关代码已预留）
- [ ] 批量读取功能
- [ ] 异步回调机制

### 测试建议

1. 测试所有识别功能的正常流程
2. 测试超时处理（断开摄像头连接）
3. 测试校验错误处理（模拟干扰）
4. 测试日志级别控制
5. 验证初始化函数是否正常工作
6. 性能测试（连续调用的响应时间）
