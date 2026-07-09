#ifndef __OBSTACLE_H
#define __OBSTACLE_H
#include "stm32f4xx.h"
void OBSTACLE_Init(void);
void OBSTACLE_Task(void);
uint16_t OBSTACLE_GetFrontCm(void);
const char* OBSTACLE_GetStateString(void);
#endif