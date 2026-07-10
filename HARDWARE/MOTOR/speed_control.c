#include "speed_control.h"
#include "encoder.h"
#include "motor.h"
#include <stdlib.h> 

#define TRIM_M1   0   
#define TRIM_M2   0   
#define TRIM_M3   0   
#define TRIM_M4   0   

/* ???????????? */
#define PID_KP  9  
#define PID_KI  3   
#define PID_KD  0  

typedef struct { int16_t integral; int16_t last_error; } PID_T;
static PID_T g_pid[4];
static volatile uint8_t g_speed_ctrl_enable = 0;
static volatile int16_t g_target_left = 0, g_target_right = 0;
static volatile int16_t g_target_ramp_left = 0, g_target_ramp_right = 0;
static volatile uint16_t g_motor_duty[4] = {0,0,0,0};

void SPEED_CTRL_Init(void) {
    TIM_TimeBaseInitTypeDef TIM; 
    NVIC_InitTypeDef NVIC_Init_Struct; 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
    TIM.TIM_Period = 49999; TIM.TIM_Prescaler = 84-1; TIM_TimeBaseInit(TIM7, &TIM);
    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
    NVIC_Init_Struct.NVIC_IRQChannel = TIM7_IRQn; 
    NVIC_Init_Struct.NVIC_IRQChannelPreemptionPriority = 2; 
    NVIC_Init_Struct.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_Init_Struct);
    TIM_Cmd(TIM7, ENABLE);
}

void SPEED_CTRL_Start(void) { g_speed_ctrl_enable = 1; }
void SPEED_CTRL_Stop(void)  { g_speed_ctrl_enable = 0; g_target_ramp_left = 0; g_target_ramp_right = 0; MOTOR_StopAll(); for(int i=0;i<4;i++) g_pid[i].integral=0; }
void SPEED_CTRL_SetSideTargetPulse(int16_t left, int16_t right) { g_target_left = left; g_target_right = right; }
int16_t SPEED_CTRL_GetTargetLeft(void) { return g_target_left; }
int16_t SPEED_CTRL_GetTargetRight(void) { return g_target_right; }
uint16_t SPEED_CTRL_GetDuty(uint8_t motor) { return (motor>=1 && motor<=4) ? g_motor_duty[motor-1] : 0; }
uint8_t SPEED_CTRL_GetStatus(void) { return g_speed_ctrl_enable; }

void TIM7_IRQHandler(void) {
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
        ENCODER_Update(); 

        if (!g_speed_ctrl_enable) { MOTOR_StopAll(); return; }

        /* ????:????,????,???? */
        if (g_target_ramp_left < g_target_left) g_target_ramp_left += 5; 
        else if (g_target_ramp_left > g_target_left) g_target_ramp_left -= 5;
        if (g_target_ramp_right < g_target_right) g_target_ramp_right += 5; 
        else if (g_target_ramp_right > g_target_right) g_target_ramp_right -= 5;

        for (int i = 0; i < 4; i++) {
            int16_t my_target = (i == 0 || i == 1) ? g_target_ramp_left : g_target_ramp_right;

            if (i == 0) my_target += TRIM_M1;
            if (i == 1) my_target += TRIM_M2;
            if (i == 2) my_target += TRIM_M3;
            if (i == 3) my_target += TRIM_M4;
            if (my_target < 0) my_target = 0;

            int16_t target_abs = abs(my_target);
            int16_t measured = ENCODER_GetSpeedPulse(i + 1); 
            int32_t error = target_abs - measured;
            
            if (error >= -1 && error <= 1) error = 0; 

            g_pid[i].integral += error;
            if (g_pid[i].integral > 80)  g_pid[i].integral = 80;   
            if (g_pid[i].integral < -80) g_pid[i].integral = -80;

            /* ?? ???? 1:?????? (Feed-Forward)
               ???? = 20 + ????*1.2?
               ???? 20 ??,??????? 44% ????!????????????! */
            int32_t base_duty = (target_abs == 0) ? 0 : (10 + (target_abs * 12 / 10)); 

            int32_t output = base_duty + (PID_KP * error + PID_KI * g_pid[i].integral) / 10;

            /* ?? ???? 2:???? (Stiction Override)
               ???????,???????(???0),?????? 15% ???????!
               ?????,? 15% ????,????! */
            if (target_abs > 0 && measured == 0) {
                output += 15; 
            }

            /* ??????? 85%,???????????? */
            if (output > 55) output = 55; 
            if (output < 0)  output = 0;

            g_motor_duty[i] = (uint16_t)output;
            MOTOR_SetDuty(i + 1, g_motor_duty[i]);
            
            /* ?????????? */
            if (my_target >= 0) MOTOR_SetDirectionDynamic(i + 1, 0); 
            else                MOTOR_SetDirectionDynamic(i + 1, 1); 
        }
    }
}