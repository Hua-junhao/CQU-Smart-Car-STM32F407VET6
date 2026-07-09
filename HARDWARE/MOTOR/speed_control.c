#include "speed_control.h"
#include "encoder.h"
#include "motor.h"
#include <stdlib.h> 

#define TRIM_M1  0   
#define TRIM_M2  0   
#define TRIM_M3  0   
#define TRIM_M4  0   

#define PID_KP  9  
#define PID_KI  3   
#define PID_KD  0  

typedef struct { int16_t integral; int16_t last_error; } PID_T;
static PID_T g_pid[4];
static volatile uint8_t g_speed_ctrl_enable = 0;
static volatile int16_t g_target_l = 0, g_target_r = 0;
static volatile uint16_t g_motor_duty[4] = {0,0,0,0};

void SPEED_CTRL_Init(void) {
    TIM_TimeBaseInitTypeDef TIM; 
    // ?? ????:????? NVIC ?????,?? NVIC_Config ??
    NVIC_InitTypeDef NVIC_Config; 
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
    TIM.TIM_Period = 49999; 
    TIM.TIM_Prescaler = 84 - 1;
    TIM.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM7, &TIM);

    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

    // ?? ?? NVIC_Config ????? NVIC
    NVIC_Config.NVIC_IRQChannel = TIM7_IRQn; 
    NVIC_Config.NVIC_IRQChannelPreemptionPriority = 2; 
    NVIC_Config.NVIC_IRQChannelSubPriority = 0;
    NVIC_Config.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_Config);
    TIM_Cmd(TIM7, ENABLE);
}

void SPEED_CTRL_Start(void) { g_speed_ctrl_enable = 1; }
void SPEED_CTRL_Stop(void)  { g_speed_ctrl_enable = 0; MOTOR_StopAll(); for(int i=0;i<4;i++) g_pid[i].integral=0; }

void SPEED_CTRL_SetSideTargetPulse(int16_t left, int16_t right) { 
    g_target_l = left; 
    g_target_r = right; 
}

int16_t SPEED_CTRL_GetTargetLeft(void) { return g_target_l; }
int16_t SPEED_CTRL_GetTargetRight(void) { return g_target_r; }
uint16_t SPEED_CTRL_GetDuty(uint8_t motor) { return (motor>=1 && motor<=4) ? g_motor_duty[motor-1] : 0; }
uint8_t SPEED_CTRL_GetStatus(void) { return g_speed_ctrl_enable; }

void TIM7_IRQHandler(void) {
    if (TIM_GetITStatus(TIM7, TIM_IT_Update)) {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
        ENCODER_Update(); 

        if (!g_speed_ctrl_enable) { MOTOR_StopAll(); return; }

        for (int i = 0; i < 4; i++) {
            int16_t target = (i < 2) ? g_target_l : g_target_r;
            if (i == 0) target += TRIM_M1;
            if (i == 1) target += TRIM_M2;
            if (i == 2) target += TRIM_M3;
            if (i == 3) target += TRIM_M4;

            int16_t measured = ENCODER_GetSpeedPulse(i + 1); 
            int32_t error = target - measured;
            
            g_pid[i].integral += error;
            if (g_pid[i].integral > 80) g_pid[i].integral = 80;   
            if (g_pid[i].integral < -80) g_pid[i].integral = -80;

            int32_t output = 25 + (PID_KP * error + PID_KI * g_pid[i].integral) / 10;

            if (output > 70) output = 70; 
            if (output < 0)  output = 0;

            g_motor_duty[i] = (uint16_t)output;
            MOTOR_SetDuty(i + 1, g_motor_duty[i]);
        }
    }
}