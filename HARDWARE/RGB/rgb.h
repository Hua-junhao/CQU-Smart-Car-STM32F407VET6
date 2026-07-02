#ifndef __RGB_H
#define __RGB_H

#include "stm32f4xx.h"
#include <stdint.h>

/* RGB LED ?????? */
#define RGB_RED_PORT     GPIOA
#define RGB_RED_PIN      GPIO_Pin_0   // PA0 -> TIM2_CH1

#define RGB_BLUE_PORT    GPIOA
#define RGB_BLUE_PIN     GPIO_Pin_1   // PA1 -> TIM2_CH2

#define RGB_GREEN_PORT   GPIOE
#define RGB_GREEN_PIN    GPIO_Pin_6   // PE6 -> TIM9_CH2

/* ???? */
void RGB_Init(void);
void RGB_AllOff(void);
void RGB_BreathingCycle(void);

#endif