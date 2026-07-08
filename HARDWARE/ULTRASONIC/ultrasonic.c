#include "ultrasonic.h"
#include "delay.h"

static volatile uint32_t g_ultrasonic_trigger_count = 0;
static volatile uint32_t g_ultrasonic_last_wait_us = 0;
static volatile uint32_t g_ultrasonic_last_high_us = 0;

static void ULTRASONIC_WriteTrig(uint8_t state)
{
    if (state) GPIO_SetBits(ULTRASONIC_TRIG_PORT, ULTRASONIC_TRIG_PIN);
    else       GPIO_ResetBits(ULTRASONIC_TRIG_PORT, ULTRASONIC_TRIG_PIN);
}

static uint8_t ULTRASONIC_ReadEcho(void)
{
    return GPIO_ReadInputDataBit(ULTRASONIC_ECHO_PORT, ULTRASONIC_ECHO_PIN);
}

static void ULTRASONIC_TIM2_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    TIM_DeInit(TIM2);
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period        = 0xFFFFFFFF; // 32?????
    TIM_TimeBaseStructure.TIM_Prescaler     = 84 - 1;     // 84MHz??????84?? ? 1MHz (1us??)
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    TIM_Cmd(TIM2, ENABLE);
}

void ULTRASONIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = ULTRASONIC_TRIG_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(ULTRASONIC_TRIG_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = ULTRASONIC_ECHO_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(ULTRASONIC_ECHO_PORT, &GPIO_InitStructure);

    ULTRASONIC_WriteTrig(0);
    ULTRASONIC_TIM2_Init();
}

uint16_t ULTRASONIC_GetDistanceCm(void)
{
    uint32_t wait_us = 0;
    uint32_t high_us = 0;

    ULTRASONIC_WriteTrig(0);
    delay_us(2);
    ULTRASONIC_WriteTrig(1);
    delay_us(20);
    ULTRASONIC_WriteTrig(0);

    g_ultrasonic_trigger_count++;
    g_ultrasonic_last_wait_us = 0;
    g_ultrasonic_last_high_us = 0;

    /* 1. ?? ECHO ?? */
    while (!ULTRASONIC_ReadEcho())
    {
        delay_us(1);
        wait_us++;
        if (wait_us > 30000) // 30ms ??????
        {
            g_ultrasonic_last_wait_us = wait_us;
            return ULTRASONIC_INVALID_CM; // ?? 999
        }
    }

    /* 2. ?? ECHO ?? */
    TIM_SetCounter(TIM2, 0);
    while (ULTRASONIC_ReadEcho())
    {
        high_us = TIM_GetCounter(TIM2);
        delay_us(1);
        if (high_us > 30000) // 30ms ??????
        {
            g_ultrasonic_last_high_us = high_us;
            return ULTRASONIC_INVALID_CM; // ?? 999
        }
    }
    
    /* ?? ?????????????????? */
    high_us = TIM_GetCounter(TIM2);
    g_ultrasonic_last_high_us = high_us;

    if (high_us < 150)
    {
        return ULTRASONIC_NEAR_CM; // ??
    }

    return (uint16_t)(high_us / 58);
}

uint16_t ULTRASONIC_GetDistanceFilteredCm(uint8_t samples)
{
    uint8_t i;
    uint8_t valid_count = 0;
    uint32_t sum = 0;
    uint16_t distance;

    if (samples == 0)
    {
        samples = 1;
    }

    for (i = 0; i < samples; i++)
    {
        distance = ULTRASONIC_GetDistanceCm();
        
        if (distance == ULTRASONIC_NEAR_CM)
        {
            return ULTRASONIC_NEAR_CM;
        }

        if ((distance > 2) && (distance < ULTRASONIC_INVALID_CM))
        {
            sum += distance;
            valid_count++;
        }
        delay_ms(20); 
    }

    if (valid_count == 0)
    {
        return ULTRASONIC_INVALID_CM; 
    }

    return (uint16_t)(sum / valid_count);
}

uint8_t ULTRASONIC_GetEchoState(void)
{
    return ULTRASONIC_ReadEcho();
}

uint32_t ULTRASONIC_GetLastWaitUs(void)
{
    return g_ultrasonic_last_high_us;
}