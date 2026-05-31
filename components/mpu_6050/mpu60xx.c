/**
 * @file mpu60xx.c
 * @brief MPU6050六轴传感器驱动实现
 * 
 * 功能：
 * - 三轴加速度计
 * - 三轴陀螺仪
 * - 温度传感器
 * - I2C通信接口
 * 
 * 注意：
 * - 需要先初始化I2C总线
 * - 传感器地址默认为0x68
 */

#include "mpu60xx.h"        // 包含MPU6050传感器的头文件，定义了寄存器地址和函数原型
#include"freertos/FreeRTOS.h"  // 包含FreeRTOS核心库，用于任务管理和延时函数

// 定义日志标签，用于ESP-IDF日志系统输出
static const char *TAG = "MPU";

// 静态布尔变量，用于跟踪I2C是否已初始化，避免重复初始化
static bool i2c_initialized = false;

/**
 * @brief MPU6050专用毫秒延时函数
 * @param cms 要延时的毫秒数
 * 
 * 使用FreeRTOS的vTaskDelay函数实现精确延时，
 * 这比使用硬件定时器更节能，适合在任务中使用
 */
void mpu60xx_delay_ms(uint32_t cms)
{
    // 将毫秒数转换为FreeRTOS的tick数
    TickType_t xDelay = cms / portTICK_PERIOD_MS;
    
    // 调用FreeRTOS延时函数，让出CPU给其他任务
    vTaskDelay( xDelay );
}
/**
 * @brief MPU6050初始化函数，配置相关寄存器
 * @param 无参数
 * @return ESP_OK表示成功，其他值表示失败
 * 
 * 执行完整的MPU6050初始化流程，包括复位、唤醒、配置各传感器量程等
 */
esp_err_t mpu60xx_init(void)
{
    // 声明错误码变量，初始化为成功状态
    esp_err_t err = ESP_OK;
    
    // 检查I2C是否已经初始化，如果没有则进行初始化
    if (!i2c_initialized) {
        // 调用I2C初始化函数
        err = mpu_i2c_init();
        // 如果I2C初始化失败，直接返回错误
        if(err != ESP_OK){       
            return err;      
        }
        // 标记I2C已初始化
        i2c_initialized = true;
    }
    
    // 向电源管理寄存器1写入0x80，触发MPU6050复位
    mpu_write_byte(MPU_PWR_MGMT1_REG,0x80);
    ESP_LOGI(TAG, "MPU6050 resetting ok!");  // 记录复位成功日志
    
    // 延时100毫秒，等待复位完成
    mpu60xx_delay_ms(100);
    
    // 向电源管理寄存器1写入0x00，唤醒MPU6050
    mpu_write_byte(MPU_PWR_MGMT1_REG,0X00);
    ESP_LOGI(TAG, "MPU6050 wake up ok!");  // 记录唤醒成功日志
    
    // 设置陀螺仪满量程范围为±2000 dps (degrees per second)
    mpu60xx_set_gyro_fsr(3);    
    ESP_LOGI(TAG, "MPU6050 set gyro fsr ok!");  // 记录陀螺仪量程设置成功日志
    
    // 设置加速度计满量程范围为±2g
    mpu60xx_set_accel_fsr(0);   
    ESP_LOGI(TAG, "MPU6050 set accel fsr ok!");  // 记录加速度计量程设置成功日志
    
    // 设置采样率为50Hz
    mpu60xx_set_rate(50);						
    ESP_LOGI(TAG, "MPU6050 set rate ok!");  // 记录采样率设置成功日志
    
    // 关闭所有中断功能
    mpu_write_byte(MPU_INT_EN_REG,0X00);
    ESP_LOGI(TAG, "MPU6050 disable all int ok!");  // 记录中断关闭成功日志
    
    // 关闭I2C主模式
    mpu_write_byte(MPU_USER_CTRL_REG,0X00);
    ESP_LOGI(TAG, "MPU6050 disable i2c master ok!");  // 记录I2C主模式关闭成功日志
    
    // 关闭FIFO功能
    mpu_write_byte(MPU_FIFO_EN_REG,0X00);
    ESP_LOGI(TAG, "MPU6050 disable fifo ok!");  // 记录FIFO关闭成功日志
    
    // 设置INT引脚为低电平有效
    mpu_write_byte(MPU_INTBP_CFG_REG,0X80);
    ESP_LOGI(TAG, "MPU6050 set int pin ok!");  // 记录INT引脚设置成功日志
    
    // 读取设备ID寄存器，验证是否为正确的MPU6050芯片
    uint8_t res=mpu_read_byte(MPU_DEVICE_ID_REG);
    ESP_LOGI(TAG, "Device ID read: 0x%02x", res);  // 输出读取到的设备ID
    
    // 检查设备ID是否匹配（MPU6050的ID为0x68，有些版本可能是0x98）
    if(res==0x68 || res==0x98)
    {
        // 设置时钟源为PLL X轴晶振
        mpu_write_byte(MPU_PWR_MGMT1_REG,0X01);
        // 使能加速度计和陀螺仪
        mpu_write_byte(MPU_PWR_MGMT2_REG,0X00);
        // 再次设置采样率为50Hz
        mpu60xx_set_rate(50);
        ESP_LOGI(TAG, "mpu init ok ");  // 记录初始化成功日志
    }
    else 
        // 如果设备ID不匹配，返回失败
        return ESP_FAIL;
    
    // 返回初始化结果
    return err;
}

