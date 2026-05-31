#ifndef TURN_DETECTION_H
#define TURN_DETECTION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 转弯类型定义
typedef enum {
    TURN_TYPE_NONE = 0,
    TURN_TYPE_LEFT = 1,
    TURN_TYPE_RIGHT = 2,
    TURN_TYPE_CROSSROAD = 3
} turn_type_t;

void turn_detection_init(void);
void turn_detection_start(void);
void turn_detection_stop(void);
uint8_t turn_detection_get_type(void);
bool turn_detection_is_turning(void);
void turn_detection_reset(void);

#ifdef __cplusplus
}
#endif

#endif
