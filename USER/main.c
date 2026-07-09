#include "stm32f4xx.h"
#include "delay.h"
#include "oled.h"
#include "motor.h"
#include "encoder.h"
#include "speed_control.h"
#include "servo.h"
#include "ultrasonic.h"
#include "obstacle.h"
#include "line_sensor.h"
#include <stdio.h>

extern FONT_T tFont12; 
uint8_t g_debug_straight_mode = 0; // 0: ????, 1: ?????

static void Keys_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; GPIO_Init(GPIOC, &GPIO_InitStructure); // KEY1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;  GPIO_Init(GPIOE, &GPIO_InitStructure); // KEY2
}

int main(void) {
    char line1[20], line2[20], line3[20], line4[20];

    delay_init(168);
    OLED_Init();
    Keys_Init();
    MOTOR_Init();
    ENCODER_Init();
    SPEED_CTRL_Init();
    SERVO_Init();
    ULTRASONIC_Init();
    LINE_Init();     
    OBSTACLE_Init();

    while (1) {
        /* KEY1 ????/?? */
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET) {
            delay_ms(15);
            if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET) {
                if (SPEED_CTRL_GetStatus()) SPEED_CTRL_Stop(); 
                else                        SPEED_CTRL_Start(); 
                while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET);
            }
        }

        /* KEY2 ?????? */
        if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == Bit_RESET) {
            delay_ms(15);
            if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == Bit_RESET) {
                g_debug_straight_mode = !g_debug_straight_mode; 
                while(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == Bit_RESET);
            }
        }

        if (SPEED_CTRL_GetStatus()) {
            OBSTACLE_Task();
        }

        /* ?? 4 ?????????????? */
        int16_t m1 = ENCODER_GetSpeedPulse(1);
        int16_t m2 = ENCODER_GetSpeedPulse(2);
        int16_t m3 = ENCODER_GetSpeedPulse(3);
        int16_t m4 = ENCODER_GetSpeedPulse(4);

        uint16_t d1 = SPEED_CTRL_GetDuty(1);
        uint16_t d2 = SPEED_CTRL_GetDuty(2);
        uint16_t d3 = SPEED_CTRL_GetDuty(3);
        uint16_t d4 = SPEED_CTRL_GetDuty(4);

        /* ?? ??????????? */
        sprintf(line1, "M:%s  Line:%02X", g_debug_straight_mode ? "STRAIGHT" : "TRACKING", LINE_ReadAll());
        sprintf(line2, "1:%-2d d%-2d  2:%-2d d%-2d", m1, d1, m2, d2);
        sprintf(line3, "3:%-2d d%-2d  4:%-2d d%-2d", m3, d3, m4, d4);
        sprintf(line4, "St:%s  T:%-2d", OBSTACLE_GetStateString(), SPEED_CTRL_GetTargetLeft());

        OLED_StartDraw();
        OLED_ClrScr(0x00);
        OLED_DispStr(0, 0,  line1, &tFont12);
        OLED_DispStr(0, 16, line2, &tFont12);
        OLED_DispStr(0, 32, line3, &tFont12);
        OLED_DispStr(0, 46, line4, &tFont12);
        OLED_EndDraw();

        delay_ms(30); 
    }
}