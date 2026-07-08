#include "obstacle.h"
#include "ultrasonic.h"
#include "servo.h"
#include "speed_control.h"
#include "line_sensor.h"
#include "delay.h"

#define OBSTACLE_SAFE_CM             24   
#define SERVO_CENTER_ANGLE           90   
#define SERVO_LEFT_ANGLE             150  
#define SERVO_RIGHT_ANGLE            30   

/* ?? ?????? */
#define LINE_BASE_SPEED              15   // ?????? (??????????)
#define LINE_KP                      4    // ???????? (??? 0.4)

#define OBSTACLE_TURN_INNER_PULSE    -10  
#define OBSTACLE_TURN_OUTER_PULSE    22   
#define OBSTACLE_TURN_MS             800  

typedef enum {
    STATE_FORWARD = 0,
    STATE_SCAN,
    STATE_LEFT,
    STATE_RIGHT,
    STATE_ESCAPE
} OBSTACLE_STATE_T;

static OBSTACLE_STATE_T g_state = STATE_FORWARD;
static uint16_t g_front_dist = 0, g_left_dist = 0, g_right_dist = 0;

void OBSTACLE_Init(void) {
    g_state = STATE_FORWARD;
    SERVO_SetAngle(1, SERVO_CENTER_ANGLE);
    SPEED_CTRL_SetSideTargetPulse(0, 0);
}

uint16_t OBSTACLE_GetFrontCm(void) { return g_front_dist; }
uint16_t OBSTACLE_GetLeftCm(void)  { return g_left_dist;  }
uint16_t OBSTACLE_GetRightCm(void) { return g_right_dist; }

const char* OBSTACLE_GetStateString(void) {
    switch (g_state) {
        case STATE_FORWARD: return "TRACKING";
        case STATE_SCAN:    return "SCAN    ";
        case STATE_LEFT:    return "LEFT    ";
        case STATE_RIGHT:   return "RIGHT   ";
        case STATE_ESCAPE:  return "ESCAPE  ";
        default:            return "UNKNOWN ";
    }
}

void OBSTACLE_Task(void) {
    if (!SPEED_CTRL_GetStatus()) {
        g_state = STATE_FORWARD;
        return;
    }

    if (g_state == STATE_FORWARD) {
        /* ?? 1. ????????(???????,???????) */
        g_front_dist = ULTRASONIC_GetDistanceCm(); 

        if (g_front_dist >= OBSTACLE_SAFE_CM || g_front_dist == ULTRASONIC_INVALID_CM || g_front_dist == 0) {
            
            /* ?? 2. ???????? (??????) */
            uint8_t line = LINE_ReadAll();
            int32_t error_sum = 0;
            int8_t active_count = 0;
            
            /* ???:?????,????? */
            if (line & 0x01) { error_sum -= 40; active_count++; } // ??
            if (line & 0x02) { error_sum -= 30; active_count++; }
            if (line & 0x04) { error_sum -= 20; active_count++; }
            if (line & 0x08) { error_sum -= 10; active_count++; } // ??
            if (line & 0x10) { error_sum += 10; active_count++; } // ??
            if (line & 0x20) { error_sum += 20; active_count++; }
            if (line & 0x40) { error_sum += 30; active_count++; }
            if (line & 0x80) { error_sum += 40; active_count++; } // ??
            
            int16_t error = 0;
            static int16_t last_error = 0;
            
            if (active_count > 0) {
                /* ?????????? */
                error = error_sum / active_count;
                last_error = error; // ?????????
            } else {
                /* ?? ?????????:?? 8 ??????,??????????????! */
                error = (last_error > 0) ? 60 : -60; 
            }
            
            /* ???????? (Kp = 0.4) */
            int16_t diff = (error * LINE_KP) / 10; 
            
            /* ??????? (??????,????????????,?????!) */
            SPEED_CTRL_SetSideTargetPulse(LINE_BASE_SPEED + diff, LINE_BASE_SPEED - diff);

        } else {
            /* ?????:????,????????? */
            g_state = STATE_SCAN;
            SPEED_CTRL_SetSideTargetPulse(0, 0); 
            delay_ms(50); 

            SERVO_SetAngle(1, SERVO_LEFT_ANGLE);
            delay_ms(150); 
            g_left_dist = ULTRASONIC_GetDistanceFilteredCm(1);

            SERVO_SetAngle(1, SERVO_RIGHT_ANGLE);
            delay_ms(150); 
            g_right_dist = ULTRASONIC_GetDistanceFilteredCm(1);

            SERVO_SetAngle(1, SERVO_CENTER_ANGLE);
            delay_ms(150);

            if (g_left_dist < g_right_dist) {
                g_state = STATE_RIGHT; 
                SPEED_CTRL_SetSideTargetPulse(OBSTACLE_TURN_OUTER_PULSE, OBSTACLE_TURN_INNER_PULSE);
            } else {
                g_state = STATE_LEFT;  
                SPEED_CTRL_SetSideTargetPulse(OBSTACLE_TURN_INNER_PULSE, OBSTACLE_TURN_OUTER_PULSE);
            }

            g_state = STATE_ESCAPE;
            delay_ms(OBSTACLE_TURN_MS); 

            /* ????,??????????? */
            SPEED_CTRL_SetSideTargetPulse(0, 0);
            delay_ms(50);
            g_state = STATE_FORWARD;
        }
    }
}