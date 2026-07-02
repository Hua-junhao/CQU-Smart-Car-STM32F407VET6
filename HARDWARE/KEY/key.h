#ifndef __KEY_H
#define __KEY_H

#include "stm32f4xx.h"

/* ??????:KEY1 ? PC13,KEY2 ? PE4 */
#define KEY1_PORT GPIOC
#define KEY1_PIN  GPIO_Pin_13

#define KEY2_PORT GPIOE
#define KEY2_PIN  GPIO_Pin_4

/* ?????????? */
#define BEEP_MODE_OFF  0
#define BEEP_MODE_LOW  1
#define BEEP_MODE_HIGH 2

/* ???? */
void KEY_Init(void);
uint8_t KEY_GetBeepMode(void);

#endif