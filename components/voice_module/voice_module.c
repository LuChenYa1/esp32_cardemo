/**
 * @file voice_module.c
 * @brief 语音控制模块实现
 * 
 * 实现语音命令接收、解析和处理功能
 * 通信协议：0xAA 0x55 [CMD] 0x55 0xAA（5字节）
 * 使用UART1，波特率115200
 * 
 * 功能：
 * - 接收并解析语音命令
 * - 控制摄像头识别模式（颜色、色块、人脸、二维码、数字、标签、20类）
 * - 控制摄像头亮度
 * - 控制运行模式（巡线/命令模式）
 * - 控制速度设置
 */

#include "voice_module.h"
#include "camera_protocol.h"
#include "pd_controller.h"
#include "turn_statemachine.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <string.h>
#include <stdatomic.h>

/* ==================== 私有变量 ==================== */

static const char *TAG = "voice_module";

// 全局标志变量定义（使用原子变量保证线程安全）
_Atomic uint8_t Flag_Color = 0;
_Atomic uint8_t Flag_Block = 0;
_Atomic uint8_t Flag_Face = 0;
_Atomic uint8_t Flag_QRCODE = 0;
_Atomic uint8_t Flag_NUMBER = 0;
_Atomic uint8_t Flag_LABEL = 0;
_Atomic uint8_t Flag_20CLASS = 0;
_Atomic uint8_t Flag_Speed = 2;
_Atomic uint8_t Flag_RunMode = 0;  // 0=命令模式（电机停止），1=巡线模式（电机运行）- 默认巡线模式

// 任务句柄
static TaskHandle_t voice_task_handle = NULL;

// 亮度控制计数器
static int bri_cnt = 2;

// 摄像头设备ID（默认为3）
#define CAMERA_DEVICE_ID 3

/* ==================== 私有函数声明 ==================== */

/**
 * @brief 解析命令帧
 * 
 * @param data 接收到的数据缓冲区
 * @param len 数据长度
 * @param frame 输出的命令帧结构
 * @return true 解析成功，false 解析失败
 */
static bool parse_command_frame(const uint8_t *data, size_t len, VoiceCommandFrame_t *frame);

/**
 * @brief 处理语音命令
 * 
 * @param command 命令字节
 * @return ESP_OK 成功，ESP_FAIL 失败
 */
static esp_err_t process_voice_command(uint8_t command);

/* ==================== 公共函数实现 ==================== */

/**
 * @brief 初始化语音模块（使用UART1）
 */
esp_err_t voice_module_init(void)
{
    ESP_LOGI(TAG, "初始化语音模块（UART1，波特率115200）...");
    
    // 配置UART参数
    const uart_config_t uart_config = {
        .baud_rate = VOICE_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    // 配置UART参数
    esp_err_t err = uart_param_config(VOICE_UART_PORT, &uart_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "UART参数配置失败: %s", esp_err_to_name(err));
        return err;
    }
    
    // 设置UART引脚
    err = uart_set_pin(VOICE_UART_PORT, 
                       UART1_TX_GPIO, 
                       UART1_RX_GPIO, 
                       UART_PIN_NO_CHANGE, 
                       UART_PIN_NO_CHANGE);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "UART引脚设置失败: %s", esp_err_to_name(err));
        return err;
    }
    
    // 安装UART驱动（只需要接收缓冲区）
    err = uart_driver_install(VOICE_UART_PORT, 
                              VOICE_UART_RX_BUFFER_SIZE, 
                              0,  // 不需要发送缓冲区
                              0, 
                              NULL, 
                              0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "UART驱动安装失败: %s", esp_err_to_name(err));
        return err;
    }
    
    // 初始化标志变量（使用原子操作）
    atomic_store(&Flag_Color, 0);
    atomic_store(&Flag_Block, 0);
    atomic_store(&Flag_Face, 0);
    atomic_store(&Flag_QRCODE, 0);
    atomic_store(&Flag_NUMBER, 0);
    atomic_store(&Flag_LABEL, 0);
    atomic_store(&Flag_20CLASS, 0);
    atomic_store(&Flag_Speed, 2);
    atomic_store(&Flag_RunMode, 0);  // 默认命令模式，等待语音命令启动
    
    ESP_LOGI(TAG, "语音模块初始化成功");
    return ESP_OK;
}

