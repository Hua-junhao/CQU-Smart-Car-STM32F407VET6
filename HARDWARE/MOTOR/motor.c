#include "motor.h"

#define MOTOR_PWM_STEPS    20
#define MOTOR_PWM_TICK_US  50

static volatile uint8_t g_motor_duty_step[4] = {0, 0, 0, 0}; 
static volatile uint8_t g_motor_pwm_step = 0;                 
static volatile uint8_t g_motor_direction[4] = {0, 0, 0, 0}; 

static void MOTOR_GPIO_Write(GPIO_TypeDef *port, uint16_t pin, uint8_t state) {
    if (state)  GPIO_SetBits(port, pin);
    else        GPIO_ResetBits(port, pin);
}

static uint8_t MOTOR_DutyToStep(uint16_t duty_percent) {
    if (duty_percent > 100) duty_percent = 100;
    uint16_t duty_step = (duty_percent * MOTOR_PWM_STEPS + 50) / 100;
    if ((duty_percent > 0) && (duty_step == 0)) duty_step = 1;
    return (uint8_t)duty_step;
}

static void MOTOR_EnableGPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    GPIO_InitStructure.GPIO_Pin = MOTOR1_EN_PIN;  GPIO_Init(MOTOR1_EN_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = MOTOR2_EN_PIN;  GPIO_Init(MOTOR2_EN_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = MOTOR3_EN_PIN | MOTOR4_EN_PIN;  GPIO_Init(MOTOR3_EN_PORT, &GPIO_InitStructure);
}

static void MOTOR_EnableAll(void) {
    MOTOR_GPIO_Write(MOTOR1_EN_PORT, MOTOR1_EN_PIN, 1);
    MOTOR_GPIO_Write(MOTOR2_EN_PORT, MOTOR2_EN_PIN, 1);
    MOTOR_GPIO_Write(MOTOR3_EN_PORT, MOTOR3_EN_PIN, 1);
    MOTOR_GPIO_Write(MOTOR4_EN_PORT, MOTOR4_EN_PIN, 1);
}

static void MOTOR_ControlGPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | 
                           RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    GPIO_InitStructure.GPIO_Pin = MOTOR3_DIR_PIN | MOTOR4_DIR_PIN; GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = MOTOR1_PWM_PIN | MOTOR1_DIR_PIN | MOTOR2_DIR_PIN | MOTOR4_PWM_PIN; GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = MOTOR2_PWM_PIN; GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = MOTOR3_PWM_PIN; GPIO_Init(GPIOE, &GPIO_InitStructure);
}

static void MOTOR_SetDirection(void) {
    MOTOR_GPIO_Write(MOTOR1_DIR_PORT, MOTOR1_DIR_PIN, 0); 
    MOTOR_GPIO_Write(MOTOR2_DIR_PORT, MOTOR2_DIR_PIN, 0); 
    MOTOR_GPIO_Write(MOTOR3_DIR_PORT, MOTOR3_DIR_PIN, 0); 
    MOTOR_GPIO_Write(MOTOR4_DIR_PORT, MOTOR4_DIR_PIN, 0); 
}

void MOTOR_SetDirectionDynamic(uint8_t motor, uint8_t direction) {
    if (motor >= 1 && motor <= 4) g_motor_direction[motor - 1] = direction;
}

/* ?? ???????????? TIM5 ????? */
static void MOTOR_TIM5_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    TIM_TimeBaseStructure.TIM_Period        = MOTOR_PWM_TICK_US - 1;
    TIM_TimeBaseStructure.TIM_Prescaler     = 84 - 1; 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

    TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel                   = TIM5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM5, ENABLE);
}

void MOTOR_Init(void) {
    MOTOR_EnableGPIO_Init();
    MOTOR_ControlGPIO_Init();
    MOTOR_SetDirection();
    MOTOR_StopAll();
    MOTOR_EnableAll();
    MOTOR_TIM5_Init();
}

void MOTOR_SetDuty(uint8_t motor, uint16_t duty_percent) {
    uint8_t step = MOTOR_DutyToStep(duty_percent);
    if(motor>=1 && motor<=4) g_motor_duty_step[motor-1] = step;
}

