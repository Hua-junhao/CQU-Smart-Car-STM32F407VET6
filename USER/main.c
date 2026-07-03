#include "stm32f4xx.h"
#include "delay.h"
#include "oled.h"
#include "motor.h"
#include "encoder.h"
#include "speed_control.h"
#include <stdio.h>

extern FONT_T tFont12; 

static void Keys_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;  GPIO_Init(GPIOE, &GPIO_InitStructure);
}

int main(void) {
    char line1[20], line2[20], line3[20], line4[20];

    delay_init(168);
    OLED_Init();
    Keys_Init();
    MOTOR_Init();
    ENCODER_Init();
    SPEED_CTRL_Init();

    while (1) {
        /* ???? (????) */
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET) {
            delay_ms(20);
            if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET) {
                if (SPEED_CTRL_GetStatus()) SPEED_CTRL_Stop(); 
                else                        SPEED_CTRL_Start(); 
                while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET);
            }
        }

        if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == Bit_RESET) {
            delay_ms(20);
            if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == Bit_RESET) {
                int16_t t = SPEED_CTRL_GetTarget();
                t += 10; if (t > 40) t = 10; 
                SPEED_CTRL_SetTarget(t);
                while(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == Bit_RESET);
            }
        }

        /* ?? OLED */
        sprintf(line1, "PID:%s T:%-2d", SPEED_CTRL_GetStatus() ? "RUN " : "STOP", SPEED_CTRL_GetTarget());
        sprintf(line2, "M1:%-2d  d1:%-2d", ENCODER_GetSpeedPulse(1), SPEED_CTRL_GetDuty(1));
        sprintf(line3, "M2:%-2d  d2:%-2d", ENCODER_GetSpeedPulse(2), SPEED_CTRL_GetDuty(2));
        sprintf(line4, "M3:%-2d  M4:%-2d", ENCODER_GetSpeedPulse(3), ENCODER_GetSpeedPulse(4));

        OLED_StartDraw();
        OLED_ClrScr(0x00);
        OLED_DispStr(0, 0,  line1, &tFont12);
        OLED_DispStr(0, 16, line2, &tFont12);
        OLED_DispStr(0, 32, line3, &tFont12);
        OLED_DispStr(0, 46, line4, &tFont12);
        OLED_EndDraw();

        delay_ms(100); 
    }
}