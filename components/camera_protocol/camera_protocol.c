/**
 * @file camera_protocol.c
 * @brief 摄像头通信协议实现
 * 
 * 功能：
 * - 通过RS485与摄像头通信
 * - 发送控制指令
 * - 接收识别结果
 * 
 * 通信协议：
 * - 帧头：0xAA 0x55
 * - 数据格式：指令 + 参数 + 校验和
 * 
 * 注意：
 * - 使用UART0的RS485互斥锁
 * - 与485舵机共用RS485总线
 */

#include "camera_protocol.h"
#include "pin_definitions.h"
#include "esp_log.h"
#include "485servo.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "CAMERA";

/* 环形缓冲区（预留用于中断接收模式） */
static uint8_t rxBuf[RX_BUF_SIZE];
static volatile uint16_t rxHead = 0, rxTail = 0;

/* 解析状态机变量 */
static enum {
    WAIT_FF1,
    WAIT_FF2,
    WAIT_A1,
    WAIT_VER,
    WAIT_LEN,
    WAIT_PAYLOAD
} state = WAIT_FF1;

static uint8_t rxFrame[32];    // 当前正在组装的帧
static uint8_t rxFrameIdx = 0; // 当前帧的索引
static uint8_t payloadCnt = 0; // 长度字段之后已接收的字节数

/* ==================== 预留函数（用于中断接收模式，当前未使用） ==================== */

#if 0  // 预留用于中断接收模式，当前使用轮询模式
static uint8_t IsValidFrameId(uint8_t id)
{
    return (id >= RS485_FRAME_ID_MIN) && (id <= RS485_FRAME_ID_MAX);
}
#endif

/**
 * @brief 计算校验和（前置声明的实现）
 * @param pData 数据指针
 * @param len 数据长度
 * @return 校验和（取反）
 */
static uint8_t CalcChecksum(uint8_t *pData, uint8_t len)
{
    uint16_t sum = 0;
    for (uint8_t i = 0; i < len; i++)
    {
        sum += pData[i];
    }
    return (uint8_t)(~sum);
}

#if 0  // 预留用于中断接收模式，当前使用轮询模式
static void StartFrameWithFF(void)
{
    state = WAIT_FF2;
    rxFrame[0] = FRAME_HEADER1;
    rxFrameIdx = 1;
    payloadCnt = 0;
}

static void ResetFrameParser(void)
{
    state = WAIT_FF1;
    rxFrameIdx = 0;
    payloadCnt = 0;
}

/* 从环形缓冲区取一个字节，返回1成功，0失败（预留用于中断接收模式） */
static uint8_t GetByteFromRxBuf(uint8_t *pData)
{
    if (rxTail == rxHead)
        return 0;
    *pData = rxBuf[rxTail];
    rxTail = (rxTail + 1) % RX_BUF_SIZE;
    return 1;
}
#endif  // 预留用于中断接收模式

/**
 * @brief 使用事件队列等待接收指定 ID 的帧（中断方式，避免 CPU 忙等待）
 * @param expectedId 期望的设备 ID
 * @param buffer 接收缓冲区
 * @param length 接收到的帧长度
 * @param timeoutMs 超时时间（毫秒）
 * @return 1=成功接收，0=超时
 */