void MOTOR_SetAllDuty(uint16_t duty_percent) {
    for(int i=1; i<=4; i++) MOTOR_SetDuty(i, duty_percent);
}

void MOTOR_StopAll(void) {
    MOTOR_SetAllDuty(0);
    MOTOR_GPIO_Write(MOTOR1_PWM_PORT, MOTOR1_PWM_PIN, 0); MOTOR_GPIO_Write(MOTOR1_DIR_PORT, MOTOR1_DIR_PIN, 0);
    MOTOR_GPIO_Write(MOTOR2_PWM_PORT, MOTOR2_PWM_PIN, 0); MOTOR_GPIO_Write(MOTOR2_DIR_PORT, MOTOR2_DIR_PIN, 0);
    MOTOR_GPIO_Write(MOTOR3_PWM_PORT, MOTOR3_PWM_PIN, 0); MOTOR_GPIO_Write(MOTOR3_DIR_PORT, MOTOR3_DIR_PIN, 0);
    MOTOR_GPIO_Write(MOTOR4_PWM_PORT, MOTOR4_PWM_PIN, 0); MOTOR_GPIO_Write(MOTOR4_DIR_PORT, MOTOR4_DIR_PIN, 0);
    g_motor_direction[0] = 0; g_motor_direction[1] = 0; g_motor_direction[2] = 0; g_motor_direction[3] = 0;
}

/* TIM5 ?????? PWM */
void TIM5_IRQHandler(void) {
    if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
        
        uint8_t pwm[4];
        for (int i = 0; i < 4; i++) {
            pwm[i] = (g_motor_pwm_step < g_motor_duty_step[i]) ? 1 : 0;
        }

        /* M1, M2 */
        if (g_motor_direction[0] == 0) { MOTOR_GPIO_Write(MOTOR1_PWM_PORT, MOTOR1_PWM_PIN, pwm[0]); MOTOR_GPIO_Write(MOTOR1_DIR_PORT, MOTOR1_DIR_PIN, 0); }
        else                           { MOTOR_GPIO_Write(MOTOR1_PWM_PORT, MOTOR1_PWM_PIN, 0);      MOTOR_GPIO_Write(MOTOR1_DIR_PORT, MOTOR1_DIR_PIN, pwm[0]); }
        if (g_motor_direction[1] == 0) { MOTOR_GPIO_Write(MOTOR2_PWM_PORT, MOTOR2_PWM_PIN, pwm[1]); MOTOR_GPIO_Write(MOTOR2_DIR_PORT, MOTOR2_DIR_PIN, 0); }
        else                           { MOTOR_GPIO_Write(MOTOR2_PWM_PORT, MOTOR2_PWM_PIN, 0);      MOTOR_GPIO_Write(MOTOR2_DIR_PORT, MOTOR2_DIR_PIN, pwm[1]); }

        /* M3 (??????) */
        if (g_motor_direction[2] == 0) { MOTOR_GPIO_Write(MOTOR3_PWM_PORT, MOTOR3_PWM_PIN, 0);      MOTOR_GPIO_Write(MOTOR3_DIR_PORT, MOTOR3_DIR_PIN, pwm[2]); }
        else                           { MOTOR_GPIO_Write(MOTOR3_PWM_PORT, MOTOR3_PWM_PIN, pwm[2]); MOTOR_GPIO_Write(MOTOR3_DIR_PORT, MOTOR3_DIR_PIN, 0); }

        /* M4 */
        if (g_motor_direction[3] == 0) { MOTOR_GPIO_Write(MOTOR4_PWM_PORT, MOTOR4_PWM_PIN, 0);      MOTOR_GPIO_Write(MOTOR4_DIR_PORT, MOTOR4_DIR_PIN, pwm[3]); }
        else                           { MOTOR_GPIO_Write(MOTOR4_PWM_PORT, MOTOR4_PWM_PIN, pwm[3]); MOTOR_GPIO_Write(MOTOR4_DIR_PORT, MOTOR4_DIR_PIN, 0); }

        g_motor_pwm_step++;
        if (g_motor_pwm_step >= MOTOR_PWM_STEPS) g_motor_pwm_step = 0;
    }
}