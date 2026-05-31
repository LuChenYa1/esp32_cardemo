#ifndef KEY_H
#define KEY_H

#include "driver/gpio.h"


#define KEY1_GPIO GPIO_NUM_38 
#define KEY2_GPIO GPIO_NUM_37  


void key_init(void);
void key_task(void);  

#endif /* KEY_H */