static uint8_t RS485_WaitFrameById(uint8_t expectedId, uint8_t *buffer, uint16_t *length, uint32_t timeoutMs)
{
    QueueHandle_t uart_queue = uart0_get_event_queue();
    if (uart_queue == NULL) {
        ESP_LOGE(TAG, "UART0 事件队列未初始化");
        return 0;
    }
    
    uart_event_t event;
    uint8_t *temp_buf = (uint8_t *)malloc(RX_BUF_SIZE);
    if (temp_buf == NULL) {
        ESP_LOGE(TAG, "内存分配失败");
        return 0;
    }
    
    uint32_t startTick = pdTICKS_TO_MS(xTaskGetTickCount());
    uint8_t found = 0;
    
    while ((pdTICKS_TO_MS(xTaskGetTickCount()) - startTick) < timeoutMs && !found)
    {
        uint32_t remainingTime = timeoutMs - (pdTICKS_TO_MS(xTaskGetTickCount()) - startTick);
        if (remainingTime == 0) break;
        
        // 关键：阻塞等待 UART 事件，没有事件时任务完全阻塞，CPU 占用 0%
        if (xQueueReceive(uart_queue, &event, pdMS_TO_TICKS(remainingTime)))
        {
            switch (event.type)
            {
            case UART_DATA:
                // 有数据到达，读取数据
                {
                    int len = uart_read_bytes(CAMERA_UART_PORT, temp_buf, 
                                             event.size, pdMS_TO_TICKS(10));
                    
                    if (len > 0) {
                        // 逐字节解析
                        for (int i = 0; i < len && !found; i++) {
                            uint8_t byte = temp_buf[i];
                            
                            // 状态机解析
                            switch (state)
                            {
                            case WAIT_FF1:
                                if (byte == FRAME_HEADER1) {
                                    state = WAIT_FF2;
                                    rxFrame[0] = byte;
                                    rxFrameIdx = 1;
                                }
                                break;
                                
                            case WAIT_FF2:
                                if (byte == FRAME_HEADER2) {
                                    rxFrame[rxFrameIdx++] = byte;
                                    state = WAIT_A1;
                                } else if (byte == FRAME_HEADER1) {
                                    rxFrameIdx = 1;
                                } else {
                                    state = WAIT_FF1;
                                }
                                break;
                                
                            case WAIT_A1:
                                if (byte == FRAME_HEADER3) {
                                    rxFrame[rxFrameIdx++] = byte;
                                    state = WAIT_VER;
                                } else {
                                    state = WAIT_FF1;
                                }
                                break;
                                
                            case WAIT_VER:
                                if (byte >= RS485_FRAME_ID_MIN && byte <= RS485_FRAME_ID_MAX) {
                                    rxFrame[rxFrameIdx++] = byte;
                                    state = WAIT_LEN;
                                } else {
                                    state = WAIT_FF1;
                                }
                                break;
                                
                            case WAIT_LEN:
                                rxFrame[rxFrameIdx++] = byte;
                                if (byte >= 3 && byte <= 5) {
                                    payloadCnt = 0;
                                    state = WAIT_PAYLOAD;
                                } else {
                                    state = WAIT_FF1;
                                }
                                break;
                                
                            case WAIT_PAYLOAD:
                                rxFrame[rxFrameIdx++] = byte;
                                if (++payloadCnt >= 3) {
                                    // 校验和检查
                                    if (CalcChecksum(&rxFrame[2], rxFrameIdx - 3) == rxFrame[rxFrameIdx - 1]) {
                                        // ID 匹配检查
                                        if (rxFrame[3] == expectedId) {
                                            memcpy(buffer, rxFrame, rxFrameIdx);
                                            *length = rxFrameIdx;
                                            state = WAIT_FF1;
                                            found = 1;
                                        }
                                    }
                                    state = WAIT_FF1;
                                }
                                break;
                            }
                        }
                    }
                }
                break;
                
            case UART_FIFO_OVF:
                ESP_LOGW(TAG, "UART FIFO 溢出");
                uart_flush_input(CAMERA_UART_PORT);
                xQueueReset(uart_queue);
                break;
                
            case UART_BUFFER_FULL:
                ESP_LOGW(TAG, "UART 缓冲区满");
                uart_flush_input(CAMERA_UART_PORT);
                xQueueReset(uart_queue);
                break;
                
            case UART_BREAK:
            case UART_PARITY_ERR:
            case UART_FRAME_ERR:
                ESP_LOGW(TAG, "UART 错误事件: %d", event.type);
                break;
                
            default:
                break;
            }
        }
    }
    
    free(temp_buf);
    return found;
}

