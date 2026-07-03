#include "speed_control.h"
#include "encoder.h"
#include "motor.h"

typedef struct { int16_t integral; int16_t last_error; } PID_T;
static PID_T g_pid[4];
static volatile uint8_t g_speed_ctrl_enable = 0;
static volatile int16_t g_target_speed = 15;
static volatile int16_t g_target_ramp = 0;
static volatile uint16_t g_motor_duty[4] = {0,0,0,0};

void SPEED_CTRL_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
    TIM_TimeBaseStructure.TIM_Period = 50000 - 1; // 50ms
    TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);

    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // PID ???
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_Cmd(TIM7, ENABLE);
}

void SPEED_CTRL_Start(void) { g_speed_ctrl_enable = 1; }
void SPEED_CTRL_Stop(void)  { g_speed_ctrl_enable = 0; g_target_ramp = 0; MOTOR_StopAll(); }
void SPEED_CTRL_SetTarget(int16_t target) { g_target_speed = target; }
int16_t SPEED_CTRL_GetTarget(void) { return g_target_speed; }
uint16_t SPEED_CTRL_GetDuty(uint8_t motor) { return (motor>=1 && motor<=4) ? g_motor_duty[motor-1] : 0; }
uint8_t SPEED_CTRL_GetStatus(void) { return g_speed_ctrl_enable; }

/* PID ?????? */
void TIM7_IRQHandler(void) {
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
        
        ENCODER_Update(); // ????

        if (!g_speed_ctrl_enable) { MOTOR_StopAll(); return; }

        if (g_target_ramp < g_target_speed) g_target_ramp += 1; // ????
        else if (g_target_ramp > g_target_speed) g_target_ramp -= 1;

        for (int i = 0; i < 4; i++) {
            int32_t error = g_target_ramp - ENCODER_GetSpeedPulse(i + 1);
            g_pid[i].integral += error;
            if (g_pid[i].integral > 30) g_pid[i].integral = 30;   // ??????
            if (g_pid[i].integral < -30) g_pid[i].integral = -30;

            int32_t base_duty = (g_target_ramp == 0) ? 0 : 20; // ???? 20%
            int32_t output = base_duty + (3 * error + 1 * g_pid[i].integral) / 10;

            if (output > 45) output = 45; // ?? ?????? 45%,?????!
            if (output < 0)  output = 0;

            g_motor_duty[i] = (uint16_t)output;
            MOTOR_SetDuty(i + 1, g_motor_duty[i]);
            g_pid[i].last_error = error;
        }
    }
}