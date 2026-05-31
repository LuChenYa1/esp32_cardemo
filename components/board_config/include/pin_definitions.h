/**
 * @file pin_definitions.h
 * @brief ESP32-S3 巡线小车引脚定义
 * 
 * 本文件定义了所有硬件接口的GPIO引脚分配
 * 
 * 硬件接口说明：
 * - SSA1-SSA6: 传感器扩展接口A组（Sensor Socket A）
 * - SSD3-SSD6: 传感器扩展接口D组（Sensor Socket D）
 * - SSA5/SSD5: 通过PCF8574 I2C扩展芯片提供
 * 
 * 引脚使用规则：
 * 1. 固定硬件引脚（电机PWM、编码器、UART、I2C、RS485）不能重复使用
 * 2. 传感器扩展接口可以连接不同传感器，但不能同时初始化
 * 3. gpio_manager会检测已初始化的引脚，防止冲突
 * 
 * 硬件版本: ESP32-S3
 * 文档参考: docs/GPIO_PIN_ALLOCATION.md
 */

#ifndef PIN_DEFINITIONS_H
#define PIN_DEFINITIONS_H

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

// ========================================
// 电机PWM控制（8个GPIO）
// ========================================
#define MOTOR1_FWD_GPIO          GPIO_NUM_2   // 电机1正向 LEDC_CH0
#define MOTOR1_REV_GPIO          GPIO_NUM_3   // 电机1反向 LEDC_CH1
#define MOTOR2_FWD_GPIO          GPIO_NUM_4   // 电机2正向 LEDC_CH2
#define MOTOR2_REV_GPIO          GPIO_NUM_5   // 电机2反向 LEDC_CH3
#define MOTOR3_FWD_GPIO          GPIO_NUM_6   // 电机3正向 LEDC_CH4
#define MOTOR3_REV_GPIO          GPIO_NUM_7   // 电机3反向 LEDC_CH5
#define MOTOR4_FWD_GPIO          GPIO_NUM_8   // 电机4正向 LEDC_CH6
#define MOTOR4_REV_GPIO          GPIO_NUM_9   // 电机4反向 LEDC_CH7

// ========================================
// 编码器PCNT（8个GPIO）
// ========================================
#define ENCODER1_A_GPIO          GPIO_NUM_41  // 编码器1 A相 PCNT单元0
#define ENCODER1_B_GPIO          GPIO_NUM_42  // 编码器1 B相 PCNT单元0
#define ENCODER2_A_GPIO          GPIO_NUM_45  // 编码器2 A相 PCNT单元1
#define ENCODER2_B_GPIO          GPIO_NUM_46  // 编码器2 B相 PCNT单元1
#define ENCODER3_A_GPIO          GPIO_NUM_14  // 编码器3 A相 PCNT单元2
#define ENCODER3_B_GPIO          GPIO_NUM_15  // 编码器3 B相 PCNT单元2
#define ENCODER4_A_GPIO          GPIO_NUM_16  // 编码器4 A相 PCNT单元3
#define ENCODER4_B_GPIO          GPIO_NUM_17  // 编码器4 B相 PCNT单元3

// ========================================
// LED指示灯
// ========================================
#define LED_GPIO                 GPIO_NUM_0   // LED指示灯

// ========================================
// 蜂鸣器
// ========================================
#define BUZZER_GPIO              GPIO_NUM_26  // 蜂鸣器输出

// ========================================
// UART0（日志输出 / RS485通信）
// ========================================
#define UART0_PORT               UART_NUM_0
#define UART0_TX_GPIO            GPIO_NUM_43  // UART0发送引脚
#define UART0_RX_GPIO            GPIO_NUM_44  // UART0接收引脚
#define UART0_BAUD_RATE          115200       // 默认波特率
#define UART0_TX_BUFFER_SIZE     1024         // 发送缓冲区大小
#define UART0_RX_BUFFER_SIZE     1024         // 接收缓冲区大小

// ========================================
// RS485方向控制
// ========================================
#define RS485_DIR_GPIO           GPIO_NUM_19  // RS485方向控制引脚
#define RS485_DIR_TX_LEVEL       1            // 发送模式电平
#define RS485_DIR_RX_LEVEL       0            // 接收模式电平

// ========================================
// UART1（外接串口模块：语音、蓝牙等）
// ========================================
#define UART1_PORT               UART_NUM_1
#define UART1_TX_GPIO            GPIO_NUM_35  // UART1发送引脚
#define UART1_RX_GPIO            GPIO_NUM_36  // UART1接收引脚
#define UART1_BAUD_RATE          115200       // 默认波特率
#define UART1_TX_BUFFER_SIZE     1024         // 发送缓冲区大小
#define UART1_RX_BUFFER_SIZE     1024         // 接收缓冲区大小