/**
 * @brief 发送帧数据
 * @param id 设备 ID
 * @param cmd 命令码
 * @param mode 模式
 * @param data 数据指针
 * @param len 数据长度
 */
static void SendFrame(uint8_t id, uint8_t cmd, uint8_t mode, uint8_t *data, uint8_t len)
{
    uint8_t txBuf[64];
    uint8_t idx = 0;

    txBuf[idx++] = 0xFF; // FRAME_HEADER1
    txBuf[idx++] = 0xFF; // FRAME_HEADER2
    txBuf[idx++] = 0xA1; // FRAME_HEADER3
    txBuf[idx++] = id;
    txBuf[idx++] = cmd;
    txBuf[idx++] = mode;
    txBuf[idx++] = len;
    for (uint8_t i = 0; i < len; i++)
        txBuf[idx++] = data[i];

    uint8_t checksum = CalcChecksum(&txBuf[2], idx - 2);
    txBuf[idx++] = checksum;

    // 发送前清空接收缓冲区，避免读取到旧数据
    uart_flush_input(CAMERA_UART_PORT);
    
    // 设置485方向为发送模式
    gpio_set_level(RS485_DIR_GPIO, RS485_DIR_TX_LEVEL);

    // 通过UART发送完整的舵机控制指令（11字节）
    uart0_send(txBuf, idx);

    // 发送完成后短暂延时确保数据发送完毕
    vTaskDelay(pdMS_TO_TICKS(10)); // 增加延时以确保数据发送完整
    // 设置485方向为接收模式
    gpio_set_level(RS485_DIR_GPIO, RS485_DIR_RX_LEVEL);
}



// 设备重启（写操作）
uint8_t Camera_Reset(uint8_t id)
{
    // 获取RS485互斥锁
    SemaphoreHandle_t mutex = uart0_get_rs485_mutex();
    if (mutex == NULL) {
        ESP_LOGE(TAG, "RS485互斥锁未初始化");
        return 0;
    }
    
    // 尝试获取互斥锁，最多等待200ms
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        ESP_LOGW(TAG, "获取RS485互斥锁超时，Camera_Reset id=0x%02X", id);
        return 0;
    }
    
    uint8_t rx_frame[16];
    uint16_t rx_len;

    uint8_t tx_data = CTRL_REBOOT;
    SendFrame(id, CMD_WRITE, MODE_DEVICE_CTRL, &tx_data, 1);
    
    uint8_t result = 0;
    if (RS485_WaitFrameById(id, rx_frame, &rx_len, CAMERA_RX_TIMEOUT_MS))
    {
        result = rx_frame[rx_len - 2];
    }
    
    // 释放互斥锁
    xSemaphoreGive(mutex);
    return result;
}

/**
 * @brief 模式切换
 */
uint8_t Camera_SetMode(uint8_t id, uint8_t func, uint8_t *result)
{
    // 获取RS485互斥锁
    SemaphoreHandle_t mutex = uart0_get_rs485_mutex();
    if (mutex == NULL) {
        ESP_LOGE(TAG, "RS485互斥锁未初始化");
        return CAMERA_TIMEOUT;
    }
    
    // 尝试获取互斥锁，最多等待200ms
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        ESP_LOGW(TAG, "获取RS485互斥锁超时，Camera_SetMode id=0x%02X", id);
        return CAMERA_TIMEOUT;
    }
    
    uint8_t rx_frame[CAMERA_FRAME_MAX_SIZE];
    uint16_t rx_len;
    uint8_t tx_data = func;

    SendFrame(id, CMD_WRITE, MODE_SWITCH, &tx_data, 1);
    
    uint8_t ret = CAMERA_TIMEOUT;
    if (RS485_WaitFrameById(id, rx_frame, &rx_len, CAMERA_RX_TIMEOUT_MS))
    {
        // 校验和检查
        if (CalcChecksum(&rx_frame[2], rx_len - 3) == rx_frame[rx_len - 1])
        {
            *result = rx_frame[rx_len - CAMERA_DATA_OFFSET];
            ESP_LOGI(TAG, "SetMode result: 0x%02X", *result);
            ret = CAMERA_OK;
        }
        else
        {
            ESP_LOGW(TAG, "SetMode checksum error");
            ret = CAMERA_CHECKSUM_ERR;
        }
    }
    else
    {
        ESP_LOGW(TAG, "SetMode timeout, id=0x%02X, func=0x%02X", id, func);
    }
    
    // 释放互斥锁
    xSemaphoreGive(mutex);
    return ret;
}

