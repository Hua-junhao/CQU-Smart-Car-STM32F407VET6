#include "led.h"
#include "delay.h" 

// 1. ????
void LED_AllOff(void)
{
    GPIO_SetBits(LED1_PORT, LED1_PIN);
    GPIO_SetBits(LED2_PORT, LED2_PIN);
    GPIO_SetBits(LED3_PORT, LED3_PIN);
    GPIO_SetBits(LED4_PORT, LED4_PIN);
}

// 2. ?????
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = LED2_PIN;
    GPIO_Init(LED2_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = LED1_PIN | LED3_PIN | LED4_PIN;
    GPIO_Init(LED1_PORT, &GPIO_InitStructure);

    LED_AllOff();
}

// 3. ?????
void LED_FlowingWater(uint16_t interval_ms)
{
    LED_AllOff();
    LED_Write(LED1_PORT, LED1_PIN, LED_ON);
    delay_ms(interval_ms);

    LED_AllOff();
    LED_Write(LED2_PORT, LED2_PIN, LED_ON);
    delay_ms(interval_ms);

    LED_AllOff();
    LED_Write(LED3_PORT, LED3_PIN, LED_ON);
    delay_ms(interval_ms);

    LED_AllOff();
    LED_Write(LED4_PORT, LED4_PIN, LED_ON);
    delay_ms(interval_ms);
    
    LED_AllOff();
}