// ========================================
// I2C主总线（PCF8574扩展芯片）
// ========================================
#define I2C_MASTER_PORT          I2C_NUM_0
#define I2C_MASTER_SCL_GPIO      GPIO_NUM_21  // I2C时钟线
#define I2C_MASTER_SDA_GPIO      GPIO_NUM_20  // I2C数据线
#define I2C_MASTER_FREQ_HZ       100000       // I2C频率 100kHz

// ========================================
// 灰度传感器（2个GPIO - ADC输入）
// 接口：飞线连接（临时方案）
// 用途：巡线检测，读取黑白线位置
// 注意：
// - 当前使用ADC2通道，与WiFi冲突
// - GPIO20占用了I2C_SDA，导致PCF8574不可用
// - 推荐改用GPIO47/48（ADC1通道，SSA4/SSD4接口）
// ========================================
#define GRAY_SENSOR_LEFT_GPIO    GPIO_NUM_18  // 左灰度传感器 ADC2_CH7（临时飞线）
#define GRAY_SENSOR_RIGHT_GPIO   GPIO_NUM_20  // 右灰度传感器 ADC2_CH9（临时飞线，占用I2C_SDA）

// 推荐配置（需要硬件修改）：
// #define GRAY_SENSOR_LEFT_GPIO    GPIO_NUM_47  // 左灰度传感器 ADC1_CH6 [SSA4]
// #define GRAY_SENSOR_RIGHT_GPIO   GPIO_NUM_48  // 右灰度传感器 ADC1_CH7 [SSD4]

// ========================================
// 五路灰度传感器（I2C设备）
// 接口：复用二路灰度传感器引脚（GPIO18/20）
// 用途：高精度巡线检测，5个传感器提供更精确的位置信息
// 通信：I2C协议，地址0x4F，速率400kHz
// 注意：
// - 与二路ADC灰度传感器互斥使用（共用GPIO18/20）
// - 使用独立I2C总线（I2C_NUM_1）
// - 后台任务2ms周期自动更新数据
// ========================================
#define FIVE_WAY_GRAY_I2C_PORT      I2C_NUM_1       // I2C总线编号
#define FIVE_WAY_GRAY_I2C_SCL_GPIO  GPIO_NUM_18     // I2C时钟线（复用二路灰度左传感器引脚）
#define FIVE_WAY_GRAY_I2C_SDA_GPIO  GPIO_NUM_20     // I2C数据线（复用二路灰度右传感器引脚）
#define FIVE_WAY_GRAY_I2C_ADDR      0x4F            // 传感器I2C地址
#define FIVE_WAY_GRAY_I2C_FREQ_HZ   400000          // I2C频率 400kHz

// ========================================
// TM1637数码管显示（2个GPIO - 软件协议）
// 接口：SSA3（CLK）+ SSA2（DIO）
// 用途：显示温湿度、距离、状态等信息
// 注意：TM1637使用开漏输出
// ========================================
#define TM1637_CLK_GPIO          GPIO_NUM_34  // TM1637时钟线 [SSA3]
#define TM1637_DIO_GPIO          GPIO_NUM_37  // TM1637数据线 [SSA2]

// ========================================
// TM1640 LED点阵屏（2个GPIO - 软件协议）
// 接口：SSA3（CLK）+ SSA2（DIO）
// 用途：显示文字、图案、动画（8x16点阵）
// 注意：与TM1637复用相同引脚，不能同时使用
// ========================================
#define TM1640_CLK_GPIO          GPIO_NUM_34  // TM1640时钟线 [SSA3]
#define TM1640_DIN_GPIO          GPIO_NUM_37  // TM1640数据线 [SSA2]

// ========================================
// DHT11温湿度传感器（1个GPIO - 单总线协议）
// 接口：SSD3
// 用途：读取环境温度和湿度
// ========================================
#define DHT11_DATA_GPIO          GPIO_NUM_33  // DHT11数据线 [SSD3]

// ========================================
// 红外避障传感器（1个GPIO - 数字输入）
// 接口：SSA1
// 用途：检测前方障碍物（0=无障碍，1=有障碍）
// 注意：GPIO38只能作为输入
// ========================================
#define IR_OBSTACLE_GPIO         GPIO_NUM_38  // 红外避障传感器 [SSA1] (Input Only)