/**
 * @brief 创建语音模块任务
 */
esp_err_t voice_module_task_create(void)
{
    // 将语音模块任务固定到 Core 1，避免影响 Core 0 的实时控制
    BaseType_t ret = xTaskCreatePinnedToCore(
        voice_module_task,
        "voice_task",
        VOICE_TASK_STACK_SIZE,
        NULL,
        VOICE_TASK_PRIORITY,
        &voice_task_handle,
        1  // 固定到 Core 1
    );
    
    if (ret != pdPASS) {
        ESP_LOGE(TAG, "创建语音模块任务失败");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "语音模块任务创建成功（Core 1）");
    return ESP_OK;
}

/**
 * @brief 删除语音模块任务
 */
void voice_module_task_delete(void)
{
    if (voice_task_handle != NULL) {
        vTaskDelete(voice_task_handle);
        voice_task_handle = NULL;
        ESP_LOGI(TAG, "语音模块任务已删除");
    }
}

/**
 * @brief 获取运行模式标志（线程安全）
 */
uint8_t voice_module_get_run_mode(void)
{
    return atomic_load(&Flag_RunMode);
}

/**
 * @brief 设置运行模式标志（线程安全）
 */
void voice_module_set_run_mode(uint8_t mode)
{
    atomic_store(&Flag_RunMode, mode);
    ESP_LOGI(TAG, "运行模式设置为: %d", mode);
}

/**
 * @brief 语音模块任务入口函数
 * 
 * 注意：
 * - 摄像头操作（Camera_SetMode等）通过RS485通信，可能阻塞较长时间
 * - 为避免影响Timer中断和系统实时性，命令处理中移除了延时和读取操作
 * - 只执行模式切换，不等待结果，减少任务执行时间
 */
