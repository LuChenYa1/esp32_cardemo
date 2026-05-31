/**
 * @file vl53l0.c
 * @brief VL53L0X激光测距传感器驱动实现
 * 
 * 功能：
 * - 激光飞行时间(ToF)测距
 * - 测量范围：2cm-200cm
 * - I2C通信接口
 * - 高精度和普通精度模式
 * 
 * 注意：
 * - 需要配置I2C引脚
 * - 传感器地址默认为0x29
 */

#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "string.h"
#include "vl53l0.h"
#include "vl53l0_i2c.h"
#include "pin_definitions.h"
#include "driver/gpio.h"

// 静态全局变量：存储传感器详细数据结构体
static sGIr_distance_sensor_DetailedData_t _detailedData;
// 静态全局变量：存储上次测量的距离值
static uint16_t _distance;

// 函数声明 - 内部辅助函数
static void writeByteData(uint8_t Reg, uint8_t byte);          // 向指定寄存器写入单字节数据
static uint8_t readByteData(uint8_t Reg);                      // 从指定寄存器读取单字节数据
static void writeData(uint8_t Reg, uint8_t *buf, uint8_t Num); // 向连续寄存器写入多字节数据
static void readData(uint8_t Reg, uint8_t Num);                // 从连续寄存器读取多字节数据
static void setDeviceAddress(uint8_t newAddr);                 // 设置设备I2C地址
static void highPrecisionEnable(eFunctionalState NewState);    // 启用/禁用高精度模式
static void dataInit(void);                                    // 传感器数据初始化
static void readGIr_distance_sensor(void);                     // 读取传感器数据
static void start(void);                                       // 启动传感器测量
static void stop(void);                                        // 停止传感器测量

/**
 * @brief 微秒延时函数
 * @param us 要延时的微秒数
 */
void Distance_DelayUs(uint16_t us)
{
    esp_rom_delay_us(us);
}

// GPIO控制函数 - SCL时钟线控制
/**
 * @brief 将SCL引脚设置为高电平
 */
static void scl_high(void)
{
    gpio_set_level(VL53L0X_SCL_GPIO, 1);
    Distance_DelayUs(15); // 延时15微秒确保信号稳定
}

/**
 * @brief 将SCL引脚设置为低电平
 */
static void scl_low(void)
{
    gpio_set_level(VL53L0X_SCL_GPIO, 0);
    Distance_DelayUs(15); // 延时15微秒确保信号稳定
}

// GPIO控制函数 - SDA数据线控制
/**
 * @brief 将SDA引脚设置为高电平
 */
static void sda_high(void)
{
    gpio_set_level(VL53L0X_SDA_GPIO, 1);
    Distance_DelayUs(15); // 延时15微秒确保信号稳定
}

/**
 * @brief 将SDA引脚设置为低电平
 */
static void sda_low(void)
{
    gpio_set_level(VL53L0X_SDA_GPIO, 0);
    Distance_DelayUs(15); // 延时15微秒确保信号稳定
}

/**
 * @brief 读取SDA引脚状态
 * @return SDA引脚当前电平状态
 */
static uint8_t sda_read(void)
{
    return (uint8_t)gpio_get_level(VL53L0X_SDA_GPIO);
}

/**
 * @brief I2C通信开始信号
 * 按照I2C协议发送起始信号
 */
static void i2c_start(void)
{
    vl53l0_sda_set_output(); // 设置SDA为输出模式
    sda_high();              // SDA拉高
    scl_high();              // SCL拉高
    Distance_DelayUs(5);     // 短暂延时
    sda_low();               // SDA拉低 - 开始信号
    Distance_DelayUs(5);     // 短暂延时
    scl_low();               // SCL拉低 - 为后续传输做准备
}

/**
 * @brief I2C通信停止信号
 * 按照I2C协议发送停止信号
 */
static void i2c_stop(void)
{
    vl53l0_sda_set_output(); // 设置SDA为输出模式
    sda_low();               // SDA拉低
    scl_high();              // SCL拉高
    Distance_DelayUs(5);     // 短暂延时
    sda_high();              // SDA拉高 - 结束信号
    Distance_DelayUs(5);     // 短暂延时
}

/**
 * @brief 毫秒级延时函数
 * @param ms 要延时的毫秒数
 */
