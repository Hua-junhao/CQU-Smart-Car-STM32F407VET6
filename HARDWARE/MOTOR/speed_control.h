#ifndef __SPEED_CONTROL_H
#define __SPEED_CONTROL_H
#include "stm32f4xx.h"
#include <stdint.h>

void SPEED_CTRL_Init(void);
void SPEED_CTRL_Start(void);
void SPEED_CTRL_Stop(void);
// ?? ?????????,?? main.c ??????
int16_t SPEED_CTRL_GetTargetLeft(void);
int16_t SPEED_CTRL_GetTargetRight(void);

void SPEED_CTRL_SetSideTargetPulse(int16_t left, int16_t right);
uint16_t SPEED_CTRL_GetDuty(uint8_t motor);
uint8_t SPEED_CTRL_GetStatus(void);

#endif