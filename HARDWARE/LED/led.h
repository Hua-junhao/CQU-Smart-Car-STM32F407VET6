#ifndef __LED_H
#define __LED_H

#include "stm32f4xx.h"

// 1. ????
#define LED1_PORT   GPIOD
#define LED1_PIN    GPIO_Pin_7

#define LED2_PORT   GPIOB
#define LED2_PIN    GPIO_Pin_3

#define LED3_PORT   GPIOD
#define LED3_PIN    GPIO_Pin_5

#define LED4_PORT   GPIOD
#define LED4_PIN    GPIO_Pin_6

// 2. ??????
#define LED_ON      0
#define LED_OFF     1

// 3. ?????
#define LED_Write(PORT, PIN, STATE) ((STATE) ? GPIO_SetBits(PORT, PIN) : GPIO_ResetBits(PORT, PIN))

// 4. ????(??:??????,?????,?????????!)
void LED_Init(void);
void LED_AllOff(void);
void LED_FlowingWater(uint16_t interval_ms);

#endif