#ifndef __ULTRASONIC_H
#define __ULTRASONIC_H

#include "stm32f4xx.h"
#include <stdint.h>

#define ULTRASONIC_TRIG_PORT    GPIOB
#define ULTRASONIC_TRIG_PIN     GPIO_Pin_4   // TR ????? PB4

#define ULTRASONIC_ECHO_PORT    GPIOB
#define ULTRASONIC_ECHO_PIN     GPIO_Pin_5   // EC ????? PB5

#define ULTRASONIC_INVALID_CM   999          // ?????
#define ULTRASONIC_NEAR_CM      0            // ?????

void ULTRASONIC_Init(void);
uint16_t ULTRASONIC_GetDistanceCm(void);
uint16_t ULTRASONIC_GetDistanceFilteredCm(uint8_t samples);
uint8_t ULTRASONIC_GetEchoState(void);
uint32_t ULTRASONIC_GetLastWaitUs(void);

#endif