/**
 * @brief 设备控制
 */
uint8_t Camera_DeviceCTRL(uint8_t id, uint8_t dat1, uint8_t dat2, uint8_t *result)
{
    // 获取RS485互斥锁
    SemaphoreHandle_t mutex = uart0_get_rs485_mutex();
    if (mutex == NULL) {
        ESP_LOGE(TAG, "RS485互斥锁未初始化");
        return CAMERA_TIMEOUT;
    }
    
    // 尝试获取互斥锁，最多等待200ms
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        ESP_LOGW(TAG, "获取RS485互斥锁超时，Camera_DeviceCTRL id=0x%02X", id);
        return CAMERA_TIMEOUT;
    }
    
    uint8_t rx_frame[CAMERA_FRAME_MAX_SIZE];
    uint16_t rx_len;
    uint8_t tx_data[2] = {dat1, dat2};

    SendFrame(id, CMD_WRITE, MODE_DEVICE_CTRL, tx_data, 2);
    
    uint8_t ret = CAMERA_TIMEOUT;
    if (RS485_WaitFrameById(id, rx_frame, &rx_len, CAMERA_RX_TIMEOUT_MS))
    {
        // 校验和检查
        if (CalcChecksum(&rx_frame[2], rx_len - 3) == rx_frame[rx_len - 1])
        {
            *result = rx_frame[rx_len - CAMERA_DATA_OFFSET];
            ESP_LOGI(TAG, "DeviceCTRL result: 0x%02X", *result);
            ret = CAMERA_OK;
        }
        else
        {
            ESP_LOGW(TAG, "DeviceCTRL checksum error");
            ret = CAMERA_CHECKSUM_ERR;
        }
    }
    else
    {
        ESP_LOGW(TAG, "DeviceCTRL timeout, id=0x%02X", id);
    }
    
    // 释放互斥锁
    xSemaphoreGive(mutex);
    return ret;
}
/**
 * @brief 读指定颜色识别
 */
uint8_t Camera_ReadColorSpec(uint8_t id, uint8_t color)
{
    // 获取RS485互斥锁
    SemaphoreHandle_t mutex = uart0_get_rs485_mutex();
    if (mutex == NULL) {
        ESP_LOGE(TAG, "RS485互斥锁未初始化");
        return 0;
    }
    
    // 尝试获取互斥锁，最多等待200ms
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        ESP_LOGW(TAG, "获取RS485互斥锁超时，Camera_ReadColorSpec id=0x%02X", id);
        return 0;
    }
    
    uint8_t rx_frame[CAMERA_FRAME_MAX_SIZE];
    uint16_t rx_len;
    uint8_t tx_data[2] = {FUNC_COLOR, color};
    
    SendFrame(id, CMD_READ, MODE_SPECIFY, tx_data, 2);
    
    uint8_t result = 0;
    if (RS485_WaitFrameById(id, rx_frame, &rx_len, CAMERA_RX_TIMEOUT_MS))
    {
        // 校验和检查
        if (CalcChecksum(&rx_frame[2], rx_len - 3) == rx_frame[rx_len - 1])
        {
            result = rx_frame[rx_len - CAMERA_DATA_OFFSET];
            ESP_LOGI(TAG, "ReadColorSpec: color=0x%02X, result=0x%02X", color, result);
        }
        else
        {
            ESP_LOGW(TAG, "ReadColorSpec checksum error");
        }
    }
    else
    {
        ESP_LOGW(TAG, "ReadColorSpec timeout, id=0x%02X, color=0x%02X", id, color);
    }
    
    // 释放互斥锁
    xSemaphoreGive(mutex);
    return result;
}

