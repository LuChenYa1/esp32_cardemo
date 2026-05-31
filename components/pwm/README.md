# PWM 电机控制模块

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: main.c使用中

---

## 组件简介

PWM模块使用ESP32-S3的LEDC（LED PWM控制器）外设，为4个直流电机提供PWM控制信号。每个电机使用双PWM通道（正向/反向），通过调节占空比实现电机的正反转和速度控制。

### 主要特性

- 支持4个独立电机的PWM控制
- 每个电机双通道控制（正向/反向）
- 10位分辨率（0-1023级速度调节）
- 1kHz PWM频率，适合大多数直流电机
- 基于ESP32-S3硬件LEDC外设，性能稳定

### 适用场景

适用于四轮驱动小车、机器人底盘等需要多电机精确速度控制的应用场景。

---

## 硬件连接

### 引脚分配

| 功能 | GPIO引脚 | 接口标识 | LEDC通道 | 说明 |
|------|---------|---------|----------|------|
| 电机1正向 | GPIO2 | SSD1 | CH0 | 左前轮正转控制 |
| 电机1反向 | GPIO3 | SSD1 | CH1 | 左前轮反转控制 |
| 电机2正向 | GPIO4 | SSD2 | CH2 | 右前轮正转控制 |
| 电机2反向 | GPIO5 | SSD2 | CH3 | 右前轮反转控制 |
| 电机3正向 | GPIO6 | SSD3 | CH4 | 左后轮正转控制 |
| 电机3反向 | GPIO7 | SSD3 | CH5 | 左后轮反转控制 |
| 电机4正向 | GPIO8 | SSD4 | CH6 | 右后轮正转控制 |
| 电机4反向 | GPIO9 | SSD4 | CH7 | 右后轮反转控制 |

### 接线说明

1. 将电机驱动板的PWM输入端连接到对应的GPIO引脚
2. 确保电机驱动板与ESP32共地（GND连接）
3. 电机驱动板需要独立供电（通常5V-12V，根据电机规格）
4. ESP32仅提供PWM信号，不直接驱动电机

### 注意事项

- ⚠️ GPIO2-9已固定用于电机PWM，不能用于其他功能
- ⚠️ 避免同时设置正向和反向占空比为非零值，可能导致电机发热或损坏
- ⚠️ 确保电机驱动板供电充足，ESP32仅提供PWM信号
- ⚠️ PWM频率1kHz适合大多数直流电机，如需调整请修改`LEDC_FREQUENCY`宏定义

### PWM参数

| 参数名称 | 值 | 说明 |
|---------|-----|------|
| PWM频率 | 1000Hz (1kHz) | 适合大多数直流电机 |
| 占空比分辨率 | 10位 (0-1023) | 提供1024级速度控制 |
| 工作模式 | LEDC低速模式 | 稳定可靠 |
| 定时器 | LEDC_TIMER_0 | 共用一个定时器 |

---

## 功能说明

### 核心功能

#### 功能1：双PWM通道电机控制

每个电机使用两个独立的PWM通道实现正反转控制：
- **正向通道**：控制电机正转速度，占空比越大转速越快
- **反向通道**：控制电机反转速度，占空比越大转速越快

通过设置不同的占空比组合，实现以下运动模式：
- **正转**：正向PWM > 0，反向PWM = 0
- **反转**：正向PWM = 0，反向PWM > 0
- **停止**：正向PWM = 0，反向PWM = 0
- **刹车**：正向PWM = 最大值，反向PWM = 最大值（不推荐长时间使用）

#### 功能2：多电机独立控制

支持4个电机的独立速度和方向控制，可实现：
- 前进/后退：所有电机同向转动
- 左转/右转：左右侧电机反向转动
- 原地旋转：左右侧电机相反方向转动
- 差速转向：调整左右侧电机速度差

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| LEDC_FREQUENCY | 1000Hz | 100-5000Hz | PWM频率，影响电机噪音和效率 |
| LEDC_DUTY_RES | 10位 | 8-14位 | 占空比分辨率，影响速度控制精度 |
| PWM_MAX_DUTY | 1023 | 根据分辨率 | 最大占空比值（2^分辨率 - 1）|
| 占空比值 | 0-1023 | 0-1023 | 直接对应硬件分辨率，无需转换 |

---

## API接口

### 初始化函数