#pragma region MPU60xx

/**
 * @brief 设置MPU6050陀螺仪传感器满量程范围
 * @param fsr 满量程范围选择：0=±250dps, 1=±500dps, 2=±1000dps, 3=±2000dps
 * @return ESP_OK表示设置成功，其他值表示设置失败
 * 
 * 通过设置陀螺仪配置寄存器的高3位来选择不同的满量程范围
 */
esp_err_t mpu60xx_set_gyro_fsr(uint8_t fsr)
{
    // 将fsr左移3位，写入陀螺仪配置寄存器的FS_SEL位段
    return mpu_write_byte(MPU_GYRO_CFG_REG,fsr<<3);
}
/**
 * @brief 设置MPU6050加速度传感器满量程范围
 * @param fsr 满量程范围选择：0=±2g, 1=±4g, 2=±8g, 3=±16g
 * @return ESP_OK表示设置成功，其他值表示设置失败
 * 
 * 通过设置加速度配置寄存器的高3位来选择不同的满量程范围
 */
esp_err_t mpu60xx_set_accel_fsr(uint8_t fsr)
{
    // 将fsr左移3位，写入加速度配置寄存器的FS_SEL位段
    return mpu_write_byte(MPU_ACCEL_CFG_REG,fsr<<3);
}
/**
 * @brief 设置MPU6050的数字低通滤波器
 * @param lpf 数字低通滤波频率，单位Hz
 * @return ESP_OK表示设置成功，其他值表示设置失败
 * 
 * 根据输入的截止频率设置相应的滤波器参数，用于减少高频噪声
 */
esp_err_t mpu60xx_set_lfp(uint16_t lpf)  
{ 
    // 声明数据变量
    uint8_t data=0;
    
    // 根据输入的滤波频率选择相应的配置值
    if(lpf>=188)data=1;    // 188Hz以上选择配置值1
    else if(lpf>=98)data=2; // 98Hz-188Hz选择配置值2
    else if(lpf>=42)data=3; // 42Hz-98Hz选择配置值3
    else if(lpf>=20)data=4; // 20Hz-42Hz选择配置值4
    else if(lpf>=10)data=5; // 10Hz-20Hz选择配置值5
    else data=6;            // 10Hz以下选择配置值6
    
    // 将配置值写入配置寄存器
    return mpu_write_byte(MPU_CFG_REG,data);
}

/**
 * @brief 设置MPU6050的采样率(假定Fs=1KHz)
 * @param rate 采样率范围：4~1000 Hz
 * @return ESP_OK表示设置成功，其他值表示设置失败
 * 
 * 通过计算SMPLRT_DIV寄存器值来设置采样率，并自动设置相应的低通滤波器
 */
esp_err_t  mpu60xx_set_rate(uint16_t rate)
{
    // 声明数据变量
    uint8_t data;
    
    // 限制采样率范围在4-1000Hz之间
    if(rate>1000)rate=1000;
    if(rate<4)rate=4;
    
    // 计算采样率分频值：SMPLRT_DIV = 1000/rate - 1
    data=1000/rate-1;
    // 将分频值写入采样率分频寄存器
    data=mpu_write_byte(MPU_SAMPLE_RATE_REG,data);
    
    // 自动设置低通滤波器为采样率的一半，以避免混叠
    return mpu60xx_set_lfp(rate/2);
}
/**
 * @brief 获取MPU6050内部温度值
 * @param 无参数
 * @return 温度值(扩大了100倍，单位0.01℃)
 * 
 * 读取MPU6050内置温度传感器的原始数据并转换为摄氏度
 */
