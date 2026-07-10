#include "obstacle.h"
#include "ultrasonic.h"
#include "servo.h"
#include "line_sensor.h"
#include "speed_control.h"
#include "delay.h"

/* ================== ?? ??????? ================== */
#define OBSTACLE_SAFE_CM     22   
#define SERVO_CENTER_ANGLE   90   
#define SERVO_LEFT_ANGLE     135  
#define SERVO_RIGHT_ANGLE    45   

/* ?? ???? 1:??????? 24,???????,????! */
#define BASE_SPEED           22   

/* ?? ???? 2:????? 4,??? S ??????????,????????! */
#define TURN_SENSITIVITY     4  			  

/* ???????????? 22,?????????? */
#define SPIN_SPEED           18   

#define INVERT_TRACKING_DIR  0    
#define EVADE_TURN_SPEED     25   
#define OBSTACLE_TURN_MS     800  

typedef enum { STATE_FORWARD=0, STATE_SCAN, STATE_LEFT, STATE_RIGHT, STATE_ESCAPE } OBSTACLE_STATE_T;
static OBSTACLE_STATE_T g_state = STATE_FORWARD;
static int16_t last_turn_error = 0; 

void OBSTACLE_Init(void) { g_state = STATE_FORWARD; SERVO_SetAngle(1, SERVO_CENTER_ANGLE); SPEED_CTRL_SetSideTargetPulse(0,0); }
uint16_t OBSTACLE_GetFrontCm(void) { return ULTRASONIC_GetDistanceFilteredCm(1); }
uint16_t OBSTACLE_GetLeftCm(void)  { return 0; } 
uint16_t OBSTACLE_GetRightCm(void) { return 0; }
const char* OBSTACLE_GetStateString(void) { return (g_state == STATE_FORWARD) ? "TRACKING" : "AVOIDING"; }

static void Process_LineTracking(void) {
    uint8_t line_data = LINE_ReadAll();
    int16_t error = 0;
    int16_t active_sensors = 0;

    for (int i = 0; i < 8; i++) {
        if (line_data & (1 << i)) {
            error += (i - 3.5) * 10; 
            active_sensors++;
        }
    }

    if (active_sensors > 0 && active_sensors <= 5) {
        error = error / active_sensors; 
        last_turn_error = error; 
        
        int16_t turn_val = (error * TURN_SENSITIVITY) / 10;
        if (INVERT_TRACKING_DIR) turn_val = -turn_val;

        /* ?? ????:?????,???????????,???? S ?! */
        int16_t left_spd  = BASE_SPEED + turn_val;
        int16_t right_spd = BASE_SPEED - turn_val;
        SPEED_CTRL_SetSideTargetPulse(left_spd, right_spd);
    } 
    else if (active_sensors > 5) {
        /* ????:???? */
        SPEED_CTRL_SetSideTargetPulse(BASE_SPEED, BASE_SPEED);
    }
    else {
        /* ?? ???/???:????,??????????“????”??????! */
        if (last_turn_error > 0) {
            SPEED_CTRL_SetSideTargetPulse(SPIN_SPEED, -SPIN_SPEED); 
        } else {
            SPEED_CTRL_SetSideTargetPulse(-SPIN_SPEED, SPIN_SPEED); 
        }
    }
}

void OBSTACLE_Task(void) {
    if (!SPEED_CTRL_GetStatus()) { SERVO_SetAngle(1, SERVO_CENTER_ANGLE); return; }

    SERVO_SetAngle(1, SERVO_CENTER_ANGLE);
    uint16_t dist = ULTRASONIC_GetDistanceFilteredCm(1);

    if (dist >= OBSTACLE_SAFE_CM || dist == ULTRASONIC_INVALID_CM) {
        g_state = STATE_FORWARD;
        Process_LineTracking(); 
    } else {
        g_state = STATE_SCAN; SPEED_CTRL_SetSideTargetPulse(0, 0); delay_ms(100); 
        SERVO_SetAngle(1, SERVO_LEFT_ANGLE);  delay_ms(150); uint16_t l = ULTRASONIC_GetDistanceFilteredCm(1);
        SERVO_SetAngle(1, SERVO_RIGHT_ANGLE); delay_ms(150); uint16_t r = ULTRASONIC_GetDistanceFilteredCm(1);
        SERVO_SetAngle(1, SERVO_CENTER_ANGLE); delay_ms(50);

        if (l < r) { g_state = STATE_RIGHT; SPEED_CTRL_SetSideTargetPulse(EVADE_TURN_SPEED, -EVADE_TURN_SPEED); } 
        else       { g_state = STATE_LEFT;  SPEED_CTRL_SetSideTargetPulse(-EVADE_TURN_SPEED, EVADE_TURN_SPEED); }

        g_state = STATE_ESCAPE; delay_ms(OBSTACLE_TURN_MS); 
        SPEED_CTRL_SetSideTargetPulse(0, 0); delay_ms(50);
        g_state = STATE_FORWARD;
    }
}