// ========================================
// 交通灯控制（2个GPIO - 数字输出）
// 接口：SSD6（信号1）+ SSA6（信号2）
// 用途：控制外部交通灯模块，每秒切换状态
// 注意：GPIO39/40可以作为输出使用
// ========================================
#define TRAFFIC_LIGHT_SIGNAL1_GPIO   GPIO_NUM_39  // 交通灯信号线1 [SSD6]
#define TRAFFIC_LIGHT_SIGNAL2_GPIO   GPIO_NUM_40  // 交通灯信号线2 [SSA6]

// ========================================
// 预留GPIO（未使用）
// 接口：SSA4 + SSD4
// 用途：可用于扩展其他传感器（如超声波、灰度传感器等）
// 特性：支持ADC1通道，与WiFi不冲突
// ========================================
#define RESERVED_GPIO_1          GPIO_NUM_47  // 预留GPIO [SSA4] ADC1_CH6
#define RESERVED_GPIO_2          GPIO_NUM_48  // 预留GPIO [SSD4] ADC1_CH7

// ========================================
// HC-SR04超声波测距传感器（2个GPIO）
// 接口：SSA4（ECHO）+ SSD4（TRIG）
// 用途：测量前方障碍物距离（2cm-400cm）
// ========================================
#define HCSR04_TRIG_GPIO         GPIO_NUM_48  // HC-SR04触发引脚 [SSD4]
#define HCSR04_ECHO_GPIO         GPIO_NUM_47  // HC-SR04回响引脚 [SSA4]

// ========================================
// VL53L0X激光测距传感器（未使用）
// 接口：SSA6（SCL）+ SSD6（SDA）
// 注意：当前项目不使用VL53L0X，GPIO39/40已用于交通灯控制
// ========================================
#define VL53L0X_SCL_GPIO         GPIO_NUM_40  // VL53L0X时钟线 [SSA6]（未使用）
#define VL53L0X_SDA_GPIO         GPIO_NUM_39  // VL53L0X数据线 [SSD6]（未使用）

// ========================================
// MPU6050陀螺仪（未使用）
// 接口：SSA6（SCL）+ SSD6（SDA）
// 注意：与VL53L0X和交通灯共用GPIO39/40，不能同时使用
// ========================================
#define MPU6050_I2C_PORT         I2C_NUM_1    // 使用I2C1（独立总线）
#define MPU6050_SCL_GPIO         GPIO_NUM_40  // MPU6050时钟线 [SSA6]（未使用）
#define MPU6050_SDA_GPIO         GPIO_NUM_39  // MPU6050数据线 [SSD6]（未使用）
#define MPU6050_I2C_ADDR         0x68         // MPU6050 I2C地址（AD0=0）
#define MPU6050_SCL_SPEED        100000       // I2C时钟频率 100kHz

// ========================================
// 硬件特性和限制说明
// ========================================
// 
// 【ADC通道限制】
// - ADC1（GPIO36-39, 47-48）: 可与WiFi同时使用
// - ADC2（GPIO0-20）: 与WiFi冲突，启用WiFi时ADC2不可用
// - 当前灰度传感器使用ADC2，如需WiFi请改用GPIO47/48
// 
// 【I2C总线冲突】
// - GPIO20被灰度传感器占用，PCF8574扩展芯片暂不可用
// - 如需使用PCF8574，请将灰度传感器改用GPIO47/48
// 
// 【传感器接口复用】
// - GPIO39/40可用于：VL53L0X、MPU6050、交通灯（三选一）
// - GPIO38可用于：DHT11、红外避障、触摸传感器（三选一）
// - 当前配置：GPIO39/40用于交通灯，GPIO38用于红外避障
// 
// 【接口标识说明】
// - [SSA1-SSA6]: 传感器扩展接口A组
// - [SSD3-SSD6]: 传感器扩展接口D组
// - [SSA5/SSD5]: 通过PCF8574 I2C扩展芯片提供（P6/P5）
// 
// 【当前使用的传感器模块（6个）】
// 1. 超声波传感器 (HC-SR04, GPIO47/48) - [SSA4/SSD4] 测距
// 2. 灰度传感器 (GPIO18/20) - 巡线核心功能
// 3. 数码管 (TM1637, GPIO34/37) - [SSA3/SSA2] 显示
// 4. DHT11温湿度 (GPIO33) - [SSD3] 温湿度检测
// 5. 红外避障 (GPIO38) - [SSA1] 障碍检测
// 6. 交通灯控制 (GPIO39/40) - [SSD6/SSA6] 信号控制

#ifdef __cplusplus
}
#endif

#endif // PIN_DEFINITIONS_H
