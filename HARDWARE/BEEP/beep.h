#ifndef __BEEP_H
#define __BEEP_H

#include "stm32f4xx.h"

/* ????????? (GPIOE_Pin_5) */
#define BEEP_PORT GPIOE
#define BEEP_PIN  GPIO_Pin_5

/* ???? */
void BEEP_Init(void);
void BEEP_On(void);
void BEEP_Off(void);
void BEEP_LowFrequency(void);
void BEEP_HighFrequency(void);

#endif
void BEEP_LineSensorTone(uint8_t line_state);