# 485Servo RS485舵机控制模块

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: 未使用

---

## 组件简介

485Servo模块用于通过RS485总线控制串行总线舵机。支持位置控制、速度控制、位置读取等功能。适用于机械臂、云台、多自由度机器人等应用。

### 主要特性

- 支持RS485总线通信，最多254个舵机
- 位置控制：设置舵机目标位置和运动速度
- 位置读取：查询舵机当前位置
- 高速通信：1Mbps波特率
- 半双工通信：自动方向控制

### 适用场景

适用于需要多舵机协同控制的应用场景，如机械臂、云台、多自由度机器人等。

---

## 概述

485Servo模块用于通过RS485总线控制串行总线舵机。支持位置控制、速度控制、位置读取等功能。适用于机械臂、云台、多自由度机器人等应用。

## 硬件连接

### 引脚定义

| 功能 | GPIO编号 | 说明 |
|------|---------|------|
| UART0 TX | GPIO43 | RS485数据发送 |
| UART0 RX | GPIO44 | RS485数据接收 |
| RS485 DIR | GPIO19 | 方向控制（高=发送，低=接收） |

### RS485总线连接

```
ESP32-S3 (GPIO43/44/19) <---> RS485芯片 <---> 舵机1 <---> 舵机2 <---> ...
```

注意：
- 总线末端需要120Ω终端电阻
- 最多支持254个舵机（ID: 1-254）
- 推荐总线长度 < 100米

## 功能说明

### 舵机协议

本模块使用标准RS485舵机协议：
- 波特率：1000000 (1Mbps)
- 数据格式：8N1（8位数据，无校验，1位停止位）
- 通信方式：半双工
- 校验方式：异或校验

### 支持的功能

1. **位置控制**：设置舵机目标位置和运动速度
2. **位置读取**：查询舵机当前位置
3. **多舵机控制**：通过ID区分不同舵机

## API 接口

### `servo485_init()`
初始化485舵机模块。

**参数：**
- 无参数

**返回值：**
- 无返回值

**说明：**
- 配置UART0为1Mbps
- 初始化RS485方向控制
- 设置初始状态为接收模式

### `servo485_deinit()`
反初始化485舵机模块。

**参数：**
- 无参数

**返回值：**
- 无返回值

**说明：**
- 释放UART和GPIO资源

### `Set_Servo_position()`
设置舵机位置。

**参数：**
- `id`: 舵机ID（1-254）
- `position`: 目标位置（0-1000，对应0°-240°）
- `speed`: 运动速度（0-255，0=最快）

**返回值：**
- 无返回值

**说明：**
- 位置范围：0-1000（具体角度范围取决于舵机型号）
- 速度值越小，运动越快

### `Read_Servo_position()`
读取舵机当前位置。

**参数：**
- `id`: 舵机ID（1-254）

**返回值：**
- 无返回值（通过UART接收响应数据）

**说明：**
- 发送查询命令后需要接收响应
- 响应数据包含当前位置信息

## 使用示例

### 示例1：基本初始化和位置控制

```c
#include "485servo.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    // 初始化485舵机
    servo485_init();
    
    // 控制ID=1的舵机移动到位置500，速度100
    Set_Servo_position(1, 500, 100);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 控制ID=1的舵机移动到位置800，速度50
    Set_Servo_position(1, 800, 50);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 读取ID=1的舵机当前位置
    Read_Servo_position(1);
}
```

### 示例2：多舵机控制

```c
#include "485servo.h"

void multi_servo_demo(void) {
    servo485_init();
    
    // 控制3个舵机同时运动
    Set_Servo_position(1, 500, 100);  // 舵机1
    vTaskDelay(pdMS_TO_TICKS(10));    // 短延时，避免总线冲突
    
    Set_Servo_position(2, 600, 100);  // 舵机2
    vTaskDelay(pdMS_TO_TICKS(10));
    
    Set_Servo_position(3, 700, 100);  // 舵机3
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 读取所有舵机位置
    Read_Servo_position(1);
    vTaskDelay(pdMS_TO_TICKS(50));
    
    Read_Servo_position(2);
    vTaskDelay(pdMS_TO_TICKS(50));
    
    Read_Servo_position(3);
}
```

### 示例3：舵机扫描运动

