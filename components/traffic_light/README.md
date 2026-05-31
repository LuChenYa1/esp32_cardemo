# 红绿灯控制模块 (Traffic Light)

## 元信息

- **版本**: 1.0.0
- **更新日期**: 2024-12-20
- **维护者**: 项目团队
- **使用状态**: ✅ main.c使用中

---

## 组件简介

红绿灯控制模块使用两根信号线控制三个灯（红、黄、绿），通过不同的信号组合实现三种灯光状态的切换。该模块采用简单的二进制编码方式，用最少的GPIO引脚实现多状态控制。

### 主要特性

- 使用2根GPIO信号线控制3种灯光状态
- 二进制编码控制逻辑（10=红灯，01=黄灯，11=绿灯）
- 固定引脚分配，避免引脚冲突
- 状态查询功能，支持获取当前灯光状态
- 初始化检查机制，防止未初始化使用

### 适用场景

适用于需要模拟交通信号灯的场景，如智能小车路口识别、交通灯识别训练、自动驾驶测试等应用。

---

## 硬件连接

### 引脚分配

| 功能 | GPIO引脚 | 接口标识 | 说明 |
|------|---------|---------|------|
| 信号线1 | GPIO39 | - | 控制信号高位 |
| 信号线2 | GPIO40 | - | 控制信号低位 |

### 控制逻辑

| 状态 | 信号线1 | 信号线2 | 二进制编码 |
|------|---------|---------|-----------|
| 红灯 | 高电平(1) | 低电平(0) | 10 |
| 黄灯 | 低电平(0) | 高电平(1) | 01 |
| 绿灯 | 高电平(1) | 高电平(1) | 11 |

### 接线说明

1. 将红绿灯模块的信号线1连接到ESP32的GPIO39
2. 将红绿灯模块的信号线2连接到ESP32的GPIO40
3. 确保红绿灯模块的电源和地线正确连接
4. 检查GPIO39/40没有被其他模块占用

### 注意事项

- ⚠️ **固定引脚**：引脚分配已固定在pin_definitions.h中，不可随意更改
- ⚠️ **电源要求**：确保红绿灯模块的工作电压与ESP32输出电平兼容

---

## 功能说明

### 核心功能

#### 功能1：红绿灯初始化

初始化GPIO39和GPIO40为输出模式，配置为推挽输出，不使用上拉/下拉电阻。初始化成功后默认设置为红灯状态。

#### 功能2：灯光状态控制

通过设置两根信号线的高低电平组合，实现三种灯光状态的切换：
- 红灯（10）：信号线1=高，信号线2=低
- 黄灯（01）：信号线1=低，信号线2=高
- 绿灯（11）：信号线1=高，信号线2=高

#### 功能3：状态查询

提供当前灯光状态的查询功能，返回当前是红灯、黄灯还是绿灯。

### 配置参数

| 参数名称 | 默认值 | 取值范围 | 说明 |
|---------|--------|---------|------|
| TRAFFIC_LIGHT_SIGNAL1_GPIO | GPIO39 | 固定 | 信号线1引脚（在pin_definitions.h中定义） |
| TRAFFIC_LIGHT_SIGNAL2_GPIO | GPIO40 | 固定 | 信号线2引脚（在pin_definitions.h中定义） |
| 初始状态 | 红灯 | 红/黄/绿 | 初始化后的默认状态 |

---

## API接口

### 初始化函数

```c
/**
 * @brief 初始化红绿灯GPIO引脚
 * 
 * @return ESP_OK 成功，ESP_FAIL 失败
 */
esp_err_t traffic_light_init(void);
```

**参数说明**：
- 无

**返回值**：
- `ESP_OK`: 初始化成功
- `ESP_FAIL`: GPIO配置失败

**使用说明**：
在使用红绿灯控制功能前必须先调用此函数进行初始化。初始化成功后会自动设置为红灯状态。

---

### 状态设置函数

```c
/**
 * @brief 设置红绿灯状态
 * 
 * @param state 目标状态（红/黄/绿）
 * @return ESP_OK 成功，ESP_ERR_INVALID_STATE 未初始化
 */
esp_err_t traffic_light_set_state(traffic_light_state_t state);
```

**参数说明**：
- `state`: 目标灯光状态
  - `TRAFFIC_LIGHT_RED`: 红灯
  - `TRAFFIC_LIGHT_YELLOW`: 黄灯
  - `TRAFFIC_LIGHT_GREEN`: 绿灯

**返回值**：
- `ESP_OK`: 设置成功
- `ESP_ERR_INVALID_STATE`: 红绿灯未初始化

**使用说明**：
调用此函数前必须先初始化红绿灯模块。函数会根据目标状态设置对应的GPIO电平组合。

---

### 状态查询函数

```c
/**
 * @brief 获取当前红绿灯状态
 * 
 * @return 当前状态
 */
traffic_light_state_t traffic_light_get_state(void);
```

