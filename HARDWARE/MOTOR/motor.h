#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f4xx.h"
#include <stdint.h>

/* ================= ?????? ================= */
/* M1 ?? (??) */
#define MOTOR1_PWM_PORT GPIOB
#define MOTOR1_PWM_PIN  GPIO_Pin_13
#define MOTOR1_DIR_PORT GPIOB
#define MOTOR1_DIR_PIN  GPIO_Pin_15
#define MOTOR1_EN_PORT  GPIOC
#define MOTOR1_EN_PIN   GPIO_Pin_8

/* M2 ?? (??) */
#define MOTOR2_PWM_PORT GPIOD
#define MOTOR2_PWM_PIN  GPIO_Pin_8
#define MOTOR2_DIR_PORT GPIOB
#define MOTOR2_DIR_PIN  GPIO_Pin_14
#define MOTOR2_EN_PORT  GPIOE
#define MOTOR2_EN_PIN   GPIO_Pin_9

/* M3 ?? (??) */
#define MOTOR3_PWM_PORT GPIOE
#define MOTOR3_PWM_PIN  GPIO_Pin_7
#define MOTOR3_DIR_PORT GPIOA
#define MOTOR3_DIR_PIN  GPIO_Pin_2
#define MOTOR3_EN_PORT  GPIOD
#define MOTOR3_EN_PIN   GPIO_Pin_15

/* M4 ?? (??) */
#define MOTOR4_PWM_PORT GPIOB
#define MOTOR4_PWM_PIN  GPIO_Pin_2
#define MOTOR4_DIR_PORT GPIOA
#define MOTOR4_DIR_PIN  GPIO_Pin_3
#define MOTOR4_EN_PORT  GPIOD
#define MOTOR4_EN_PIN   GPIO_Pin_14

/* ================= ???? ================= */
void MOTOR_Init(void);
void MOTOR_SetDuty(uint8_t motor, uint16_t duty_percent);
void MOTOR_SetAllDuty(uint16_t duty_percent);
void MOTOR_SetLeftDuty(uint16_t duty_percent);
void MOTOR_SetRightDuty(uint16_t duty_percent);
void MOTOR_StopAll(void);

#endif