/**
 * @brief 读非指定颜色识别
 */
uint8_t Camera_ReadColorNonSpec(uint8_t id)
{
    // 获取RS485互斥锁
    SemaphoreHandle_t mutex = uart0_get_rs485_mutex();
    if (mutex == NULL) {
        ESP_LOGE(TAG, "RS485互斥锁未初始化");
        return 0;
    }
    
    // 尝试获取互斥锁，最多等待200ms
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        ESP_LOGW(TAG, "获取RS485互斥锁超时，Camera_ReadColorNonSpec id=0x%02X", id);
        return 0;
    }
    
    uint8_t rx_frame[CAMERA_FRAME_MAX_SIZE];
    uint16_t rx_len;
    uint8_t tx_data = FUNC_COLOR;

    SendFrame(id, CMD_READ, MODE_NON_SPECIFY, &tx_data, 1);
    
    uint8_t detected_color = 0;
    if (RS485_WaitFrameById(id, rx_frame, &rx_len, CAMERA_RX_TIMEOUT_MS))
    {
        // 校验和检查
        if (CalcChecksum(&rx_frame[2], rx_len - 3) == rx_frame[rx_len - 1])
        {
            detected_color = rx_frame[rx_len - CAMERA_DATA_OFFSET];
            
            // 打印检测到的颜色
            const char *color_name = "UNKNOWN";
            switch (detected_color)
            {
            case COLOR_RED:    color_name = "RED";    break;
            case COLOR_GREEN:  color_name = "GREEN";  break;
            case COLOR_BLUE:   color_name = "BLUE";   break;
            case COLOR_YELLOW: color_name = "YELLOW"; break;
            case COLOR_GRAY:   color_name = "GRAY";   break;
            case COLOR_PURPLE: color_name = "PURPLE"; break;
            case COLOR_WHITE:  color_name = "WHITE";  break;
            case COLOR_BLACK:  color_name = "BLACK";  break;
            }
            ESP_LOGI(TAG, "ReadColorNonSpec: detected=%s (0x%02X)", color_name, detected_color);
        }
        else
        {
            ESP_LOGW(TAG, "ReadColorNonSpec checksum error");
        }
    }
    else
    {
        ESP_LOGW(TAG, "ReadColorNonSpec timeout, id=0x%02X", id);
    }
    
    // 释放互斥锁
    xSemaphoreGive(mutex);
    return detected_color;
}

/**
 * @brief 读指定人脸识别
 */
uint8_t Camera_ReadFaceSpec(uint8_t id, uint8_t face_id)
{
    // 获取RS485互斥锁
    SemaphoreHandle_t mutex = uart0_get_rs485_mutex();
    if (mutex == NULL) {
        ESP_LOGE(TAG, "RS485互斥锁未初始化");
        return 0;
    }
    
    // 尝试获取互斥锁，最多等待200ms
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        ESP_LOGW(TAG, "获取RS485互斥锁超时，Camera_ReadFaceSpec id=0x%02X", id);
        return 0;
    }
    
    uint8_t rx_frame[CAMERA_FRAME_MAX_SIZE];
    uint16_t rx_len;
    uint8_t tx_data[2] = {FUNC_FACE, face_id};

    SendFrame(id, CMD_READ, MODE_SPECIFY, tx_data, 2);
    
    uint8_t result = 0;
    if (RS485_WaitFrameById(id, rx_frame, &rx_len, CAMERA_RX_TIMEOUT_MS))
    {
        // 校验和检查
        if (CalcChecksum(&rx_frame[2], rx_len - 3) == rx_frame[rx_len - 1])
        {
            result = rx_frame[rx_len - CAMERA_DATA_OFFSET];
            ESP_LOGI(TAG, "ReadFaceSpec: face_id=0x%02X, result=0x%02X", face_id, result);
        }
        else
        {
            ESP_LOGW(TAG, "ReadFaceSpec checksum error");
        }
    }
    else
    {
        ESP_LOGW(TAG, "ReadFaceSpec timeout, id=0x%02X, face_id=0x%02X", id, face_id);
    }
    
    // 释放互斥锁
    xSemaphoreGive(mutex);
    return result;
}