**参数说明**：
- 无

**返回值**：
- `TRAFFIC_LIGHT_RED`: 当前为红灯
- `TRAFFIC_LIGHT_YELLOW`: 当前为黄灯
- `TRAFFIC_LIGHT_GREEN`: 当前为绿灯

**使用说明**：
可随时调用此函数查询当前灯光状态，无需初始化检查。

---

## 使用示例

### 基本使用示例

```c
#include "traffic_light.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "TRAFFIC_LIGHT_EXAMPLE";

void traffic_light_example(void)
{
    // 1. 初始化红绿灯模块
    esp_err_t ret = traffic_light_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "红绿灯初始化失败");
        return;
    }
    ESP_LOGI(TAG, "红绿灯初始化成功");
    
    // 2. 循环切换灯光状态
    while (1) {
        // 红灯
        traffic_light_set_state(TRAFFIC_LIGHT_RED);
        ESP_LOGI(TAG, "当前状态: 红灯");
        vTaskDelay(pdMS_TO_TICKS(3000));  // 持续3秒
        
        // 黄灯
        traffic_light_set_state(TRAFFIC_LIGHT_YELLOW);
        ESP_LOGI(TAG, "当前状态: 黄灯");
        vTaskDelay(pdMS_TO_TICKS(1000));  // 持续1秒
        
        // 绿灯
        traffic_light_set_state(TRAFFIC_LIGHT_GREEN);
        ESP_LOGI(TAG, "当前状态: 绿灯");
        vTaskDelay(pdMS_TO_TICKS(3000));  // 持续3秒
    }
}
```

### 状态查询示例

```c
void check_traffic_light_state(void)
{
    // 查询当前状态
    traffic_light_state_t current_state = traffic_light_get_state();
    
    switch (current_state) {
        case TRAFFIC_LIGHT_RED:
            ESP_LOGI(TAG, "当前是红灯，请停车");
            break;
        case TRAFFIC_LIGHT_YELLOW:
            ESP_LOGI(TAG, "当前是黄灯，请减速");
            break;
        case TRAFFIC_LIGHT_GREEN:
            ESP_LOGI(TAG, "当前是绿灯，可以通行");
            break;
    }
}
```

---

## 注意事项

### 硬件限制

- ⚠️ **固定引脚**：引脚已在pin_definitions.h中固定定义，修改需同步更新
- ⚠️ **电平兼容**：确保红绿灯模块支持3.3V电平信号

### 软件限制

- ⚠️ **初始化检查**：必须先调用traffic_light_init()才能使用其他功能
- ⚠️ **状态保存**：当前状态保存在静态变量中，重启后会丢失

### 线程安全

- 该模块不是线程安全的，如果多个任务需要控制红绿灯，需要添加互斥锁保护
- 建议在单一任务中控制红绿灯状态

### 性能考虑

- GPIO操作非常快速，状态切换延迟可忽略不计
- 如需精确的时序控制，建议使用定时器触发状态切换

---

## 故障排除

### 常见问题

#### 问题1：初始化失败

**现象**：调用traffic_light_init()返回ESP_FAIL

**原因**：GPIO配置失败，可能是引脚被其他模块占用

**解决方案**：
2. 查看串口日志，确认具体是哪个GPIO配置失败
3. 使用GPIO管理器检查引脚分配情况

#### 问题2：灯光不亮或状态错误

**现象**：设置状态后灯光不亮或显示错误的颜色

**原因**：硬件连接错误或电源问题

**解决方案**：
1. 检查GPIO39/40的物理连接是否正确
2. 使用万用表测量GPIO输出电平是否正确
3. 检查红绿灯模块的电源供电是否正常
4. 确认红绿灯模块的控制逻辑与代码一致

#### 问题3：设置状态返回ESP_ERR_INVALID_STATE

**现象**：调用traffic_light_set_state()返回错误

**原因**：红绿灯模块未初始化

**解决方案**：
1. 确保在调用traffic_light_set_state()前已调用traffic_light_init()
2. 检查初始化是否成功（返回ESP_OK）

---

## 参考资料

### 相关文档

- [项目根目录README](../../README.md)
- [GPIO引脚分配文档](../../docs/GPIO_PIN_ALLOCATION.md)
- [引脚定义头文件](../board_config/include/pin_definitions.h)

### 代码示例

- 测试程序：`main/main.c` - 包含红绿灯控制的完整示例

### 相关组件

- [board_config](../board_config/README.md) - 板级配置和引脚定义
- [gpio_manager](../board_config/README.md) - GPIO管理器

---

## 版本历史

| 版本 | 日期 | 变更内容 | 维护者 |
|------|------|---------|--------|
| 1.0.0 | 2024-12-20 | 初始版本，完整功能实现 | 项目团队 |

---

**项目**: ESP32模块集成项目  
**组件路径**: `components/traffic_light/`  
**文档类型**: 组件使用说明