```c
#include "485servo.h"

void servo_sweep_task(void *pvParameters) {
    servo485_init();
    
    while (1) {
        // 从0扫描到1000
        for (uint16_t pos = 0; pos <= 1000; pos += 50) {
            Set_Servo_position(1, pos, 50);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        // 从1000扫描回0
        for (uint16_t pos = 1000; pos > 0; pos -= 50) {
            Set_Servo_position(1, pos, 50);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}
```

### 示例4：机械臂控制

```c
#include "485servo.h"

// 机械臂关节定义
#define SERVO_BASE      1  // 底座旋转
#define SERVO_SHOULDER  2  // 肩关节
#define SERVO_ELBOW     3  // 肘关节
#define SERVO_WRIST     4  // 腕关节
#define SERVO_GRIPPER   5  // 夹爪

void robot_arm_init(void) {
    servo485_init();
    
    // 初始化所有关节到中间位置
    Set_Servo_position(SERVO_BASE, 500, 100);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    Set_Servo_position(SERVO_SHOULDER, 500, 100);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    Set_Servo_position(SERVO_ELBOW, 500, 100);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    Set_Servo_position(SERVO_WRIST, 500, 100);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    Set_Servo_position(SERVO_GRIPPER, 300, 100);  // 夹爪打开
}

void robot_arm_grab(void) {
    // 移动到目标位置
    Set_Servo_position(SERVO_BASE, 600, 80);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    Set_Servo_position(SERVO_SHOULDER, 700, 80);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    Set_Servo_position(SERVO_ELBOW, 400, 80);
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // 夹取物体
    Set_Servo_position(SERVO_GRIPPER, 700, 50);  // 夹爪闭合
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // 抬起
    Set_Servo_position(SERVO_SHOULDER, 500, 80);
}
```

### 示例5：位置反馈控制

```c
#include "485servo.h"
#include "uart.h"

void servo_position_feedback(void) {
    servo485_init();
    
    // 设置目标位置
    Set_Servo_position(1, 800, 100);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 等待舵机运动完成
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // 读取实际位置
    Read_Servo_position(1);
    
    // 接收位置反馈数据
    uint8_t response[16];
    int len = uart0_recv(response, sizeof(response), pdMS_TO_TICKS(100));
    
    if (len > 0) {
        // 解析位置数据（根据舵机协议）
        uint16_t current_pos = (response[5] << 8) | response[6];
        ESP_LOGI("SERVO", "当前位置: %d", current_pos);
    }
}
```

## 依赖

- uart 组件（RS485通信）
- board_config 组件（引脚定义）
- ESP-IDF driver 组件（UART驱动）
- FreeRTOS（任务延时）

## 注意事项

1. **舵机ID**: 每个舵机必须有唯一的ID（1-254）
2. **总线冲突**: 多舵机控制时需要间隔10ms以上
3. **位置范围**: 不同型号舵机位置范围可能不同，注意查看数据手册
4. **速度参数**: 速度值0表示最快，255表示最慢
5. **电源要求**: 舵机需要独立供电（通常5V-12V），不能由ESP32供电
6. **终端电阻**: 总线末端需要120Ω终端电阻
7. **总线长度**: 推荐 < 100米，过长会导致通信不稳定
8. **多任务保护**: 多任务访问时使用`rs485_send_protected()`

## 常见问题

### Q: 舵机不响应？
A: 检查：
   - 舵机ID是否正确
   - 舵机供电是否正常
   - RS485接线是否正确（A-A, B-B）
   - 波特率是否匹配（1Mbps）
   - 终端电阻是否连接

### Q: 舵机抖动或运动不稳定？
A: 可能原因：
   - 供电不足，使用大容量电源
   - 总线干扰，检查接线质量
   - 通信速率过高，降低波特率
   - 多舵机同时控制，增加延时间隔

### Q: 如何设置舵机ID？
A: 使用舵机厂商提供的调试软件设置ID，或参考舵机协议发送ID设置命令。

### Q: 位置值如何对应角度？
A: 取决于舵机型号，常见对应关系：
   - 0-1000 → 0°-240°
   - 0-4095 → 0°-360°
   具体请查看舵机数据手册

### Q: 如何实现同步控制？
A: 使用舵机的同步写入命令（SYNC_WRITE），一次发送多个舵机的目标位置。

## 相关文档

- [引脚定义](../board_config/include/pin_definitions.h)
- [UART模块](../uart/README.md) - RS485通信基础
- [舵机任务模块](../servo_task/README.md) - 高级舵机控制
- 舵机数据手册（根据具体型号查阅）



---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 添加元信息和版本历史 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/485servo/`  
**文档类型**: 组件使用说明
