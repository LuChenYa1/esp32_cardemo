#ifndef SERVO_TASK_H
#define SERVO_TASK_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 舵机位置组结构体
 * 
 * 定义一组舵机的目标位置，用于实现往复运动
 */
typedef struct {
    uint16_t servo_id_02_pos;  // 舵机ID 0x02的位置
    uint16_t servo_id_03_pos;  // 舵机ID 0x03的位置
    uint16_t servo_id_04_pos;  // 舵机ID 0x04的位置
    uint16_t servo_id_05_pos;  // 舵机ID 0x05的位置
} ServoPositionGroup_t;

/**
 * @brief 创建舵机自动往复控制任务
 * 
 * 初始化RS485舵机通信，创建FreeRTOS任务实现舵机自动往复运动。
 * 任务优先级设置为3，每3秒在两组位置之间切换。
 * 
 * @return TaskHandle_t 返回创建的任务句柄，失败返回NULL
 */
TaskHandle_t servo_task_create(void);

/**
 * @brief 删除舵机任务
 * 
 * 停止舵机任务并释放相关资源
 * 
 * @param task_handle 要删除的任务句柄
 */
void servo_task_delete(TaskHandle_t task_handle);

/**
 * @brief 设置舵机位置组（带重试机制）
 * 
 * 向4个舵机发送位置命令，失败时自动重试最多3次
 * 
 * @param pos 舵机位置组指针
 * @return int 返回0表示成功，-1表示失败
 */
int servo_set_position_group_with_retry(const ServoPositionGroup_t *pos);

#ifdef __cplusplus
}
#endif

#endif // SERVO_TASK_H