static void delay_ms(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

/**
 * @brief 向I2C总线写入一个字节数据
 * @param data 要写入的数据字节
 * @return ACK/NACK响应状态（0=ACK，1=NACK）
 */
static uint8_t i2c_write_byte(uint8_t data)
{
    vl53l0_sda_set_output(); // 设置SDA为输出模式
    for (uint8_t i = 0; i < 8; i++)
    { // 逐位发送8位数据
        if (data & 0x80)
        {               // 检查最高位是否为1
            sda_high(); // 发送1
        }
        else
        {
            sda_low(); // 发送0
        }
        data <<= 1; // 左移一位，准备发送下一位
        scl_high(); // 时钟拉高
        scl_low();  // 时钟拉低，完成一位传输
    }
    vl53l0_sda_set_input(); // 切换为输入模式接收ACK
    scl_high();
    uint8_t ack = sda_read(); // 读取ACK信号（0=ACK，1=NACK）
    scl_low();
    return ack;
}

/**
 * @brief 从I2C总线读取一个字节数据
 * @param ack 应答信号（1=发送ACK，0=发送NACK）
 * @return 读取到的数据字节
 */
static uint8_t i2c_read_byte(uint8_t ack)
{
    vl53l0_sda_set_input(); // 设置SDA为输入模式
    uint8_t data = 0;
    for (uint8_t i = 0; i < 8; i++)
    {               // 逐位读取8位数据
        data <<= 1; // 左移一位，为接收下一位做准备
        scl_high(); // 时钟拉高
        if (sda_read())
        {                 // 读取SDA上的数据位
            data |= 0x01; // 如果为高电平则设置最低位为1
        }
        scl_low(); // 时钟拉低
    }
    vl53l0_sda_set_output(); // 切换为输出模式发送ACK/NACK
    if (ack)
    {
        sda_low(); // 发送ACK信号
    }
    else
    {
        sda_high(); // 发送NACK信号
    }
    scl_high();
    scl_low();
    return data;
}

/**
 * @brief 向连续寄存器写入多个字节数据
 * @param Reg 起始寄存器地址
 * @param buf 数据缓冲区指针
 * @param Num 要写入的字节数
 */
static void writeData(uint8_t Reg, uint8_t *buf, uint8_t Num)
{
    for (uint8_t i = 0; i < Num; i++)
    {
        writeByteData(Reg++, buf[i]); // 依次向递增地址的寄存器写入数据
    }
}

/**
 * @brief 从连续寄存器读取多个字节数据
 * @param Reg 起始寄存器地址
 * @param Num 要读取的字节数
 */
static void readData(uint8_t Reg, uint8_t Num)
{
    i2c_start();                                            // 发送I2C开始信号
    i2c_write_byte(_detailedData.i2cDevAddr << 1);          // 发送设备地址（写操作）
    i2c_write_byte(Reg);                                    // 发送起始寄存器地址
    i2c_start();                                            // 重新发送开始信号
    i2c_write_byte((_detailedData.i2cDevAddr << 1) | 0x01); // 发送设备地址（读操作）

    // 循环读取指定数量的字节数据
    for (int i = 0; i < Num; i++)
    {
        // 最后一个字节发送NACK，其他字节发送ACK
        _detailedData.originalData[i] = i2c_read_byte(i < (Num - 1) ? 1 : 0);
    }
    i2c_stop(); // 发送I2C停止信号
}

/**
 * @brief 向指定寄存器写入单字节数据
 * @param Reg 寄存器地址
 * @param byte 要写入的数据
 */
static void writeByteData(uint8_t Reg, uint8_t byte)
{
    i2c_start();                                   // 发送I2C开始信号
    i2c_write_byte(_detailedData.i2cDevAddr << 1); // 发送设备地址（写操作）
    i2c_write_byte(Reg);                           // 发送寄存器地址
    i2c_write_byte(byte);                          // 发送数据
    i2c_stop();                                    // 发送I2C停止信号
}

/**
 * @brief 从指定寄存器读取单字节数据
 * @param Reg 寄存器地址
 * @return 读取到的数据
 */
static uint8_t readByteData(uint8_t Reg)
{
    uint8_t data;
    i2c_start();                                            // 发送I2C开始信号
    i2c_write_byte(_detailedData.i2cDevAddr << 1);          // 发送设备地址（写操作）
    i2c_write_byte(Reg);                                    // 发送寄存器地址
    i2c_start();                                            // 重新发送开始信号
    i2c_write_byte((_detailedData.i2cDevAddr << 1) | 0x01); // 发送设备地址（读操作）
    data = i2c_read_byte(0);                                // 读取数据，发送NACK结束
    i2c_stop();                                             // 发送I2C停止信号
    return data;
}

/**
 * @brief 设置传感器设备的I2C地址
 * @param newAddr 新的I2C地址
 */
static void setDeviceAddress(uint8_t newAddr)
{
    newAddr &= 0x7F;                                                          // 确保地址为7位
    writeByteData(GIr_distance_sensor_REG_I2C_SLAVE_DEVICE_ADDRESS, newAddr); // 写入新地址到寄存器
    _detailedData.i2cDevAddr = newAddr;                                       // 更新内部地址缓存
}

/**
 * @brief 启用/禁用高精度模式
 * @param NewState eENABLE启用，edisable禁用
 */
static void highPrecisionEnable(eFunctionalState NewState)
{
    writeByteData(GIr_distance_sensor_REG_SYSTEM_RANGE_CONFIG, NewState);
}

/**
 * @brief 传感器数据初始化
 * 执行基本配置和校准
 */
static void dataInit(void)
{
    uint8_t data;
#ifdef ESD_2V8
    // 在ESD保护模式下修改电源配置
    data = readByteData(GIr_distance_sensor_REG_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV);
    data = (data & 0xFE) | 0x01; // 设置第0位为1
    writeByteData(GIr_distance_sensor_REG_VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, data);
#endif
    // 执行传感器初始化序列
    writeByteData(0x88, 0x00); // 写入特定配置值
    writeByteData(0x80, 0x01); // 使能特定功能
    writeByteData(0xFF, 0x01); // 使能访问内部寄存器
    writeByteData(0x00, 0x00); // 写入配置值
    readByteData(0x91);        // 读取默认阈值
    writeByteData(0x91, 0x3c); // 设置新的阈值
    writeByteData(0x00, 0x01); // 恢复访问权限
    writeByteData(0xFF, 0x00); // 禁用内部寄存器访问
    writeByteData(0x80, 0x00); // 禁用特定功能
}

/**
 * @brief 读取传感器原始数据并解析
 * 解析环境光计数、信号计数、距离和状态信息
 */
static void readGIr_distance_sensor(void)
{
    // 从结果范围状态寄存器开始读取12个字节的数据
    readData(GIr_distance_sensor_REG_RESULT_RANGE_STATUS, 12);

    // 解析环境光计数（寄存器偏移6和7）
    _detailedData.ambientCount = ((_detailedData.originalData[6] & 0xFF) << 8) |
                                 (_detailedData.originalData[7] & 0xFF);

    // 解析信号计数（寄存器偏移8和9）
    _detailedData.signalCount = ((_detailedData.originalData[8] & 0xFF) << 8) |
                                (_detailedData.originalData[9] & 0xFF);

    // 解析距离数据（寄存器偏移10和11）
    _detailedData.distance = ((_detailedData.originalData[10] & 0xFF) << 8) |
                             (_detailedData.originalData[11] & 0xFF);

    // 解析状态信息（寄存器偏移0，位3-6）
    _detailedData.status = ((_detailedData.originalData[0] & 0x78) >> 3);
}

/**
 * @brief 启动传感器测量
 * 根据当前模式执行单次或连续测量
 */
static void start(void)
{
    uint8_t DeviceMode;                                                       // 设备工作模式变量，用于存储传感器当前的工作模式（单次测量/连续测量等）
    uint8_t Byte;                                                             // 临时存储变量，用于读取寄存器状态值，判断测量是否完成
    uint8_t StartStopByte = GIr_distance_sensor_REG_SYSRANGE_MODE_START_STOP; // 启停状态掩码，用于检测测量启动/停止状态
    uint32_t LoopNb;                                                          // 循环计数器，用于防止等待测量完成时无限循环，提供超时保护

    DeviceMode = _detailedData.mode; // 获取当前工作模式

    // 执行基本配置序列
    writeByteData(0x80, 0x01); // 写入寄存器0x80，设置MSB（最高有效位）使能标志，激活特定功能模块
    writeByteData(0xFF, 0x01); // 写入寄存器0xFF，使能内部寄存器访问权限，允许访问高级配置寄存器
    writeByteData(0x00, 0x00); // 写入寄存器0x00，清除访问掩码或复位某些内部状态
    writeByteData(0x91, 0x3c); // 写入寄存器0x91，设置新的阈值或配置参数（0x3c=60，可能与信号检测阈值有关）
    writeByteData(0x00, 0x01); // 写入寄存器0x00，恢复访问权限或激活新配置
    writeByteData(0xFF, 0x00); // 写入寄存器0xFF，禁用内部寄存器访问权限，恢复正常操作模式
    writeByteData(0x80, 0x00); // 写入寄存器0x80，禁用MSB使能标志，关闭特定功能模块或返回正常模式

    switch (DeviceMode)
    {
    case eSingle:
        // 单次测量模式
        writeByteData(GIr_distance_sensor_REG_SYSRANGE_START, 0x01);
        Byte = StartStopByte;
        LoopNb = 0;
        // 等待测量完成
        do
        {
            if (LoopNb > 0)
            {
                Byte = readByteData(GIr_distance_sensor_REG_SYSRANGE_START);
            }
            LoopNb++;
        } while (((Byte & StartStopByte) == StartStopByte) &&
                 (LoopNb < GIr_distance_sensor_DEFAULT_MAX_LOOP));
        break;
    case eContinuous:
        // 连续测量模式
        writeByteData(GIr_distance_sensor_REG_SYSRANGE_START, GIr_distance_sensor_REG_SYSRANGE_MODE_BACKTOBACK);
        break;
    default:
        printf("---选择的模式不支持---\r\n"); // 不支持的模式
    }
}

/**
 * @brief 停止传感器测量
 * 停止当前测量过程并重置相关寄存器
 */
static void stop(void)
{
    // 将系统范围启动寄存器设置为单次测量模式，停止当前的连续测量
    // 这个操作告诉传感器停止当前的测量周期并进入空闲状态
    writeByteData(GIr_distance_sensor_REG_SYSRANGE_START, GIr_distance_sensor_REG_SYSRANGE_MODE_SINGLESHOT);

    // 以下是一系列寄存器重置操作，用于恢复传感器到初始配置状态
    writeByteData(0xFF, 0x01); // 重新使能内部寄存器访问权限
    writeByteData(0x00, 0x00); // 清除某些内部配置位或状态标志
    writeByteData(0x91, 0x00); // 重置阈值寄存器，清除之前设置的阈值（0x3c -> 0x00）
    writeByteData(0x00, 0x01); // 恢复某些配置位到默认状态
    writeByteData(0xFF, 0x00); // 禁用内部寄存器访问权限，回到标准操作模式
}

/**
 * @brief 传感器初始化函数
 * 执行完整的传感器初始化流程
 */
void Gir_distance_sensor_init(void)
{
    uint8_t val1;
    delay_ms(2000); // 等待传感器稳定

    // 设置默认I2C地址
    _detailedData.i2cDevAddr = GIr_distance_sensor_DEF_I2C_ADDR;

    // 初始化GPIO引脚
    vl53l0_i2c_init();

    // 拉高SDA和SCL引脚
    sda_high();
    scl_high();

    // 执行数据初始化
    dataInit();
    // 设置设备地址为0x29
    setDeviceAddress(0x29);

    // 读取并打印修订版本ID
    val1 = readByteData(GIr_distance_sensor_REG_IDENTIFICATION_REVISION_ID);
    printf("\r\n修订版本ID: %X\r\n", val1);

    // 读取并打印设备型号ID
    val1 = readByteData(GIr_distance_sensor_REG_IDENTIFICATION_MODEL_ID);
    printf("设备型号ID: %X\r\n\r\n", val1);
}

/**
 * @brief 设置传感器工作模式
 * @param precision 测量精度模式（eHigh高精度，eNormal普通精度）
 */
void Gir_setMode(ePrecisionState precision)
{
    _detailedData.mode = 1; // 设置为单次测量模式
    if (precision == eHigh)
    {
        highPrecisionEnable(eENABLE); // 启用高精度模式
        _detailedData.precision = precision;
    }
    else
    {
        highPrecisionEnable(edisable); // 禁用高精度模式
        _detailedData.precision = precision;
    }
    start(); // 启动传感器以应用新模式
}

/**
 * @brief 获取距离测量值
 * @return 距离值（单位：厘米）
 */
float getDistance(void)
{
    if (_detailedData.mode == eSingle)
    {
        start(); // 在单次模式下每次读取前启动测量
    }

    readGIr_distance_sensor(); // 读取传感器数据

    // 处理异常值：如果读取到20mm，则使用上次有效值
    if (_detailedData.distance == 20)
    {
        _detailedData.distance = _distance;
    }
    else
    {
        _distance = _detailedData.distance;
    }

    float distance_mm;
    if (_detailedData.precision == eHigh)
    {
        distance_mm = _detailedData.distance / 4.0; // 高精度模式下除以4
    }
    else
    {
        distance_mm = _detailedData.distance; // 普通模式直接使用
    }

    // 转换为厘米并打印
    float distance_cm = distance_mm / 10.0;
    // ESP_LOGI("红外测距", "VL53L0 任务数据: 距离=%.2f cm", distance_cm);

    // 返回最终距离值（转换为厘米，保留一位小数，减去3cm校准值）
    return (float)((uint32_t)(distance_cm * 10 + 0.5)) / 10.0 - 3;
}