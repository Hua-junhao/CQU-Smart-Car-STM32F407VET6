#ifndef __LINE_SENSOR_H
#define __LINE_SENSOR_H
#include "stm32f4xx.h"
#include <stdint.h>

void LINE_Init(void);
uint8_t LINE_ReadAll(void);

#endif