void voice_module_task(void *pvParameters)
{
    uint8_t rx_buffer[VOICE_UART_RX_BUFFER_SIZE];
    
    ESP_LOGI(TAG, "语音模块任务启动");
    
    while (1) {
        // 非阻塞式读取UART数据（超时时间100ms）
        int len = uart_read_bytes(VOICE_UART_PORT, 
                                  rx_buffer, 
                                  sizeof(rx_buffer), 
                                  pdMS_TO_TICKS(100));
        
        if (len > 0) {
            // 记录接收到的数据（调试用）
            ESP_LOGD(TAG, "接收到 %d 字节数据", len);
            
            // 记录开始时间（用于验证处理时间不超过10ms）
            int64_t start_time = esp_timer_get_time();
            
            // 解析命令帧
            VoiceCommandFrame_t frame;
            if (parse_command_frame(rx_buffer, len, &frame)) {
                // 处理命令
                process_voice_command(frame.command);
            }
            
            // 计算处理时间
            int64_t elapsed_us = esp_timer_get_time() - start_time;
            if (elapsed_us > 10000) {  // 10ms = 10000us
                ESP_LOGW(TAG, "命令处理时间超过10ms: %lld us", elapsed_us);
            }
        }
        
        // 短暂延时，避免占用过多CPU
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

/* ==================== 私有函数实现 ==================== */

/**
 * @brief 解析命令帧
 * 
 * 帧格式：[0xAA] [0x55] [CMD] [0x55] [0xAA]
 */
static bool parse_command_frame(const uint8_t *data, size_t len, VoiceCommandFrame_t *frame)
{
    if (data == NULL || frame == NULL || len < VOICE_CMD_FRAME_LEN) {
        return false;
    }
    
    // 查找帧头 0xAA 0x55
    for (size_t i = 0; i < len - VOICE_CMD_FRAME_LEN + 1; i++) {
        if (data[i] == VOICE_CMD_FRAME_HEADER1 && data[i + 1] == VOICE_CMD_FRAME_HEADER2) {
            // 检查是否有足够的数据
            if (i + VOICE_CMD_FRAME_LEN <= len) {
                // 检查帧尾 0x55 0xAA
                if (data[i + 3] == VOICE_CMD_FRAME_TAIL1 && data[i + 4] == VOICE_CMD_FRAME_TAIL2) {
                    // 解析帧数据
                    frame->header1 = data[i];
                    frame->header2 = data[i + 1];
                    frame->command = data[i + 2];
                    frame->tail1 = data[i + 3];
                    frame->tail2 = data[i + 4];
                    
                    ESP_LOGD(TAG, "解析到命令帧: cmd=0x%02X", frame->command);
                    return true;
                }
            }
        }
    }
    
    ESP_LOGD(TAG, "未找到有效的命令帧");
    return false;
}

/**
 * @brief 处理语音命令
 */
static esp_err_t process_voice_command(uint8_t command)
{
    uint8_t result = 0;
    
    switch (command) {
        case VOICE_CMD_COLOR:  // 0x01 颜色识别模式
            ESP_LOGI(TAG, "收到颜色识别命令");
            // 清零所有标志
            atomic_store(&Flag_Color, 0);
            atomic_store(&Flag_Block, 0);
            atomic_store(&Flag_Face, 0);
            atomic_store(&Flag_QRCODE, 0);
            atomic_store(&Flag_NUMBER, 0);
            atomic_store(&Flag_LABEL, 0);
            atomic_store(&Flag_20CLASS, 0);
            // 切换摄像头模式
            if (Camera_SetMode(CAMERA_DEVICE_ID, FUNC_COLOR, &result) == CAMERA_OK) {
                ESP_LOGI(TAG, "摄像头切换到颜色识别模式成功");
                // 延时等待摄像头稳定
                vTaskDelay(pdMS_TO_TICKS(100));
                // 读取一次识别结果
                uint8_t color = Camera_ReadColorNonSpec(CAMERA_DEVICE_ID);
                if (color != 0) {
                    ESP_LOGI(TAG, "检测到颜色: 0x%02X", color);
                } else {
                    ESP_LOGI(TAG, "未检测到颜色");
                }
            } else {
                ESP_LOGW(TAG, "摄像头切换到颜色识别模式失败");
            }
            break;
            
        case VOICE_CMD_BLOCK:  // 0x02 色块识别模式
            ESP_LOGI(TAG, "收到色块识别命令");
            // 清零所有标志
            atomic_store(&Flag_Color, 0);
            atomic_store(&Flag_Block, 0);
            atomic_store(&Flag_Face, 0);
            atomic_store(&Flag_QRCODE, 0);
            atomic_store(&Flag_NUMBER, 0);
            atomic_store(&Flag_LABEL, 0);
            atomic_store(&Flag_20CLASS, 0);
            // 切换摄像头模式
            if (Camera_SetMode(CAMERA_DEVICE_ID, FUNC_BlOCK, &result) == CAMERA_OK) {
                ESP_LOGI(TAG, "摄像头切换到色块识别模式成功");
                // TODO: 实现色块识别读取
                ESP_LOGI(TAG, "色块识别功能待实现");
            } else {
                ESP_LOGW(TAG, "摄像头切换到色块识别模式失败");
            }
            break;
            
        case VOICE_CMD_FACE:  // 0x03 人脸识别模式
            ESP_LOGI(TAG, "收到人脸识别命令");
            // 清零所有标志
            atomic_store(&Flag_Color, 0);
            atomic_store(&Flag_Block, 0);
            atomic_store(&Flag_Face, 0);
            atomic_store(&Flag_QRCODE, 0);
            atomic_store(&Flag_NUMBER, 0);
            atomic_store(&Flag_LABEL, 0);
            atomic_store(&Flag_20CLASS, 0);
            // 切换摄像头模式（不读取结果，避免阻塞）
            if (Camera_SetMode(CAMERA_DEVICE_ID, FUNC_FACE, &result) == CAMERA_OK) {
                ESP_LOGI(TAG, "摄像头切换到人脸识别模式成功");
            } else {
                ESP_LOGW(TAG, "摄像头切换到人脸识别模式失败");
            }
            break;
            
        case VOICE_CMD_QRCODE:  // 0x04 二维码识别模式
            ESP_LOGI(TAG, "收到二维码识别命令");
            // 清零所有标志
            atomic_store(&Flag_Color, 0);
            atomic_store(&Flag_Block, 0);
            atomic_store(&Flag_Face, 0);
            atomic_store(&Flag_QRCODE, 0);
            atomic_store(&Flag_NUMBER, 0);
            atomic_store(&Flag_LABEL, 0);
            atomic_store(&Flag_20CLASS, 0);
            // 切换摄像头模式（不读取结果，避免阻塞）
            if (Camera_SetMode(CAMERA_DEVICE_ID, FUNC_QRCODE, &result) == CAMERA_OK) {
                ESP_LOGI(TAG, "摄像头切换到二维码识别模式成功");
            } else {
                ESP_LOGW(TAG, "摄像头切换到二维码识别模式失败");
            }
            break;
            
        case VOICE_CMD_NUMBER:  // 0x05 数字识别模式
            ESP_LOGI(TAG, "收到数字识别命令");
            // 清零所有标志
            atomic_store(&Flag_Color, 0);
            atomic_store(&Flag_Block, 0);
            atomic_store(&Flag_Face, 0);
            atomic_store(&Flag_QRCODE, 0);
            atomic_store(&Flag_NUMBER, 0);
            atomic_store(&Flag_LABEL, 0);
            atomic_store(&Flag_20CLASS, 0);
            // 切换摄像头模式（不读取结果，避免阻塞）
            if (Camera_SetMode(CAMERA_DEVICE_ID, FUNC_NUMBER, &result) == CAMERA_OK) {
                ESP_LOGI(TAG, "摄像头切换到数字识别模式成功");
            } else {
                ESP_LOGW(TAG, "摄像头切换到数字识别模式失败");
            }
            break;
            
        case VOICE_CMD_LABEL:  // 0x06 标签识别模式
            ESP_LOGI(TAG, "收到标签识别命令");
            // 清零所有标志
            atomic_store(&Flag_Color, 0);
            atomic_store(&Flag_Block, 0);
            atomic_store(&Flag_Face, 0);
            atomic_store(&Flag_QRCODE, 0);
            atomic_store(&Flag_NUMBER, 0);
            atomic_store(&Flag_LABEL, 0);
            atomic_store(&Flag_20CLASS, 0);
            // 切换摄像头模式（不读取结果，避免阻塞）
            if (Camera_SetMode(CAMERA_DEVICE_ID, FUNC_LABEL, &result) == CAMERA_OK) {
                ESP_LOGI(TAG, "摄像头切换到标签识别模式成功");
            } else {
                ESP_LOGW(TAG, "摄像头切换到标签识别模式失败");
            }
            break;
            
        case VOICE_CMD_20CLASS:  // 0x07 20类物体识别模式
            ESP_LOGI(TAG, "收到20类识别命令");
            // 清零所有标志
            atomic_store(&Flag_Color, 0);
            atomic_store(&Flag_Block, 0);
            atomic_store(&Flag_Face, 0);
            atomic_store(&Flag_QRCODE, 0);
            atomic_store(&Flag_NUMBER, 0);
            atomic_store(&Flag_LABEL, 0);
            atomic_store(&Flag_20CLASS, 0);
            // 切换摄像头模式
            if (Camera_SetMode(CAMERA_DEVICE_ID, FUNC_20CLASS, &result) == CAMERA_OK) {
                ESP_LOGI(TAG, "摄像头切换到20类识别模式成功");
                // TODO: 实现20类识别读取
                ESP_LOGI(TAG, "20类识别功能待实现");
            } else {
                ESP_LOGW(TAG, "摄像头切换到20类识别模式失败");
            }
            break;
            
        case VOICE_CMD_CAPTURE:  // 0x09 拍照模式
            ESP_LOGI(TAG, "收到拍照命令");
            atomic_store(&Flag_Color, 0);
            atomic_store(&Flag_Block, 0);
            atomic_store(&Flag_Face, 0);
            atomic_store(&Flag_QRCODE, 0);
            atomic_store(&Flag_NUMBER, 0);
            atomic_store(&Flag_LABEL, 0);
            atomic_store(&Flag_20CLASS, 0);
            // 调用摄像头设置函数
            if (Camera_SetMode(CAMERA_DEVICE_ID, FUNC_CAM_CAPTURE, &result) == CAMERA_OK) {
                ESP_LOGI(TAG, "摄像头切换到拍照模式成功");
            } else {
                ESP_LOGW(TAG, "摄像头切换到拍照模式失败");
            }
            break;
            
        case VOICE_CMD_RESET:  // 0x0B 摄像头复位
            ESP_LOGI(TAG, "收到摄像头复位命令");
            // 调用摄像头复位函数
            result = Camera_Reset(CAMERA_DEVICE_ID);
            if (result == RESULT_SUCCESS) {
                ESP_LOGI(TAG, "摄像头复位成功");
            } else {
                ESP_LOGW(TAG, "摄像头复位失败");
            }
            atomic_store(&Flag_Color, 0);
            atomic_store(&Flag_Block, 0);
            atomic_store(&Flag_Face, 0);
            atomic_store(&Flag_QRCODE, 0);
            atomic_store(&Flag_NUMBER, 0);
            atomic_store(&Flag_LABEL, 0);
            atomic_store(&Flag_20CLASS, 0);
            break;
            
        case VOICE_CMD_BRIGHTNESS_UP:  // 0x0C 增加亮度
            ESP_LOGI(TAG, "收到增加亮度命令");
            if (bri_cnt < 4) bri_cnt++;
            // 调用摄像头亮度控制函数
            if (Camera_DeviceCTRL(CAMERA_DEVICE_ID, CTRL_BRIGHTNESS, bri_cnt, &result) == CAMERA_OK) {
                ESP_LOGI(TAG, "摄像头亮度调整成功，当前亮度=%d", bri_cnt);
            } else {
                ESP_LOGW(TAG, "摄像头亮度调整失败");
            }
            break;
            
        case VOICE_CMD_BRIGHTNESS_DOWN:  // 0x0D 降低亮度
            ESP_LOGI(TAG, "收到降低亮度命令");
            if (bri_cnt > 0) bri_cnt--;
            // 调用摄像头亮度控制函数
            if (Camera_DeviceCTRL(CAMERA_DEVICE_ID, CTRL_BRIGHTNESS, bri_cnt, &result) == CAMERA_OK) {
                ESP_LOGI(TAG, "摄像头亮度调整成功，当前亮度=%d", bri_cnt);
            } else {
                ESP_LOGW(TAG, "摄像头亮度调整失败");
            }
            break;
            
        case VOICE_CMD_SPEED_HIGH:  // 0x0E 速度设置为2（高速）
            ESP_LOGI(TAG, "收到高速命令");
            atomic_store(&Flag_Speed, 2);
            // 设置PD控制器速度参数
            pd_controller_set_params(999, 8.0f, 12.0f);
            // 设置转弯后退时间（高速：18 ticks = 180ms）
            turn_statemachine_set_back_ticks(24);
            ESP_LOGI(TAG, "速度已设置为高速（999），后退时间18 ticks");
            break;
            
        case VOICE_CMD_SPEED_LOW:  // 0x0F 速度设置为1（低速）
            ESP_LOGI(TAG, "收到低速命令");
            atomic_store(&Flag_Speed, 1);
            // 设置PD控制器速度参数
            pd_controller_set_params(700, 5.0f, 12.0f);
            // 设置转弯后退时间（低速：12 ticks = 120ms）
            turn_statemachine_set_back_ticks(13);
            ESP_LOGI(TAG, "速度已设置为低速（700），后退时间12 ticks");
            break;
            
        case VOICE_CMD_RUN_MODE:  // 0x11 巡线模式 - 启动电机
            ESP_LOGI(TAG, "收到巡线模式命令（启动电机）");
            atomic_store(&Flag_RunMode, 1);
            break;
            
        case VOICE_CMD_CMD_MODE:  // 0x12 命令模式 - 停止电机
            ESP_LOGI(TAG, "收到命令模式命令（停止电机）");
            atomic_store(&Flag_RunMode, 0);
            break;
            
        default:
            ESP_LOGW(TAG, "未知命令: 0x%02X", command);
            return ESP_FAIL;
    }
    
    return ESP_OK;
}
