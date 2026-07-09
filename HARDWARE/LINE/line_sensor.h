#ifndef __LINE_SENSOR_H
#define __LINE_SENSOR_H

#include "stm32f4xx.h"
#include <stdint.h>

#define LINE_SENSOR_COUNT  8

/* 8?????????? */
#define LINE_CH1_PORT GPIOB
#define LINE_CH1_PIN  GPIO_Pin_0

#define LINE_CH2_PORT GPIOB
#define LINE_CH2_PIN  GPIO_Pin_1

#define LINE_CH3_PORT GPIOC
#define LINE_CH3_PIN  GPIO_Pin_4

#define LINE_CH4_PORT GPIOC
#define LINE_CH4_PIN  GPIO_Pin_5

#define LINE_CH5_PORT GPIOA
#define LINE_CH5_PIN  GPIO_Pin_4

#define LINE_CH6_PORT GPIOA
#define LINE_CH6_PIN  GPIO_Pin_5

#define LINE_CH7_PORT GPIOC
#define LINE_CH7_PIN  GPIO_Pin_3

#define LINE_CH8_PORT GPIOC
#define LINE_CH8_PIN  GPIO_Pin_2

void LINE_Init(void);
uint8_t LINE_ReadChannel(uint8_t channel);
uint8_t LINE_ReadAll(void);

#endif