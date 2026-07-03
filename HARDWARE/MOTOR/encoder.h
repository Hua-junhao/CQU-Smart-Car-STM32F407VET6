#ifndef __ENCODER_H
#define __ENCODER_H
#include "stm32f4xx.h"

void ENCODER_Init(void);
void ENCODER_Update(void);
int16_t ENCODER_GetSpeedPulse(uint8_t motor);

#endif