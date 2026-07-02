#include "key.h"
#include "beep.h"
#include "delay.h"

/* ??????????????,?? volatile ????????????? */
static volatile uint8_t g_beep_mode = BEEP_MODE_OFF;

/* ?? KEY1 ??? */
static uint8_t KEY1_IsPressed(void)
{
    if (GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN) == Bit_RESET)
    {
        return 1; // ??
    }
    return 0; // ??
}

/* ?? KEY2 ??? */
static uint8_t KEY2_IsPressed(void)
{
    if (GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN) == Bit_RESET)
    {
        return 1; // ??
    }
    return 0; // ??
}

/* ??????????(KEY2 ????) */
static void KEY_UpdateBeepMode(void)
{
    if (KEY2_IsPressed())
    {
        g_beep_mode = BEEP_MODE_HIGH;
    }
    else if (KEY1_IsPressed())
    {
        g_beep_mode = BEEP_MODE_LOW;
    }
    else
    {
        g_beep_mode = BEEP_MODE_OFF;
        BEEP_Off(); // ??????,???????
    }
}

/* ???????????? */
uint8_t KEY_GetBeepMode(void)
{
    return g_beep_mode;
}

/* ????????SYSCFG???EXTI????NVIC????? */
void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 1. ???? */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); // ????????????

    /* 2. ?? KEY1 (PC13) ????? */
    GPIO_InitStructure.GPIO_Pin   = KEY1_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(KEY1_PORT, &GPIO_InitStructure);

    /* 3. ?? KEY2 (PE4) ????? */
    GPIO_InitStructure.GPIO_Pin   = KEY2_PIN;
    GPIO_Init(KEY2_PORT, &GPIO_InitStructure);

    /* 4. ?? GPIO ? EXTI ????? */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13); // PC13 -> EXTI13
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource4);  // PE4  -> EXTI4

    /* 5. ?? EXTI Line13 (KEY1),??????????(?????,???) */
    EXTI_InitStructure.EXTI_Line    = EXTI_Line13;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* 6. ?? EXTI Line4 (KEY2),?? */
    EXTI_InitStructure.EXTI_Line    = EXTI_Line4;
    EXTI_Init(&EXTI_InitStructure);

    /* 7. ?? NVIC ?????:KEY1 ?? EXTI15_10_IRQn */
    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // ????? 2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 2; // ????? 2
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 8. ?? NVIC ?????:KEY2 ?? EXTI4_IRQn */
    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI4_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    /* ????????????? */
    KEY_UpdateBeepMode();
}

/* ================== ?????? ================== */

/* KEY1 (EXTI13) ???? */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line13) != RESET)
    {
        delay_ms(20); // ????
        KEY_UpdateBeepMode();
        EXTI_ClearITPendingBit(EXTI_Line13); // ?????????
    }
}

/* KEY2 (EXTI4) ???? */
void EXTI4_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        delay_ms(20); // ????
        KEY_UpdateBeepMode();
        EXTI_ClearITPendingBit(EXTI_Line4);  // ?????????
    }
}