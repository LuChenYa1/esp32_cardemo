#ifndef VL53L0_H
#define VL53L0_H

#include "driver/i2c.h"  // ESP32 I2C驱动库头文件
#include "esp_err.h"     // ESP32错误代码定义头文件

// 默认I2C设备地址：VL53L0X传感器的标准地址为0x29
#define GIr_distance_sensor_DEF_I2C_ADDR 0x29

// 寄存器地址定义 - 用于访问VL53L0X内部寄存器
#define GIr_distance_sensor_REG_IDENTIFICATION_MODEL_ID       0x00c0  // 设备型号识别寄存器地址
#define GIr_distance_sensor_REG_IDENTIFICATION_REVISION_ID    0x00c2  // 修订版本识别寄存器地址
#define GIr_distance_sensor_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD 0x0050  // 预范围配置VCSEL周期寄存器地址
#define GIr_distance_sensor_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD 0x0070  // 最终范围配置VCSEL周期寄存器地址
#define GIr_distance_sensor_REG_SYSRANGE_START                0x0000  // 系统测距启动寄存器地址
#define GIr_distance_sensor_REG_RESULT_INTERRUPT_STATUS       0x0013  // 中断状态结果寄存器地址
#define GIr_distance_sensor_REG_RESULT_RANGE_STATUS           0x0014  // 范围状态结果寄存器地址
#define GIr_distance_sensor_REG_I2C_SLAVE_DEVICE_ADDRESS      0x008a  // I2C从设备地址寄存器地址
#define GIr_distance_sensor_REG_SYSTEM_RANGE_CONFIG           0x0009  // 系统范围配置寄存器地址
#define GIr_distance_sensor_REG_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV 0x0089  // 高压电源配置寄存器地址
#define GIr_distance_sensor_REG_SYSRANGE_MODE_SINGLESHOT      0x0000  // 单次测距模式值
#define GIr_distance_sensor_REG_SYSRANGE_MODE_START_STOP      0x0001  // 启停模式值
#define GIr_distance_sensor_REG_SYSRANGE_MODE_BACKTOBACK      0x0002  // 连续测距模式值
#define GIr_distance_sensor_REG_SYSRANGE_MODE_TIMED           0x0004  // 定时测距模式值

// 设备模式定义 - 用于设置传感器工作模式
#define GIr_distance_sensor_DEVICEMODE_SINGLE_RANGING         0  // 单次测距模式
#define GIr_distance_sensor_DEVICEMODE_CONTINUOUS_RANGING     1  // 连续测距模式
#define GIr_distance_sensor_DEVICEMODE_CONTINUOUS_TIMED_RANGING 3  // 定时连续测距模式
#define GIr_distance_sensor_DEFAULT_MAX_LOOP                  200  // 默认最大循环次数

// ESD保护宏定义：启用2.8V ESD保护
#define ESD_2V8

// 功能状态枚举：用于启用或禁用特定功能
typedef enum {
    edisable = 0,  // 禁用状态
    eENABLE = 1    // 启用状态
} eFunctionalState;

// 精度状态枚举：用于设置传感器测量精度
typedef enum {
    eHigh = 0,     // 高精度模式
    eLow = 1       // 低精度模式（普通精度）
} ePrecisionState;

// 模式状态枚举：用于设置传感器工作模式
typedef enum {
    eSingle = 0,     // 单次测量模式
    eContinuous = 1  // 连续测量模式
} eModeState;

// 传感器详细数据结构体：用于存储传感器的各种测量数据
typedef struct {
    uint8_t i2cDevAddr;      // I2C设备地址
    uint8_t mode;            // 当前工作模式
    uint8_t precision;       // 精度设置
    uint8_t originalData[16]; // 原始数据缓冲区（16字节）
    uint16_t ambientCount;   // 环境光计数值
    uint16_t signalCount;    // 信号计数值
    uint16_t distance;       // 测量距离值（毫米）
    uint8_t status;          // 测量状态信息
} sGIr_distance_sensor_DetailedData_t;

// 函数声明 - 延时和GPIO控制函数
/**
 * @brief 微秒级延时函数
 * @param us 要延时的微秒数
 */
void Distance_DelayUs(uint16_t us);      

/**
 * @brief 初始化距离传感器相关的GPIO引脚
 */
void Distance_IO_Init(void);             

/**
 * @brief 将SDA引脚设置为输入模式
 */
void Distance_SDA_SetInput(void);         

/**
 * @brief 将SDA引脚设置为输出模式
 */
void Distance_SDA_SetOutput(void);        

// 传感器主要功能函数声明
/**
 * @brief 初始化VL53L0X距离传感器
 * 完成传感器的基本配置和校准
 */
void Gir_distance_sensor_init(void);

/**
 * @brief 设置传感器工作模式
 * @param precision 精度模式（高精度或普通精度）
 */
void Gir_setMode(ePrecisionState precision);

/**
 * @brief 获取当前距离测量值
 * @return 距离值（单位：厘米）
 */
float getDistance(void);

#endif /* VL53L0_H */