float mpu60xx_get_temperature(void)
{
    // 声明存储原始数据的缓冲区
    uint8_t buf[2]; 
    // 声明原始温度值变量
    short raw;
    // 声明转换后的温度值变量
    float temp;
    
    // 从温度高位和低位寄存器读取2字节原始数据
    mpu_read_buf(MPU_TEMP_OUTH_REG,2,buf); 
    
    // 将两个字节合并为一个16位有符号整数
    raw=((uint16_t)buf[0]<<8)|buf[1];  
    
    // 将原始值转换为摄氏度：温度 = 36.53 + raw/340
    temp=36.53+((double)raw)/340;  
    
    // 返回转换后的温度值
    return temp;
}
/**
 * @brief 获取MPU6050陀螺仪三轴原始数据
 * @param gx 指向陀螺仪x轴原始数据的指针(带符号)
 * @param gy 指向陀螺仪y轴原始数据的指针(带符号)
 * @param gz 指向陀螺仪z轴原始数据的指针(带符号)
 * @return ESP_OK表示成功，其他值表示失败
 * 
 * 从陀螺仪三个轴的输出寄存器读取原始数据
 */
esp_err_t mpu60xx_get_gyroscope(short *gx,short *gy,short *gz)
{
    // 声明缓冲区，用于存储6字节的陀螺仪数据
    uint8_t buf[6];
    // 声明返回结果变量
    esp_err_t res;  
    
    // 从陀螺仪X轴高位寄存器开始读取6字节连续数据
    res=mpu_read_buf(MPU_GYRO_XOUTH_REG,6,buf);
    
    // 如果读取成功，解析数据到对应的变量
    if(res==0)
    {
        // 从buf[0]和buf[1]合成X轴陀螺仪数据
        *gx=((uint16_t)buf[0]<<8)|buf[1];
        // 从buf[2]和buf[3]合成Y轴陀螺仪数据
        *gy=((uint16_t)buf[2]<<8)|buf[3];  
        // 从buf[4]和buf[5]合成Z轴陀螺仪数据
        *gz=((uint16_t)buf[4]<<8)|buf[5];
    } 	
    // 返回读取结果
    return res;
}
/**
 * @brief 获取MPU6050加速度计三轴原始数据
 * @param ax 指向加速度计x轴原始数据的指针(带符号)
 * @param ay 指向加速度计y轴原始数据的指针(带符号)
 * @param az 指向加速度计z轴原始数据的指针(带符号)
 * @return ESP_OK表示成功，其他值表示失败
 * 
 * 从加速度计三个轴的输出寄存器读取原始数据
 */
esp_err_t mpu60xx_get_accelerometer(short *ax,short *ay,short *az)
{
    // 声明缓冲区，用于存储6字节的加速度计数据
    uint8_t buf[6];  
    
    // 从加速度计X轴高位寄存器开始读取6字节连续数据
    esp_err_t res=mpu_read_buf(MPU_ACCEL_XOUTH_REG,6,buf);
    
    // 如果读取成功，解析数据到对应的变量
    if(res==0)
    {
        // 从buf[0]和buf[1]合成X轴加速度数据
        *ax=((uint16_t)buf[0]<<8)|buf[1];  
        // 从buf[2]和buf[3]合成Y轴加速度数据
        *ay=((uint16_t)buf[2]<<8)|buf[3];  
        // 从buf[4]和buf[5]合成Z轴加速度数据
        *az=((uint16_t)buf[4]<<8)|buf[5];
    } 	
    // 返回读取结果
    return res;;
}

/**
 * @brief MPU6050测试函数，读取并打印传感器数据
 * 
 * 一次性读取加速度计、陀螺仪和温度数据，并通过日志输出
 */
void mpu60xx_test()
{
    // 声明加速度传感器原始数据变量
    short aacx,aacy,aacz;		// 加速度传感器原始数据
	
    // 声明陀螺仪原始数据变量
    short gyrox,gyroy,gyroz;	// 陀螺仪原始数据
	
    // 声明温度变量
    float temp;					// 温度值
	
    // 先尝试读取加速度计数据，如果成功则继续读取其他数据
    if(mpu60xx_get_accelerometer(&aacx,&aacy,&aacz)==ESP_OK){
        // 获取温度值
        temp=mpu60xx_get_temperature();	
        
        // 获取陀螺仪数据
        mpu60xx_get_gyroscope(&gyrox,&gyroy,&gyroz);	
        
        // 通过日志输出所有传感器数据
        ESP_LOGI(TAG, "MPU 任务数据: 温度=%.02fC, 加速度(ax=%d, ay=%d, az=%d), 陀螺仪(gx=%d, gy=%d, gz=%d)",
                 temp, aacx, aacy, aacz, gyrox, gyroy, gyroz);
    }
}

#pragma endregion
