#include "obstacle.h"
#include "ultrasonic.h"
#include "line_sensor.h"
#include "speed_control.h"
#include "servo.h"
#include "delay.h"

static uint8_t g_state = 0; 
void OBSTACLE_Init(void) { g_state = 0; }
const char* OBSTACLE_GetStateString(void) { return (g_state==0)?"TRACK":"AVOID"; }

void OBSTACLE_Task(void) {
    uint16_t dist = ULTRASONIC_GetDistanceFilteredCm(1);
    
    // ???????? (22cm??)
    if(dist < 22 && dist > 0) {
        g_state = 1;
        SPEED_CTRL_SetSideTargetPulse(0,0); delay_ms(100);
        
        SERVO_SetAngle(1, 135); delay_ms(300); uint16_t l = ULTRASONIC_GetDistanceFilteredCm(1);
        SERVO_SetAngle(1, 45);  delay_ms(300); uint16_t r = ULTRASONIC_GetDistanceFilteredCm(1);
        SERVO_SetAngle(1, 90);
        
        // ????:????????
        if(l < r) SPEED_CTRL_SetSideTargetPulse(25, -25); 
        else      SPEED_CTRL_SetSideTargetPulse(-25, 25);
        
        delay_ms(800); // ????
        g_state = 0;
    } else {
        g_state = 0;
        uint8_t data = LINE_ReadAll();
        int16_t err = 0; int cnt = 0;
        for(int i=0; i<8; i++) if(data&(1<<i)) { err += (i-3.5)*5; cnt++; }
        
        // ????:??????????
        if(cnt) {
             SPEED_CTRL_SetSideTargetPulse(20+err, 20-err);
        } else {
             SPEED_CTRL_SetSideTargetPulse(20, 20);
        }
    }
}