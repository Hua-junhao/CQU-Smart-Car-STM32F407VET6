#ifndef __SERVO_H
#define __SERVO_H

#include "stm32f4xx.h"
#include <stdint.h>

#define SERVO_CENTER_ANGLE  90
#define SERVO_FRAME_US      20000 // 50Hz ?? = 20000?? (20??)

/* ???? */
void SERVO_Init(void);
void SERVO_SetAngle(uint8_t servo, uint8_t angle);
void SERVO_SetPulseUs(uint8_t servo, uint16_t pulse_us);
uint8_t SERVO_GetAngle(uint8_t servo);
uint16_t SERVO_GetPulseUs(uint8_t servo);

#endif