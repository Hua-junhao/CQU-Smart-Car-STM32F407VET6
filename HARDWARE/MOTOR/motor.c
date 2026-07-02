#include "motor.h"

/* ?? PWM ??:20 ????,?? 5%;20 * 50us = 1ms,PWM??? 1kHz */
#define MOTOR_PWM_STEPS    20
#define MOTOR_PWM_TICK_US  50

static volatile uint8_t g_motor_duty_step[4] = {0, 0, 0, 0};
static volatile uint8_t g_motor_pwm_step = 0;

/* ?? GPIO ??????????? */
static void MOTOR_GPIO_Write(GPIO_TypeDef *port, uint16_t pin, uint8_t state)
{
    if (state)
    {
        GPIO_SetBits(port, pin);
    }
    else
    {
        GPIO_ResetBits(port, pin);
    }
}

/* ???????????? PWM ???? */
static void MOTOR_SetPWMPins(uint8_t m1, uint8_t m2, uint8_t m3, uint8_t m4)
{
    MOTOR_GPIO_Write(MOTOR1_PWM_PORT, MOTOR1_PWM_PIN, m1);
    MOTOR_GPIO_Write(MOTOR2_PWM_PORT, MOTOR2_PWM_PIN, m2);
    MOTOR_GPIO_Write(MOTOR3_PWM_PORT, MOTOR3_PWM_PIN, m3);
    MOTOR_GPIO_Write(MOTOR4_PWM_PORT, MOTOR4_PWM_PIN, m4);
}

/* ???(0~100)????? PWM ??(0~20) */
static uint8_t MOTOR_DutyToStep(uint16_t duty_percent)
{
    uint16_t duty_step;
    if (duty_percent > 100) 
    {
        duty_percent = 100;
    }
    duty_step = (duty_percent * MOTOR_PWM_STEPS + 50) / 100;
    
    /* ??????????????0??????? */
    if ((duty_percent > 0) && (duty_step == 0))
    {
        duty_step = 1;
    }
    return (uint8_t)duty_step;
}

/* ================== ??????? ================== */

/* 1. ????? EN ???? */
static void MOTOR_EnableGPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    GPIO_InitStructure.GPIO_Pin = MOTOR1_EN_PIN;
    GPIO_Init(MOTOR1_EN_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = MOTOR3_EN_PIN | MOTOR4_EN_PIN;
    GPIO_Init(MOTOR3_EN_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = MOTOR2_EN_PIN;
    GPIO_Init(MOTOR2_EN_PORT, &GPIO_InitStructure);
}

/* 2. ?????????? (?????) */
static void MOTOR_EnableAll(void)
{
    MOTOR_GPIO_Write(MOTOR1_EN_PORT, MOTOR1_EN_PIN, 1);
    MOTOR_GPIO_Write(MOTOR2_EN_PORT, MOTOR2_EN_PIN, 1);
    MOTOR_GPIO_Write(MOTOR3_EN_PORT, MOTOR3_EN_PIN, 1);
    MOTOR_GPIO_Write(MOTOR4_EN_PORT, MOTOR4_EN_PIN, 1);
}

/* 3. ???????(DIR)???PWM??(PWM)?? */
static void MOTOR_ControlGPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | 
                           RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    /* GPIOA (M3_DIR, M4_DIR) */
    GPIO_InitStructure.GPIO_Pin = MOTOR3_DIR_PIN | MOTOR4_DIR_PIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* GPIOB (M1_PWM, M1_DIR, M2_DIR, M4_PWM) */
    GPIO_InitStructure.GPIO_Pin = MOTOR1_PWM_PIN | MOTOR1_DIR_PIN | 
                                  MOTOR2_DIR_PIN | MOTOR4_PWM_PIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* GPIOD (M2_PWM) */
    GPIO_InitStructure.GPIO_Pin = MOTOR2_PWM_PIN;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* GPIOE (M3_PWM) */
    GPIO_InitStructure.GPIO_Pin = MOTOR3_PWM_PIN;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/* 4. ????????????? */
static void MOTOR_SetDirection(void)
{
    /* ?? M1/M2 ??: DIR=0 */
    MOTOR_GPIO_Write(MOTOR1_DIR_PORT, MOTOR1_DIR_PIN, 0);
    MOTOR_GPIO_Write(MOTOR2_DIR_PORT, MOTOR2_DIR_PIN, 0);

    /* ?? M3/M4 ??: M3 DIR=1, M4 DIR=0 (????????) */
    MOTOR_GPIO_Write(MOTOR3_DIR_PORT, MOTOR3_DIR_PIN, 1);
    MOTOR_GPIO_Write(MOTOR4_DIR_PORT, MOTOR4_DIR_PIN, 0);
}

/* 5. ??? TIM3 ???? (????PWM?????) */
static void MOTOR_TIM3_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* 50us ????? */
    TIM_TimeBaseStructure.TIM_Period        = MOTOR_PWM_TICK_US - 1;
    TIM_TimeBaseStructure.TIM_Prescaler     = 84 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel                   = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM3, ENABLE);
}

