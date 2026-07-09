#include "encoder.h"

#define ENCODER_MIN_EDGE_TICKS 3 

static volatile int32_t g_encoder_raw_pulse[4] = {0, 0, 0, 0};
static volatile int16_t g_encoder_speed_pulse[4] = {0, 0, 0, 0};

static int8_t ENCODER_GetPolarity(uint8_t motor_index) {
    static const int8_t polarity[4] = {-1, -1, -1, 1}; 
    if (motor_index < 4) return polarity[motor_index];
    return 1;
}

void ENCODER_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    TIM_TimeBaseStructure.TIM_Period = 100 - 1; 
    TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1; 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

    TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM5, ENABLE);
}

void TIM5_IRQHandler(void) {
    if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
        
        uint8_t a[4], b[4];
        static uint8_t last_a[4] = {0,0,0,0};
        static uint8_t guard[4] = {0,0,0,0}; 
        
        a[0] = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8); b[0] = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9);
        a[1] = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12); b[1] = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_13);
        a[2] = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7); b[2] = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6);
        a[3] = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6); b[3] = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7);

        for (int i = 0; i < 4; i++) {
            if (guard[i] > 0) guard[i]--; 
            
            if (a[i] != last_a[i]) { 
                /* ?? ????:????????,???????????,????? last_a ??!
                   ??????“??????”? Bug,???????? 100% ??????! */
                if (guard[i] == 0) {
                    if (a[i] != b[i]) g_encoder_raw_pulse[i] += ENCODER_GetPolarity(i);
                    else              g_encoder_raw_pulse[i] -= ENCODER_GetPolarity(i);
                    guard[i] = ENCODER_MIN_EDGE_TICKS; 
                }
                last_a[i] = a[i]; // ?? ???????!
            }
        }
    }
}

void ENCODER_Update(void) {
    static int16_t history[4][4] = {0}; 
    static uint8_t h_index = 0;

    for (int i = 0; i < 4; i++) {
        int16_t speed = g_encoder_raw_pulse[i];
        g_encoder_raw_pulse[i] = 0; 
        
        if (speed < 0) speed = -speed; 
        
        history[i][h_index] = speed;

        int32_t sum = 0;
        for (int j = 0; j < 4; j++) {
            sum += history[i][j];
        }
        
        g_encoder_speed_pulse[i] = (int16_t)(sum / 4); 
    }
    
    h_index = (h_index + 1) % 4; 
}

int16_t ENCODER_GetSpeedPulse(uint8_t motor) {
    if (motor>=1 && motor<=4) return g_encoder_speed_pulse[motor - 1];
    return 0;
}