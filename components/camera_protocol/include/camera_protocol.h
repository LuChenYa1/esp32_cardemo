#ifndef __RS485_RECEIVER_H
#define __RS485_RECEIVER_H

#include "../../uart/include/uart.h"
#include "pin_definitions.h"
#include "esp_log.h"

// 依赖说明：
// - uart.h: UART 端口定义
// - 485servo: uart0_send() 函数（RS485 发送）
// - pin_definitions.h: RS485引脚定义

#define CAMERA_UART_PORT    UART_NUM_0
// RS485方向控制引脚定义在 pin_definitions.h 中
// #define RS485_DIR_GPIO         GPIO_NUM_19
// #define RS485_DIR_TX_LEVEL     1
// #define RS485_DIR_RX_LEVEL     0

/* 环形缓冲区大小（预留用于中断接收模式） */
#define RX_BUF_SIZE         128
#define TX_BUF_SIZE         64

/* 帧头定义 */
#define FRAME_HEADER1       0xFF
#define FRAME_HEADER2       0xFF
#define FRAME_HEADER3       0xA1
#define RS485_FRAME_ID_MIN   0x01
#define RS485_FRAME_ID_MAX   0xFE

/* 帧缓冲区大小 */
#define CAMERA_FRAME_MAX_SIZE       16      // 单帧最大长度
#define CAMERA_DATA_OFFSET          2       // 数据字段相对于帧尾的偏移

/* 超时配置 */
#define CAMERA_RX_TIMEOUT_MS 100  // 从100ms减少到50ms，减少阻塞时间

/* 摄像头日志级别配置（调试完成后改为 ESP_LOG_WARN） */
#define CAMERA_LOG_LEVEL  ESP_LOG_INFO

/* 命令定义 */
#define CMD_READ            0x02    // 读命令
#define CMD_WRITE           0x03    // 写命令

/* 模式定义（MODE字段） */
#define MODE_SPECIFY        0x01    // 指定模式（读操作）
#define MODE_NON_SPECIFY    0x02    // 非指定模式（读操作）
#define MODE_SWITCH         0x01    // 模式切换（写操作）
#define MODE_DEVICE_CTRL    0x02    // 设备控制（写操作）

/* 功能码（根据Excel表格整理，部分列举，其余可自行添加） */
// 识别类型（用于模式切换、读指定/非指定）
#define FUNC_COLOR         	0x01    // 颜色识别
#define FUNC_BlOCK          0x02    // 色块识别
#define FUNC_FACE          	0x03    // 人脸识别
#define FUNC_QRCODE         0x04    // 二维码识别
#define FUNC_NUMBER         0x05    // 数字识别
#define FUNC_LABEL          0x06    // 标签识别
#define FUNC_20CLASS        0x07    // 20类识别
#define FUNC_DEEPLEARN      0x08    // 深度学习
#define FUNC_CAM_CAPTURE    0x09    // 摄像头捕获（模式切换的一种）

// 设备控制项
#define CTRL_REBOOT         0x01    // 重启
#define CTRL_BRIGHTNESS     0x02    // 亮度
#define CTRL_CONTRAST       0x03    // 对比度
#define CTRL_EXPOSURE       0x04    // 曝光
#define CTRL_WB             0x05    // 白平衡
#define CTRL_SATURATION     0x06    // 饱和度
#define CTRL_SHARPEN        0x07    // 锐化
#define CTRL_LANG           0x11    // 系统语言
#define CTRL_OUTPUT_MODE    0x12    // 输出模式
#define CTRL_BORDER         0x13    // 边框显示
#define CTRL_SWITCH3        0x14    // 开关3
#define CTRL_SWITCH4        0x15    // 开关4
#define CTRL_BAUDRATE       0x16    // 串口波特率
#define CTRL_COLOR_AREA     0x21    // 颜色识别区域
#define CTRL_COLOR_VALUE    0x22    // 颜色识别颜色
#define CTRL_BLOCK_NUM      0x23    // 色块识别数量
#define CTRL_BLOCK_AREA     0x24    // 色块识别区域
#define CTRL_LINE_NUM       0x25    // 线条识别数量
#define CTRL_NUMBER_AREA    0x26    // 数字识别区域

// 颜色值
#define COLOR_RED           0x01
#define COLOR_GREEN         0x02
#define COLOR_BLUE          0x03
#define COLOR_YELLOW        0x04
#define COLOR_GRAY          0x05
#define COLOR_PURPLE        0x06
#define COLOR_WHITE         0x07
#define COLOR_BLACK         0x08

// 数字识别参数
#define NUMBER_0            0x10
#define NUMBER_1            0x01
#define NUMBER_2            0x02
#define NUMBER_3            0x03
#define NUMBER_4            0x04
#define NUMBER_5            0x05
#define NUMBER_6            0x06
#define NUMBER_7            0x07
#define NUMBER_8            0x08
#define NUMBER_9            0x09

// 标签识别参数
#define LABEL_1             0x01
#define LABEL_2             0x02
// ... 可继续定义到 LABEL_20 (0x14)