/* ??????? */
void MOTOR_Init(void)
{
    MOTOR_EnableGPIO_Init();
    MOTOR_ControlGPIO_Init();
    MOTOR_SetDirection();
    MOTOR_StopAll();
    MOTOR_EnableAll();
    MOTOR_TIM3_Init();
}

/* ================== ?????? ================== */

void MOTOR_SetDuty(uint8_t motor, uint16_t duty_percent)
{
    uint8_t duty_step = MOTOR_DutyToStep(duty_percent);
    switch (motor)
    {
        case 1: g_motor_duty_step[0] = duty_step; break;
        case 2: g_motor_duty_step[1] = duty_step; break;
        case 3: g_motor_duty_step[2] = duty_step; break;
        case 4: g_motor_duty_step[3] = duty_step; break;
        default: break;
    }
}

void MOTOR_SetAllDuty(uint16_t duty_percent)
{
    MOTOR_SetDuty(1, duty_percent);
    MOTOR_SetDuty(2, duty_percent);
    MOTOR_SetDuty(3, duty_percent);
    MOTOR_SetDuty(4, duty_percent);
}

void MOTOR_SetLeftDuty(uint16_t duty_percent)
{
    MOTOR_SetDuty(1, duty_percent);
    MOTOR_SetDuty(2, duty_percent);
}

void MOTOR_SetRightDuty(uint16_t duty_percent)
{
    MOTOR_SetDuty(3, duty_percent);
    MOTOR_SetDuty(4, duty_percent);
}

void MOTOR_StopAll(void)
{
    MOTOR_SetAllDuty(0);
    /* ???????;M3 ?????;M4 ?????,????? */
    MOTOR_SetPWMPins(0, 0, 1, 0); 
}

/* ================== TIM3 ?????? ================== */

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        uint8_t m1_pwm, m2_pwm, m3_pwm, m4_pwm;
        
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

        /* ????:PWM ??????? */
        m1_pwm = (g_motor_pwm_step < g_motor_duty_step[0]) ? 1 : 0;
        m2_pwm = (g_motor_pwm_step < g_motor_duty_step[1]) ? 1 : 0;
        
        /* M3 ???????,M4 ??????? */
        m3_pwm = (g_motor_pwm_step < g_motor_duty_step[2]) ? 0 : 1;
        m4_pwm = (g_motor_pwm_step < g_motor_duty_step[3]) ? 1 : 0;

        MOTOR_SetPWMPins(m1_pwm, m2_pwm, m3_pwm, m4_pwm);

        g_motor_pwm_step++;
        if (g_motor_pwm_step >= MOTOR_PWM_STEPS)
        {
            g_motor_pwm_step = 0;
        }
    }
}