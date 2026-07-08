#include "stm32f4xx.h"
#include "delay.h"
#include "oled.h"
#include "motor.h"
#include "encoder.h"
#include "speed_control.h"
#include <stdio.h>

extern FONT_T tFont12; 

/* ?? GPIO ??? (KEY1:PC13, KEY2:PE4) */
static void Keys_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; 
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;  
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/* ?? ??????????????? (?? main ???????) */
static void Scan_Keys_PID(void)
{
    static uint8_t key1_last = 1;
    static uint8_t key2_last = 1;
    
    uint8_t key1_now = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13);
    uint8_t key2_now = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4);

    /* KEY1 ?? ? ?? PID ???/?? (RUN / STOP) */
    if (key1_last == 1 && key1_now == 0) {
        delay_ms(15); // ????
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == 0) {
            if (SPEED_CTRL_GetStatus()) SPEED_CTRL_Stop(); 
            else                        SPEED_CTRL_Start(); 
        }
    }
    key1_last = key1_now;

    /* KEY2 ?? ? ????????? 10->20->30->40 ????? */
    if (key2_last == 1 && key2_now == 0) {
        delay_ms(15); // ????
        if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == 0) {
            int16_t t = SPEED_CTRL_GetTarget();
            t += 10;
            if (t > 40) t = 10; 
            SPEED_CTRL_SetTarget(t);
        }
    }
    key2_last = key2_now;
}

int main(void)
{
    char line1[20], line2[20], line3[20], line4[20];

    /* 1. ????? */
    delay_init(168);
    OLED_Init();
    Keys_Init();
    MOTOR_Init();
    ENCODER_Init();
    SPEED_CTRL_Init();

    /* 2. ????????????? */
    MOTOR_StopAll();

    while (1)
    {
        /* 3. ???? */
        Scan_Keys_PID();

        /* 4. ????????????? PID ????? */
        int16_t m1_spd = ENCODER_GetSpeedPulse(1);
        int16_t m2_spd = ENCODER_GetSpeedPulse(2);
        int16_t m3_spd = ENCODER_GetSpeedPulse(3);
        int16_t m4_spd = ENCODER_GetSpeedPulse(4);

        int16_t target  = SPEED_CTRL_GetTarget();
        uint16_t d1     = SPEED_CTRL_GetDuty(1);
        uint16_t d2     = SPEED_CTRL_GetDuty(2);
        uint16_t d3     = SPEED_CTRL_GetDuty(3);
        uint16_t d4     = SPEED_CTRL_GetDuty(4);

        /* 5. ??????:?????????????????? */
        int16_t avg_spd = (m1_spd + m2_spd + m3_spd + m4_spd) / 4;
        uint16_t avg_l_pwm = (d1 + d2) / 2;
        uint16_t avg_r_pwm = (d3 + d4) / 2;
        int16_t error      = target - avg_spd;

        /* 6. ????????? (????????? P9 ????) */
        sprintf(line1, "PID:%s T:%-2d A:%-2d", SPEED_CTRL_GetStatus() ? "RUN " : "STOP", target, avg_spd);
        sprintf(line2, "L:%-2d R:%-2d E:%-2d", avg_l_pwm, avg_r_pwm, error);
        sprintf(line3, "M1:%-2d M2:%-2d d1:%-2d", m1_spd, m2_spd, d1);
        sprintf(line4, "M3:%-2d M4:%-2d d3:%-2d", m3_spd, m4_spd, d3);

        /* 7. OLED ???? */
        OLED_StartDraw();
        OLED_ClrScr(0x00);
        OLED_DispStr(0, 0,  line1, &tFont12);
        OLED_DispStr(0, 16, line2, &tFont12);
        OLED_DispStr(0, 32, line3, &tFont12);
        OLED_DispStr(0, 46, line4, &tFont12);
        OLED_EndDraw();

        delay_ms(100); // 100ms ???,???????
    }
}