#include "speed_control.h"
#include "encoder.h"
#include "motor.h"
#include <stdlib.h> 

#define TRIM_M1  -1   
#define TRIM_M2  -1   
#define TRIM_M3   0   
#define TRIM_M4   0   

#define PID_KP  5  // ?????????
#define PID_KI  1  // ??????
#define PID_KD  0  

typedef struct { int16_t integral; int16_t last_error; } PID_T;
static PID_T g_pid[4];
static volatile uint8_t g_speed_ctrl_enable = 0;

static volatile int16_t g_target_left = 0;
static volatile int16_t g_target_right = 0;
static volatile int16_t g_target_ramp_left = 0;
static volatile int16_t g_target_ramp_right = 0;
static volatile uint16_t g_motor_duty[4] = {0,0,0,0};

void SPEED_CTRL_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
    TIM_TimeBaseStructure.TIM_Period = 50000 - 1; 
    TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);

    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_Cmd(TIM7, ENABLE);
}

void SPEED_CTRL_Start(void) { g_speed_ctrl_enable = 1; }
void SPEED_CTRL_Stop(void)  { g_speed_ctrl_enable = 0; g_target_ramp_left = 0; g_target_ramp_right = 0; MOTOR_StopAll(); }

void SPEED_CTRL_SetSideTargetPulse(int16_t left, int16_t right) { 
    g_target_left = left; 
    g_target_right = right; 
}

int16_t SPEED_CTRL_GetTargetLeft(void) { return g_target_left; }
int16_t SPEED_CTRL_GetTargetRight(void) { return g_target_right; }
uint16_t SPEED_CTRL_GetDuty(uint8_t motor) { return (motor>=1 && motor<=4) ? g_motor_duty[motor-1] : 0; }
uint8_t SPEED_CTRL_GetStatus(void) { return g_speed_ctrl_enable; }

void TIM7_IRQHandler(void) {
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
        
        ENCODER_Update(); 

        if (!g_speed_ctrl_enable) { MOTOR_StopAll(); return; }

        /* ????,???????????? */
        if (g_target_ramp_left < g_target_left) g_target_ramp_left += 5; 
        else if (g_target_ramp_left > g_target_left) g_target_ramp_left -= 5;

        if (g_target_ramp_right < g_target_right) g_target_ramp_right += 5; 
        else if (g_target_ramp_right > g_target_right) g_target_ramp_right -= 5;

        for (int i = 0; i < 4; i++) {
            int16_t my_target = 0;
            
            if (i == 0 || i == 1) { 
                my_target = g_target_ramp_left;
                if (g_target_ramp_left >= 0)  MOTOR_SetDirectionDynamic(i + 1, 0); 
                else                          MOTOR_SetDirectionDynamic(i + 1, 1); 
            } else { 
                my_target = g_target_ramp_right;
                if (g_target_ramp_right >= 0) MOTOR_SetDirectionDynamic(i + 1, 0); 
                else                          MOTOR_SetDirectionDynamic(i + 1, 1); 
            }

            if (i == 0) my_target += TRIM_M1;
            if (i == 1) my_target += TRIM_M2;
            if (i == 2) my_target += TRIM_M3;
            if (i == 3) my_target += TRIM_M4;

            int16_t target_abs = abs(my_target);
            int16_t measured = ENCODER_GetSpeedPulse(i + 1); 
            int32_t error = target_abs - measured;
            
            g_pid[i].integral += error;
            if (g_pid[i].integral > 100)  g_pid[i].integral = 100;   
            if (g_pid[i].integral < -100) g_pid[i].integral = -100;

            int32_t base_duty = (target_abs == 0) ? 0 : 25; 
            int32_t output = base_duty + (PID_KP * error + PID_KI * g_pid[i].integral) / 10;

            if (target_abs > 0 && measured <= 1 && output < 28) output = 28; 
            if (output > 65) output = 65; 
            if (output < 0)  output = 0;

            g_motor_duty[i] = (uint16_t)output;
            MOTOR_SetDuty(i + 1, g_motor_duty[i]);
            g_pid[i].last_error = error;
        }
    }
}