#include "stm32f4xx.h"
#include "delay.h"
#include "oled.h"
#include "motor.h"
#include "encoder.h"
#include "speed_control.h"
#include "servo.h"
#include "ultrasonic.h"
#include "line_sensor.h"
#include "obstacle.h"
#include <stdio.h>

extern FONT_T tFont12; 

static void Keys_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13; 
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

static void FormatLineState(uint8_t state, char* buf) {
    for(int i=0; i<8; i++) buf[i] = (state & (1 << i)) ? '1' : '0';
    buf[8] = '\0';
}

int main(void) {
    char line1[20], line2[20], line3[20], line4[20], line_str[9];
    uint8_t oled_update_counter = 0;

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
        /* KEY1 ?? */
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET) {
            delay_ms(15);
            if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET) {
                if (SPEED_CTRL_GetStatus()) {
                    SPEED_CTRL_Stop(); 
                    OBSTACLE_Init(); 
                } else {
                    SPEED_CTRL_Start(); 
                }
                while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_RESET);
            }
        }

        /* ?? ????:?????????????? Delay ??,
           ?????????????,????? 10 ?! */
        if (SPEED_CTRL_GetStatus()) {
            OBSTACLE_Task();
        }

        /* ?? ????:??? 20 ??????????,????????????! */
        oled_update_counter++;
        if (oled_update_counter >= 20) {
            oled_update_counter = 0;

            FormatLineState(LINE_ReadAll(), line_str);
            uint16_t f = OBSTACLE_GetFrontCm();
            int16_t t_l = SPEED_CTRL_GetTargetLeft();
            int16_t t_r = SPEED_CTRL_GetTargetRight();
            
            sprintf(line1, "Ln:%s F:%03d", line_str, (f==999)?0:f);
            sprintf(line2, "St: %s", OBSTACLE_GetStateString());
            sprintf(line3, "T L:%-3d R:%-3d", t_l, t_r);
            sprintf(line4, "Enc L:%-3d R:%-3d", ENCODER_GetSpeedPulse(1), ENCODER_GetSpeedPulse(3));

            OLED_StartDraw();
            OLED_ClrScr(0x00);
            OLED_DispStr(0, 0,  line1, &tFont12);
            OLED_DispStr(0, 16, line2, &tFont12);
            OLED_DispStr(0, 32, line3, &tFont12);
            OLED_DispStr(0, 46, line4, &tFont12);
            OLED_EndDraw();
        }

        /* ????????? 2 ??,????????? */
        delay_ms(2); 
    }
}