/**
 * @brief 读非指定人脸识别
 */
uint8_t Camera_ReadFaceNonSpec(uint8_t id)
{
    // 获取RS485互斥锁
    SemaphoreHandle_t mutex = uart0_get_rs485_mutex();
    if (mutex == NULL) {
        ESP_LOGE(TAG, "RS485互斥锁未初始化");
        return 0;
    }
    
    // 尝试获取互斥锁，最多等待200ms
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        ESP_LOGW(TAG, "获取RS485互斥锁超时，Camera_ReadFaceNonSpec id=0x%02X", id);
        return 0;
    }
    
    uint8_t rx_frame[CAMERA_FRAME_MAX_SIZE];
    uint16_t rx_len;
    uint8_t tx_data = FUNC_FACE;

    SendFrame(id, CMD_READ, MODE_NON_SPECIFY, &tx_data, 1);
    
    uint8_t detected_id = 0;
    if (RS485_WaitFrameById(id, rx_frame, &rx_len, CAMERA_RX_TIMEOUT_MS))
    {
        // 校验和检查
        if (CalcChecksum(&rx_frame[2], rx_len - 3) == rx_frame[rx_len - 1])
        {
            detected_id = rx_frame[rx_len - CAMERA_DATA_OFFSET];
            ESP_LOGI(TAG, "ReadFaceNonSpec: detected_id=0x%02X", detected_id);
        }
        else
        {
            ESP_LOGW(TAG, "ReadFaceNonSpec checksum error");
        }
    }
    else
    {
        ESP_LOGW(TAG, "ReadFaceNonSpec timeout, id=0x%02X", id);
    }
    
    // 释放互斥锁
    xSemaphoreGive(mutex);
    return detected_id;
}

/**
 * @brief 读指定数字识别
 */
uint8_t Camera_ReadNumberSpec(uint8_t id, uint8_t num_param)
{
    // 获取RS485互斥锁
    SemaphoreHandle_t mutex = uart0_get_rs485_mutex();
    if (mutex == NULL) {
        ESP_LOGE(TAG, "RS485互斥锁未初始化");
        return 0;
    }
    
    // 尝试获取互斥锁，最多等待200ms
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        ESP_LOGW(TAG, "获取RS485互斥锁超时，Camera_ReadNumberSpec id=0x%02X", id);
        return 0;
    }
    
    uint8_t rx_frame[CAMERA_FRAME_MAX_SIZE];
    uint16_t rx_len;
    uint8_t tx_data[2] = {FUNC_NUMBER, num_param};

    SendFrame(id, CMD_READ, MODE_SPECIFY, tx_data, 2);
    
    uint8_t result = 0;
    if (RS485_WaitFrameById(id, rx_frame, &rx_len, CAMERA_RX_TIMEOUT_MS))
    {
        // 校验和检查
        if (CalcChecksum(&rx_frame[2], rx_len - 3) == rx_frame[rx_len - 1])
        {
            result = rx_frame[rx_len - CAMERA_DATA_OFFSET];
            ESP_LOGI(TAG, "ReadNumberSpec: num_param=0x%02X, result=0x%02X", num_param, result);
        }
        else
        {
            ESP_LOGW(TAG, "ReadNumberSpec checksum error");
        }
    }
    else
    {
        ESP_LOGW(TAG, "ReadNumberSpec timeout, id=0x%02X, num_param=0x%02X", id, num_param);
    }
    
    // 释放互斥锁
    xSemaphoreGive(mutex);
    return result;
}

/**
 * @brief 读非指定数字识别
 */