```c
/**
 * @brief 初始化LEDC外设和PWM通道
 * 
 * 配置LEDC定时器和8个PWM通道（4个电机×2通道）
 * 初始化后所有通道占空比为0（电机停止状态）
 * 
 * @return 无返回值
 */
void ledc_init(void);
```

**参数说明**：
- 无参数

**返回值**：
- 无返回值

**使用说明**：
- 必须在使用任何电机控制函数前调用
- 只需调用一次，初始化所有4个电机的PWM通道
- 初始化失败会通过ESP_ERROR_CHECK触发断言

---

### 电机控制函数

```c
/**
 * @brief 设置电机1的正向和反向PWM占空比
 * 
 * @param motor1_duty 正向占空比 (0-1023)
 * @param motor2_duty 反向占空比 (0-1023)
 * @return 无返回值
 */
void set_motor1_speed(uint16_t motor1_duty, uint16_t motor2_duty);

/**
 * @brief 设置电机2的正向和反向PWM占空比
 * 
 * @param motor1_duty 正向占空比 (0-1023)
 * @param motor2_duty 反向占空比 (0-1023)
 * @return 无返回值
 */
void set_motor2_speed(uint16_t motor1_duty, uint16_t motor2_duty);

/**
 * @brief 设置电机3的正向和反向PWM占空比
 * 
 * @param motor1_duty 正向占空比 (0-1023)
 * @param motor2_duty 反向占空比 (0-1023)
 * @return 无返回值
 */
void set_motor3_speed(uint16_t motor1_duty, uint16_t motor2_duty);

/**
 * @brief 设置电机4的正向和反向PWM占空比
 * 
 * @param motor1_duty 正向占空比 (0-1023)
 * @param motor2_duty 反向占空比 (0-1023)
 * @return 无返回值
 */
void set_motor4_speed(uint16_t motor1_duty, uint16_t motor2_duty);
```

**参数说明**：
- `motor1_duty`: 正向占空比，范围0-1023，直接对应硬件分辨率
- `motor2_duty`: 反向占空比，范围0-1023，直接对应硬件分辨率
- 超出范围的值会被自动限制在0-1023之间

**返回值**：
- 无返回值

**使用说明**：
- 占空比值直接对应硬件分辨率，无需百分比转换
- 占空比越大，电机转速越快
- 避免同时设置正向和反向为非零值
- 可以实时调用，立即生效

---

## 使用示例

### 示例1：基本初始化和电机控制

```c
#include "pwm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void) {
    // 初始化PWM
    ledc_init();
    
    // 电机1正转，速度50% (512/1023)
    set_motor1_speed(512, 0);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 电机1反转，速度30% (307/1023)
    set_motor1_speed(0, 307);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 电机1停止
    set_motor1_speed(0, 0);
}
```

### 示例2：四轮小车前进

```c
void car_forward(uint16_t speed) {
    // 所有电机正转
    set_motor1_speed(speed, 0);
    set_motor2_speed(speed, 0);
    set_motor3_speed(speed, 0);
    set_motor4_speed(speed, 0);
}

void car_backward(uint16_t speed) {
    // 所有电机反转
    set_motor1_speed(0, speed);
    set_motor2_speed(0, speed);
    set_motor3_speed(0, speed);
    set_motor4_speed(0, speed);
}

void car_stop(void) {
    // 所有电机停止
    set_motor1_speed(0, 0);
    set_motor2_speed(0, 0);
    set_motor3_speed(0, 0);
    set_motor4_speed(0, 0);
}

void app_main(void) {
    ledc_init();
    
    // 前进2秒，速度70%
    car_forward(716);  // 716 ≈ 1023 * 0.7
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 后退2秒，速度50%
    car_backward(512);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // 停止
    car_stop();
}
```

### 示例3：原地转向

```c
void car_turn_left(uint16_t speed) {
    // 左侧电机反转，右侧电机正转
    set_motor1_speed(0, speed);  // 左前反转
    set_motor2_speed(speed, 0);  // 右前正转
    set_motor3_speed(0, speed);  // 左后反转
    set_motor4_speed(speed, 0);  // 右后正转
}

void car_turn_right(uint16_t speed) {
    // 左侧电机正转，右侧电机反转
    set_motor1_speed(speed, 0);  // 左前正转
    set_motor2_speed(0, speed);  // 右前反转
    set_motor3_speed(speed, 0);  // 左后正转
    set_motor4_speed(0, speed);  // 右后反转
}
```

### 示例4：速度渐变控制

