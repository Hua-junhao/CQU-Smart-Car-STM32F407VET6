#ifndef __OBSTACLE_H
#define __OBSTACLE_H

#include "stm32f4xx.h"
#include <stdint.h>

/* ???????? */
void OBSTACLE_Init(void);
void OBSTACLE_Task(void);

/* ??????(?? OLED ??) */
uint16_t OBSTACLE_GetFrontCm(void);
uint16_t OBSTACLE_GetLeftCm(void);
uint16_t OBSTACLE_GetRightCm(void);
const char* OBSTACLE_GetStateString(void);

#endif