uint8_t Camera_ReadNumberNonSpec(uint8_t id)
{
    // 获取RS485互斥锁
    SemaphoreHandle_t mutex = uart0_get_rs485_mutex();
    if (mutex == NULL) {
        ESP_LOGE(TAG, "RS485互斥锁未初始化");
        return 0;
    }
    
    // 尝试获取互斥锁，最多等待200ms
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        ESP_LOGW(TAG, "获取RS485互斥锁超时，Camera_ReadNumberNonSpec id=0x%02X", id);
        return 0;
    }
    
    uint8_t rx_frame[CAMERA_FRAME_MAX_SIZE];
    uint16_t rx_len;
    uint8_t tx_data = FUNC_NUMBER;

    SendFrame(id, CMD_READ, MODE_NON_SPECIFY, &tx_data, 1);
    
    uint8_t number = 0;
    if (RS485_WaitFrameById(id, rx_frame, &rx_len, CAMERA_RX_TIMEOUT_MS))
    {
        // 校验和检查
        if (CalcChecksum(&rx_frame[2], rx_len - 3) == rx_frame[rx_len - 1])
        {
            number = rx_frame[rx_len - CAMERA_DATA_OFFSET];
            
            // 打印检测到的数字
            const char *num_name = "UNKNOWN";
            switch (number)
            {
            case NUMBER_0: num_name = "NUMBER_0"; break;
            case NUMBER_1: num_name = "NUMBER_1"; break;
            case NUMBER_2: num_name = "NUMBER_2"; break;
            case NUMBER_3: num_name = "NUMBER_3"; break;
            case NUMBER_4: num_name = "NUMBER_4"; break;
            case NUMBER_5: num_name = "NUMBER_5"; break;
            case NUMBER_6: num_name = "NUMBER_6"; break;
            case NUMBER_7: num_name = "NUMBER_7"; break;
            case NUMBER_8: num_name = "NUMBER_8"; break;
            case NUMBER_9: num_name = "NUMBER_9"; break;
            }
            ESP_LOGI(TAG, "ReadNumberNonSpec: detected=%s (0x%02X)", num_name, number);
        }
        else
        {
            ESP_LOGW(TAG, "ReadNumberNonSpec checksum error");
        }
    }
    else
    {
        ESP_LOGW(TAG, "ReadNumberNonSpec timeout, id=0x%02X", id);
    }
    
    // 释放互斥锁
    xSemaphoreGive(mutex);
    return number;
}

/**
 * @brief 读指定标签识别
 */
uint8_t Camera_ReadLabelSpec(uint8_t id, uint8_t label)
{
    // 获取RS485互斥锁
    SemaphoreHandle_t mutex = uart0_get_rs485_mutex();
    if (mutex == NULL) {
        ESP_LOGE(TAG, "RS485互斥锁未初始化");
        return 0;
    }
    
    // 尝试获取互斥锁，最多等待200ms
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        ESP_LOGW(TAG, "获取RS485互斥锁超时，Camera_ReadLabelSpec id=0x%02X", id);
        return 0;
    }
    
    uint8_t rx_frame[CAMERA_FRAME_MAX_SIZE];
    uint16_t rx_len;
    uint8_t tx_data[2] = {FUNC_LABEL, label};

    SendFrame(id, CMD_READ, MODE_SPECIFY, tx_data, 2);
    
    uint8_t result = 0;
    if (RS485_WaitFrameById(id, rx_frame, &rx_len, CAMERA_RX_TIMEOUT_MS))
    {
        // 校验和检查
        if (CalcChecksum(&rx_frame[2], rx_len - 3) == rx_frame[rx_len - 1])
        {
            result = rx_frame[rx_len - CAMERA_DATA_OFFSET];
            ESP_LOGI(TAG, "ReadLabelSpec: label=0x%02X, result=0x%02X", label, result);
        }
        else
        {
            ESP_LOGW(TAG, "ReadLabelSpec checksum error");
        }
    }
    else
    {
        ESP_LOGW(TAG, "ReadLabelSpec timeout, id=0x%02X, label=0x%02X", id, label);
    }
    
    // 释放互斥锁
    xSemaphoreGive(mutex);
    return result;
}

