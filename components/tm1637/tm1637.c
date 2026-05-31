#include "tm1637.h"
#include "pin_definitions.h"
#include "driver/gpio.h"
#include "us_delay.h"
#include "esp_rom_sys.h"
#include "esp_log.h"

static const char *TAG = "TM1637";

/**
 * 数字显示编码表 - 存储数字0-9以及带小数点的数字在7段数码管上的显示编码
 * 每个编码对应7段数码管的a,b,c,d,e,f,g段
 */
const uint8_t num_code[] = 
{
    //0    ,1   ,2   ,3   ,4   ,5   ,6   ,7   ,8   ,9   ,A   ,b   ,C   ,d   ,E   ,F  	
	0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
	//0.   ,1.  ,2.  ,3.  ,4.  ,5.  ,6.  ,7.  ,8.  ,9.  ,Null
	0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x00
};
 
/**
 * 数码管地址表 - 存储4个数码管的地址命令
 * 用于指定数据写入哪个数码管
 */
const uint8_t u8TubeAddrTab[] = 
{
	0xC0,0xC1,0xC2,0xC3  // 分别对应第1-4个数码管的地址
};

/**
 * @brief 初始化TM1637数码管
 * 
 * 配置GPIO引脚并初始化TM1637显示
 */
void tm1637_init(void)
{
    // 配置SCL和SDA引脚为输出模式
    gpio_set_direction(TM1637_CLK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(TM1637_DIO_GPIO, GPIO_MODE_OUTPUT);
    
    // 初始化SDA线为高电平
    gpio_set_level(TM1637_DIO_GPIO, 1);
    udelay(10);
    
    // 初始化SCL线为高电平
    gpio_set_level(TM1637_CLK_GPIO, 1);
    udelay(10);
    
    // 发送停止信号，确保TM1637处于空闲状态
    tm1637_stop();
    
    // 开启TM1637显示
    tm1637_switch(true);
    
    // 设置TM1637为固定地址模式
    tm1637_wt_cmd(0x44);
    
    ESP_LOGI(TAG, "TM1637初始化完成: CLK=GPIO%d, DIO=GPIO%d", 
             TM1637_CLK_GPIO, TM1637_DIO_GPIO);
}
 
/**
 * 设置TM1637的数据线(DIO)状态
 * @param state: DIO线的状态(true为高电平，false为低电平)
 */
void tm1637_wt_sda(bool state)
{
    gpio_set_level(TM1637_DIO_GPIO, state);  // 设置DIO引脚电平
    esp_rom_delay_us(10);                    // 延时10微秒，保证信号稳定
}
 
/**
 * 设置TM1637的时钟线(CLK)状态
 * @param state: CLK线的状态(true为高电平，false为低电平)
 */
void tm1637_wt_scl(bool state)
{
    gpio_set_level(TM1637_CLK_GPIO, state);  // 设置CLK引脚电平
    esp_rom_delay_us(10);                    // 延时10微秒，保证信号稳定
}
 
/**
 * 读取TM1637的数据线(DIO)状态
 * @return: DIO线当前的电平状态
 */
uint8_t tm1637_rd_sda()
{
    uint8_t bitvalue;
    
    // 将DIO设置为输入模式以读取数据
    gpio_reset_pin(TM1637_DIO_GPIO);
    gpio_set_direction(TM1637_DIO_GPIO, GPIO_MODE_INPUT);
    
    // 读取DIO引脚电平
    bitvalue = gpio_get_level(TM1637_DIO_GPIO);
    
    udelay(5);  // 短暂延时
    
    // 恢复DIO为输出模式
    gpio_set_direction(TM1637_DIO_GPIO, GPIO_MODE_OUTPUT);
    
    return bitvalue;
}
 
/**
 * TM1637起始信号 - 发送I2C通信的起始条件
 * SDA从高变低，SCL保持低电平
 */
void tm1637_start()
{
    tm1637_wt_sda(true);    // SDA拉高
    tm1637_wt_scl(true);    // SCL拉高
    tm1637_wt_sda(false);   // SDA拉低 - 起始信号
    tm1637_wt_scl(false);   // SCL拉低 - 为后续数据传输做准备
}
 
/**
 * TM1637停止信号 - 发送I2C通信的停止条件
 * SDA从低变高，SCL保持高电平
 */
void tm1637_stop()
{
    tm1637_wt_scl(false);   // SCL拉低
    tm1637_wt_sda(false);   // SDA拉低
    tm1637_wt_scl(true);    // SCL拉高
    tm1637_wt_sda(true);    // SDA拉高 - 停止信号
}
 
/**
 * TM1637读取应答信号 - 接收从设备的应答
 * @return: 应答状态（当前实现总是返回0）
 */
uint8_t tm1637_rd_ack()
{
    tm1637_wt_scl(0);       // SCL拉低
    tm1637_wt_scl(1);       // SCL拉高
    tm1637_wt_scl(0);       // SCL再次拉低
    return 0;               // 当前实现未实际读取应答，直接返回0
}
 
/**
 * 向TM1637发送一个字节数据
 * @param byte: 要发送的8位数据
 */
void tm1637_wt_byte(uint8_t byte)
{
    uint8_t i;

    // 循环发送8位数据，从最低位开始
    for (i = 0; i < 8; i++)
    {
        tm1637_wt_scl(0);           // 时钟拉低，准备发送数据
        
        if(byte & 0x1)              // 判断最低位是否为1
            tm1637_wt_sda(1);       // 如果是1，SDA拉高
        else
            tm1637_wt_sda(0);       // 如果是0，SDA拉低
 
        byte >>= 1;                 // 数据右移一位，准备发送下一位
 
        tm1637_wt_scl(1);           // 时钟拉高，完成一位数据传输
    }
}
 
/**
 * 向TM1637发送控制命令
 * @param cmd: 要发送的命令字节
 */
void tm1637_wt_cmd(uint8_t cmd)
{
    tm1637_start();         // 发送起始信号
    tm1637_wt_byte(cmd);    // 发送命令字节
    tm1637_rd_ack();        // 读取应答信号
    tm1637_stop();          // 发送停止信号
}
 
/**
 * 向TM1637的指定数码管写入显示数据
 * @param grid: 数码管位置地址
 * @param data: 要显示的编码数据
 */
void tm1637_wt_data(uint8_t grid, uint8_t data)
{
    tm1637_start();             // 发送起始信号
    tm1637_wt_byte(grid);       // 发送数码管地址
    tm1637_rd_ack();            // 读取应答信号
    tm1637_wt_byte(data);       // 发送显示数据
    tm1637_rd_ack();            // 读取应答信号
    tm1637_stop();              // 发送停止信号
}
 
/**
 * 控制TM1637显示开关
 * @param bstate: 显示状态(true开启显示，false关闭显示)
 */
void tm1637_switch(bool bstate)
{
    // 根据状态发送对应的显示控制命令
    bstate ? tm1637_wt_cmd(DISP_ON) : tm1637_wt_cmd(DISP_OFF);
}
 
/**
 * 在4位数码管上显示指定数据
 * @param td: 包含4个数码管显示内容的数组指针
 */
void tm1637_tubedisplay(uint8_t * td)
{
    int i=0;
    for(i=0; i<4; i++)      // 遍历4个数码管
    {
        // 向每个数码管发送对应的显示数据
        tm1637_wt_data(u8TubeAddrTab[i], num_code[td[i]]);
    }
}
 
/**
 * 将16位数值转换为4位数码管显示格式
 * @param u16Data: 要显示的数值(最大9999)
 */
void tm1637_disp_num_process(uint16_t u16Data)
{
    uint8_t tube_data[4] = {0};     // 存储4个数码管的显示数据

    // 限制数值范围，最大显示9999
	if (u16Data > 9999)
	{
		u16Data = 9999; // 最多四位数
	}
	
	if (u16Data > 999) // 四位数处理
	{
		tube_data[0] = (uint8_t)(u16Data / 1000);      // 千位
		tube_data[1] = (uint8_t)(u16Data / 100 % 10);  // 百位
		tube_data[2] = (uint8_t)(u16Data % 100 / 10);  // 十位
	    tube_data[3] = (uint8_t)(u16Data % 10);        // 个位
	}
	else if (u16Data > 99) // 三位数处理
	{
		tube_data[0] = TUBE_DISPLAY_NULL;              // 不显示第一位
		tube_data[1] = (uint8_t)(u16Data / 100);       // 百位
		tube_data[2] = (uint8_t)(u16Data / 10 % 10);   // 十位
	    tube_data[3] = (uint8_t)(u16Data % 10);        // 个位	   
	}
	else if (u16Data > 9) // 两位数处理
	{
		tube_data[0] = TUBE_DISPLAY_NULL;              // 不显示第一位
	    tube_data[1] = TUBE_DISPLAY_NULL;              // 不显示第二位
		tube_data[2] = (uint8_t)(u16Data / 10);        // 十位
	    tube_data[3] = (uint8_t)(u16Data % 10);        // 个位
	}
	else // 一位数处理
	{
		tube_data[0] = TUBE_DISPLAY_NULL;              // 不显示第一位
	    tube_data[1] = TUBE_DISPLAY_NULL;              // 不显示第二位
		tube_data[2] = TUBE_DISPLAY_NULL;              // 不显示第三位
	    tube_data[3] = (uint8_t)u16Data;               // 个位
	}
 
    tm1637_wt_cmd(WRT_DATA);        // 发送写数据命令
	tm1637_tubedisplay(tube_data);  // 更新数码管显示
}