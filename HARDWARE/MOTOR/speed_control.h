#ifndef __SPEED_CONTROL_H
#define __SPEED_CONTROL_H
#include "stm32f4xx.h"

void SPEED_CTRL_Init(void);
void SPEED_CTRL_Start(void);
void SPEED_CTRL_Stop(void);
void SPEED_CTRL_SetTarget(int16_t target);
int16_t SPEED_CTRL_GetTarget(void);
uint16_t SPEED_CTRL_GetDuty(uint8_t motor);
uint8_t SPEED_CTRL_GetStatus(void);

#endif