#include "rgb.h"
#include "delay.h"

/* 
   ???????? (indexWave)?
   ? 50 ?,??????????????????
   ?????????????????(0, 1, 2, 3, 4...),????????;
   ?????????,????????????
*/
static const uint16_t indexWave[50] = {
    0, 1, 2, 3, 4, 5, 6, 8, 10, 12,
    15, 18, 22, 26, 31, 37, 43, 50, 58, 68,
    79, 91, 104, 119, 136, 157, 178, 201, 230, 258,
    294, 331, 373, 417, 467, 518, 575, 634, 697, 755,
    813, 866, 911, 946, 970, 985, 993, 997, 998, 999
};

/* 
   ?? 1 ???????:
   - ??: 50 ?
   - ????: 10 ??
   - ????(??+??): 50 * 10ms + 50 * 10ms = 1000ms = 1.0 ?!
*/
#define BREATH_STEPS            50
#define BREATH_STEP_DELAY_MS    10
#define RGB_OFF_DELAY_MS        100   // ????????? 100ms

/* ================== ????????????? ================== */
static void RGB_SetRed(uint16_t duty)
{
    TIM_SetCompare1(TIM2, duty);
}

static void RGB_SetBlue(uint16_t duty)
{
    TIM_SetCompare2(TIM2, duty);
}

static void RGB_SetGreen(uint16_t duty)
{
    TIM_SetCompare2(TIM9, duty);
}

/* ???? RGB ? */
void RGB_AllOff(void)
{
    RGB_SetRed(0);
    RGB_SetBlue(0);
    RGB_SetGreen(0);
}

/* ================== ??????? ================== */

static void RGB_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM2);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource6, GPIO_AF_TIM9);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

    GPIO_InitStructure.GPIO_Pin   = RGB_RED_PIN | RGB_BLUE_PIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = RGB_GREEN_PIN;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

static void RGB_TIM2_PWM_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseStructure.TIM_Period        = 999;
    TIM_TimeBaseStructure.TIM_Prescaler     = 84 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_Low;
    TIM_OCInitStructure.TIM_Pulse       = 0;

    TIM_OC1Init(TIM2, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_OC2Init(TIM2, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

static void RGB_TIM9_PWM_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);

    TIM_TimeBaseStructure.TIM_Period        = 999;
    TIM_TimeBaseStructure.TIM_Prescaler     = 168 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM9, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_Low;
    TIM_OCInitStructure.TIM_Pulse       = 0;

    TIM_OC2Init(TIM9, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM9, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM9, ENABLE);
    TIM_Cmd(TIM9, ENABLE);
}

void RGB_Init(void)
{
    RGB_GPIO_Init();
    RGB_TIM2_PWM_Init();
    RGB_TIM9_PWM_Init();
    RGB_AllOff();
}

/* ================== ???????????? ================== */

static void RGB_BreatheRed(void)
{
    int16_t i;
    
    /* 1. ????(?? 0.5?) */
    for (i = 0; i < BREATH_STEPS; i++)
    {
        RGB_SetRed(indexWave[i]);
        delay_ms(BREATH_STEP_DELAY_MS);
    }
    /* 2. ????(?? 0.5?) */
    for (i = BREATH_STEPS - 1; i >= 0; i--)
    {
        RGB_SetRed(indexWave[i]);
        delay_ms(BREATH_STEP_DELAY_MS);
    }
    
    RGB_SetRed(0); // ????
    delay_ms(RGB_OFF_DELAY_MS);
}

static void RGB_BreatheGreen(void)
{
    int16_t i;
    
    /* 1. ????(?? 0.5?) */
    for (i = 0; i < BREATH_STEPS; i++)
    {
        RGB_SetGreen(indexWave[i]);
        delay_ms(BREATH_STEP_DELAY_MS);
    }
    /* 2. ????(?? 0.5?) */
    for (i = BREATH_STEPS - 1; i >= 0; i--)
    {
        RGB_SetGreen(indexWave[i]);
        delay_ms(BREATH_STEP_DELAY_MS);
    }
    
    RGB_SetGreen(0);
    delay_ms(RGB_OFF_DELAY_MS);
}

static void RGB_BreatheBlue(void)
{
    int16_t i;
    
    /* 1. ????(?? 0.5?) */
    for (i = 0; i < BREATH_STEPS; i++)
    {
        RGB_SetBlue(indexWave[i]);
        delay_ms(BREATH_STEP_DELAY_MS);
    }
    /* 2. ????(?? 0.5?) */
    for (i = BREATH_STEPS - 1; i >= 0; i--)
    {
        RGB_SetBlue(indexWave[i]);
        delay_ms(BREATH_STEP_DELAY_MS);
    }
    
    RGB_SetBlue(0);
    delay_ms(RGB_OFF_DELAY_MS);
}

/* ??????? */
void RGB_BreathingCycle(void)
{
    RGB_BreatheRed();
    RGB_BreatheGreen();
    RGB_BreatheBlue();
}