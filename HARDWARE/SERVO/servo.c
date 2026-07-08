#include "servo.h"

static uint8_t g_servo_angle = SERVO_CENTER_ANGLE;
static uint16_t g_servo_pulse_us = 1500;

/* ? 0~180? ????? 500us ~ 2500us ?? */
uint16_t SERVO_AngleToPulseUs(uint8_t angle)
{
    if (angle > 180) angle = 180;
    return (uint16_t)(500 + ((uint32_t)angle * 2000 / 180));
}

/* ????? TIM11_CH1 (PB9 ??) ??? */
static void SERVO_TIM11_PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    /* ?? GPIOB ? TIM11 ?? */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11, ENABLE); // TIM11 ?? APB2 ? (168MHz)

    /* ?? PB9 ????? TIM11 (?? P2 ??) */
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_TIM11);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    TIM_DeInit(TIM11);

    /* ??????:????????? 1MHz (1us ????) */
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period        = SERVO_FRAME_US - 1; // 19999 (20000us ?? 50Hz ??)
    TIM_TimeBaseStructure.TIM_Prescaler     = 168 - 1;            // 168Mhz ?? ? 1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM11, &TIM_TimeBaseStructure);

    /* ???? PWM1 ?? (?????) */
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = g_servo_pulse_us;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OC1Init(TIM11, &TIM_OCInitStructure);
    
    TIM_OC1PreloadConfig(TIM11, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM11, ENABLE);

    TIM_Cmd(TIM11, ENABLE);
}

/* ??? */
void SERVO_Init(void)
{
    g_servo_angle = SERVO_CENTER_ANGLE;
    g_servo_pulse_us = SERVO_AngleToPulseUs(g_servo_angle);
    SERVO_TIM11_PWM_Init();
}

/* ?????? (500us ~ 2500us) */
void SERVO_SetPulseUs(uint8_t servo, uint16_t pulse_us)
{
    if (pulse_us < 500)  pulse_us = 500;
    if (pulse_us > 2500) pulse_us = 2500;
    g_servo_pulse_us = pulse_us;
    
    /* ????? CCR1 ????????? */
    TIM_SetCompare1(TIM11, g_servo_pulse_us);
    
    /* ???????????,??? */
    g_servo_angle = (uint8_t)((pulse_us - 500) * 180 / 2000);
}

/* ?????? (0 ~ 180?) */
void SERVO_SetAngle(uint8_t servo, uint8_t angle)
{
    if (angle > 180) angle = 180;
    g_servo_angle = angle;
    uint16_t pulse_us = SERVO_AngleToPulseUs(angle);
    SERVO_SetPulseUs(servo, pulse_us);
}

/* ?????? */
uint8_t SERVO_GetAngle(uint8_t servo)
{
    return g_servo_angle;
}

/* ?????? (us) */
uint16_t SERVO_GetPulseUs(uint8_t servo)
{
    return g_servo_pulse_us;
}