/**
 * @brief 读非指定标签识别
 */
uint8_t Camera_ReadLabelNonSpec(uint8_t id)
{
    // 获取RS485互斥锁
    SemaphoreHandle_t mutex = uart0_get_rs485_mutex();
    if (mutex == NULL) {
        ESP_LOGE(TAG, "RS485互斥锁未初始化");
        return 0;
    }
    
    // 尝试获取互斥锁，最多等待200ms
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        ESP_LOGW(TAG, "获取RS485互斥锁超时，Camera_ReadLabelNonSpec id=0x%02X", id);
        return 0;
    }
    
    uint8_t tx_data = FUNC_LABEL;
    uint8_t rx_frame[CAMERA_FRAME_MAX_SIZE];
    uint16_t rx_len;

    SendFrame(id, CMD_READ, MODE_NON_SPECIFY, &tx_data, 1);
    
    uint8_t label = 0;
    if (RS485_WaitFrameById(id, rx_frame, &rx_len, CAMERA_RX_TIMEOUT_MS))
    {
        // 校验和检查
        if (CalcChecksum(&rx_frame[2], rx_len - 3) == rx_frame[rx_len - 1])
        {
            label = rx_frame[rx_len - CAMERA_DATA_OFFSET];
            ESP_LOGI(TAG, "ReadLabelNonSpec: detected_label=0x%02X", label);
        }
        else
        {
            ESP_LOGW(TAG, "ReadLabelNonSpec checksum error");
        }
    }
    else
    {
        ESP_LOGW(TAG, "ReadLabelNonSpec timeout, id=0x%02X", id);
    }
    
    // 释放互斥锁
    xSemaphoreGive(mutex);
    return label;
}

/**
 * @brief 读取二维码数据
 * @param qr_buf   : 用于存放二维码数据的缓冲区（预留，当前未使用）
 * @param buf_len  : 输入时为缓冲区最大长度，输出时为实际接收到的数据长度（预留，当前未使用）
 * @return 检测结果（0x01=检测到，0x00=未检测到/超时）
 * @note TODO: 实现二维码数据解析，当前仅返回检测结果
 */
uint8_t Camera_ReadQR_Code(uint8_t id, uint8_t *qr_buf, uint8_t *buf_len)
{
    // 获取RS485互斥锁
    SemaphoreHandle_t mutex = uart0_get_rs485_mutex();
    if (mutex == NULL) {
        ESP_LOGE(TAG, "RS485互斥锁未初始化");
        return 0;
    }
    
    // 尝试获取互斥锁，最多等待200ms
    if (xSemaphoreTake(mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        ESP_LOGW(TAG, "获取RS485互斥锁超时，Camera_ReadQR_Code id=0x%02X", id);
        return 0;
    }
    
    uint8_t tx_data = FUNC_QRCODE;
    uint8_t rx_frame[CAMERA_FRAME_MAX_SIZE];
    uint16_t rx_len;

    SendFrame(id, CMD_READ, MODE_NON_SPECIFY, &tx_data, 1);
    
    uint8_t result = 0;
    if (RS485_WaitFrameById(id, rx_frame, &rx_len, CAMERA_RX_TIMEOUT_MS))
    {
        // 校验和检查
        if (CalcChecksum(&rx_frame[2], rx_len - 3) == rx_frame[rx_len - 1])
        {
            // TODO: 解析二维码数据到 qr_buf
            result = rx_frame[rx_len - CAMERA_DATA_OFFSET];
            ESP_LOGI(TAG, "ReadQR_Code: result=0x%02X", result);
        }
        else
        {
            ESP_LOGW(TAG, "ReadQR_Code checksum error");
        }
    }
    else
    {
        ESP_LOGW(TAG, "ReadQR_Code timeout, id=0x%02X", id);
    }
    
    // 释放互斥锁
    xSemaphoreGive(mutex);
    return result;
}