```c
void motor_ramp_up(void) {
    ledc_init();
    
    // 从0逐渐加速到最大速度
    for (uint16_t duty = 0; duty <= 1023; duty += 10) {
        set_motor1_speed(duty, 0);
        vTaskDelay(pdMS_TO_TICKS(50));  // 每50ms增加一次
    }
    
    // 从最大速度逐渐减速到0
    for (uint16_t duty = 1023; duty > 0; duty -= 10) {
        set_motor1_speed(duty, 0);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    set_motor1_speed(0, 0);
}
```

---

## 注意事项

### 硬件限制

- ⚠️ GPIO2-9已固定用于电机PWM，不能复用于其他功能
- ⚠️ 电机驱动板需要独立供电，ESP32仅提供PWM信号
- ⚠️ 必须确保ESP32与电机驱动板共地（GND连接）
- ⚠️ 电机驱动板的电源电压需匹配电机规格（通常5V-12V）

### 软件限制

- ⚠️ 必须先调用`ledc_init()`再使用电机控制函数
- ⚠️ 避免同时设置正向和反向占空比为非零值，可能导致电机发热或H桥短路
- ⚠️ 占空比范围0-1023，超出范围会被自动限制
- ⚠️ 修改PWM频率或分辨率需要重新编译代码

### 线程安全

- 电机控制函数是线程安全的，可以在多个任务中调用
- LEDC驱动内部使用硬件寄存器操作，不需要额外的互斥锁
- 建议在同一任务中集中管理电机控制逻辑，避免多任务冲突

### 性能考虑

- PWM频率1kHz适合大多数直流电机，更高频率会增加开关损耗
- 10位分辨率提供1024级速度控制，满足大多数应用需求
- 更高分辨率会降低最大PWM频率（硬件限制）
- 电机启动需要最小占空比（通常10-20%），低于此值电机可能不转

---

## 故障排除

### 常见问题

#### 问题1：电机不转或转速不稳定

**现象**：调用电机控制函数后，电机没有反应或转速忽快忽慢

**原因**：
- 电机驱动板供电不足或未供电
- PWM信号线连接错误或接触不良
- 占空比设置过低，低于电机启动阈值
- 电机驱动板与ESP32未共地

**解决方案**：
1. 检查电机驱动板电源指示灯是否亮起
2. 使用万用表测量驱动板供电电压是否正常
3. 检查PWM信号线连接是否正确（参考引脚分配表）
4. 尝试增大占空比到200以上（约20%）
5. 确保ESP32的GND与驱动板的GND连接

#### 问题2：电机发热严重

**现象**：电机运行一段时间后温度明显升高

**原因**：
- 同时设置了正向和反向占空比为非零值
- 电机长时间高速运行
- 电机驱动板电流过大
- 电机负载过重

**解决方案**：
1. 检查代码，确保正向和反向占空比不会同时为非零
2. 降低占空比，减少电机转速
3. 增加电机运行间隔，避免长时间连续运行
4. 检查机械结构，减少电机负载

#### 问题3：如何将百分比转换为占空比？

**现象**：需要使用百分比（0-100%）控制电机速度

**原因**：API使用硬件占空比值（0-1023），不是百分比

**解决方案**：
使用公式：`duty = (1023 * percent) / 100`
- 例如：50% = 512，70% = 716，100% = 1023

#### 问题4：可以修改PWM频率吗？

**现象**：需要调整PWM频率以适配特殊电机

**原因**：默认频率1kHz不适合某些电机

**解决方案**：
1. 修改`pwm.h`中的`LEDC_FREQUENCY`宏定义
2. 注意：频率越高，分辨率越低（硬件限制）
3. 推荐范围：500Hz-5000Hz
4. 修改后需要重新编译代码

---

## 相关文档

- [项目根目录README](../../README.md)
- [GPIO引脚分配文档](../../docs/GPIO_PIN_ALLOCATION.md)
- [配置参数指南](../../docs/CONFIGURATION_GUIDE.md)
- [引脚定义头文件](../board_config/include/pin_definitions.h)
- [编码器模块](../encoder/README.md) - 配合PWM实现闭环控制

### 数据手册

- [ESP-IDF LEDC文档](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/ledc.html)
- ESP32-S3技术参考手册 - LEDC章节

### 代码示例

- 测试程序：`main/main.c` - 包含完整的电机控制示例

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，基于模板创建完整文档 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/pwm/`  
**文档类型**: 组件使用说明