// 20类识别物品参数
#define ITEM_1              0x01
#define ITEM_2              0x02
// ... 可继续定义到 ITEM_20 (0x14)

// 结果值
#define RESULT_SUCCESS      0x01
#define RESULT_FAIL         0x00
#define RESULT_YES          0x01
#define RESULT_NO           0x02

/* 返回状态码 */
#define CAMERA_OK           0    // 成功
#define CAMERA_TIMEOUT      1    // 超时
#define CAMERA_CHECKSUM_ERR 2    // 校验错误
#define CAMERA_RESP_ERR     3    // 响应错误

/* 函数声明 */

/**
 * @brief 初始化摄像头协议
 * @note 会自动调用 servo485_init() 初始化 UART 和 RS485 方向控制引脚
 */
void Camera_Init(void);

/**
 * @brief 设备重启
 * @param id 摄像头设备 ID
 * @return 重启结果（0x01=成功，0x00=失败/超时）
 */
uint8_t Camera_Reset(uint8_t id);

/**
 * @brief 模式切换
 * @param id 摄像头设备 ID
 * @param func 功能码（FUNC_COLOR, FUNC_FACE 等）
 * @param result 输出参数，存储切换结果（0x01=成功，0x00=失败）
 * @return CAMERA_OK 成功，CAMERA_TIMEOUT 超时，CAMERA_CHECKSUM_ERR 校验错误
 */
uint8_t Camera_SetMode(uint8_t id, uint8_t func, uint8_t *result);

/**
 * @brief 设备控制
 * @param id 摄像头设备 ID
 * @param dat1 控制项
 * @param dat2 控制参数
 * @param result 输出参数，存储控制结果（0x01=成功，0x00=失败）
 * @return CAMERA_OK 成功，CAMERA_TIMEOUT 超时，CAMERA_CHECKSUM_ERR 校验错误
 */
uint8_t Camera_DeviceCTRL(uint8_t id, uint8_t dat1, uint8_t dat2, uint8_t *result);

/**
 * @brief 读指定颜色识别
 * @param id 摄像头设备 ID
 * @param color 颜色值（COLOR_RED, COLOR_GREEN 等）
 * @return 识别结果（0x01=检测到，0x00=未检测到/超时）
 */
uint8_t Camera_ReadColorSpec(uint8_t id, uint8_t color);

/**
 * @brief 读非指定颜色识别
 * @param id 摄像头设备 ID
 * @return 检测到的颜色值（COLOR_RED, COLOR_GREEN 等），0x00=未检测到/超时
 */
uint8_t Camera_ReadColorNonSpec(uint8_t id);

/**
 * @brief 读指定人脸识别
 * @param id 摄像头设备 ID
 * @param face_id 人脸 ID
 * @return 识别结果（0x01=检测到，0x00=未检测到/超时）
 */
uint8_t Camera_ReadFaceSpec(uint8_t id, uint8_t face_id);

/**
 * @brief 读非指定人脸识别
 * @param id 摄像头设备 ID
 * @return 检测到的人脸 ID（0x01-0xFE），0x00=未检测到/超时
 */
uint8_t Camera_ReadFaceNonSpec(uint8_t id);

/**
 * @brief 读指定数字识别
 * @param id 摄像头设备 ID
 * @param num_param 数字参数（NUMBER_0 ~ NUMBER_9）
 * @return 识别结果（0x01=检测到，0x00=未检测到/超时）
 */
uint8_t Camera_ReadNumberSpec(uint8_t id, uint8_t num_param);

/**
 * @brief 读非指定数字识别
 * @param id 摄像头设备 ID
 * @return 检测到的数字值（NUMBER_0 ~ NUMBER_9），0x00=未检测到/超时
 */
uint8_t Camera_ReadNumberNonSpec(uint8_t id);

/**
 * @brief 读指定标签识别
 * @param id 摄像头设备 ID
 * @param label 标签值（LABEL_1 ~ LABEL_20）
 * @return 识别结果（0x01=检测到，0x00=未检测到/超时）
 */
uint8_t Camera_ReadLabelSpec(uint8_t id, uint8_t label);

/**
 * @brief 读非指定标签识别
 * @param id 摄像头设备 ID
 * @return 检测到的标签值（LABEL_1 ~ LABEL_20），0x00=未检测到/超时
 */
uint8_t Camera_ReadLabelNonSpec(uint8_t id);

/**
 * @brief 读取二维码数据
 * @param id 摄像头设备 ID
 * @param qr_buf 用于存放二维码数据的缓冲区（预留，当前未使用）
 * @param buf_len 输入时为缓冲区最大长度，输出时为实际接收到的数据长度（预留，当前未使用）
 * @return 检测结果（0x01=检测到，0x00=未检测到/超时）
 * @note TODO: 实现二维码数据解析，当前仅返回检测结果
 */
uint8_t Camera_ReadQR_Code(uint8_t id, uint8_t *qr_buf, uint8_t *buf_len);

#endif
