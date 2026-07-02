#include "beep.h"
#include "delay.h"

/* ????? */
void BEEP_On(void)
{
    GPIO_SetBits(BEEP_PORT, BEEP_PIN);
}

/* ????? */
void BEEP_Off(void)
{
    GPIO_ResetBits(BEEP_PORT, BEEP_PIN);
}

/* ???????? */
void BEEP_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* ???:?? GPIOE ?? */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    /* ???:?? PE5 ?????,???? */
    GPIO_InitStructure.GPIO_Pin   = BEEP_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
    GPIO_Init(BEEP_PORT, &GPIO_InitStructure);

    /* ???:???????? */
    BEEP_Off();
}

/* ???????? */
void BEEP_LowFrequency(void)
{
    BEEP_On();
    delay_us(1000); // ????
    BEEP_Off();
    delay_us(1000);
}

/* ???????? */
void BEEP_HighFrequency(void)
{
    BEEP_On();
    delay_us(50);   // ????,?????
    BEEP_Off();
    delay_us(50);
}
/* ???:?? 8 ??????,????????? */
void BEEP_LineSensorTone(uint8_t line_state)
{
    uint16_t delay_time;
    
    if(line_state == 0)
    {
        BEEP_Off(); // ?????????
        return;
    }
    
    /* ??:???????,line_state ????,?????????,?????? */
    delay_time = 50 + line_state * 2; 
    
    BEEP_On();
    delay_us(delay_time);
    BEEP_Off();
    delay_us(delay_time);
}