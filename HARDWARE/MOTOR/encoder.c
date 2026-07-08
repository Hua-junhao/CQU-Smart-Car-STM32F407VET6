#include "encoder.h"

void ENCODER_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;

    /* ?? ?????????????? */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_TIM8, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 | RCC_APB1Periph_TIM4, ENABLE);

    /* ?????? (TIM1, TIM3, TIM4, TIM8) */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4); GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM3);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM8);  GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_TIM8);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9; GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13; GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;   GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* ?? 4 ??????????? */
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period    = 65535;
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
    TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure);

    TIM_EncoderInterfaceConfig(TIM1, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    TIM_EncoderInterfaceConfig(TIM8, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);

    /* ?? ????:? 4 ?????????? 15 ?(??)???????,????????????! */
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 15; 
    
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInit(TIM1, &TIM_ICInitStructure); TIM_ICInit(TIM3, &TIM_ICInitStructure);
    TIM_ICInit(TIM4, &TIM_ICInitStructure); TIM_ICInit(TIM8, &TIM_ICInitStructure);
    
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIM1, &TIM_ICInitStructure); TIM_ICInit(TIM3, &TIM_ICInitStructure);
    TIM_ICInit(TIM4, &TIM_ICInitStructure); TIM_ICInit(TIM8, &TIM_ICInitStructure);

    TIM_SetCounter(TIM1, 0); TIM_SetCounter(TIM3, 0);
    TIM_SetCounter(TIM4, 0); TIM_SetCounter(TIM8, 0);

    TIM_Cmd(TIM1, ENABLE); TIM_Cmd(TIM3, ENABLE);
    TIM_Cmd(TIM4, ENABLE); TIM_Cmd(TIM8, ENABLE);
}

static volatile int16_t g_speed_pulse[4] = {0,0,0,0};

/* 50ms ????????? */
void ENCODER_Update(void) {
    int16_t raw[4];
    
    /* ?? ??????? */
    raw[0] = (int16_t)TIM_GetCounter(TIM1); TIM_SetCounter(TIM1, 0); // M1
    raw[1] = (int16_t)TIM_GetCounter(TIM4); TIM_SetCounter(TIM4, 0); // M2
    raw[2] = (int16_t)TIM_GetCounter(TIM3); TIM_SetCounter(TIM3, 0); // M3
    raw[3] = (int16_t)TIM_GetCounter(TIM8); TIM_SetCounter(TIM8, 0); // M4

    for (int i = 0; i < 4; i++) {
        /* ?????? */
        int16_t speed = raw[i];
        if (i == 0 || i == 1 || i == 2) speed = -speed; // ????

        if (speed < 0) speed = -speed; // ????

        if (speed == 0) {
            g_speed_pulse[i] = 0;
        } else {
            g_speed_pulse[i] = (int16_t)((g_speed_pulse[i] * 2 + speed + 1) / 3); // ??????
        }
    }
}

int16_t ENCODER_GetSpeedPulse(uint8_t motor) {
    if (motor>=1 && motor<=4) return g_speed_pulse[motor - 1];
    return 0;
}