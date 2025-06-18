#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// U3
const int MOTOR_LEFT_IN1_PIN  = 15;   // AIN1 của U3 
const int MOTOR_LEFT_IN2_PIN  = 2;  // AIN2 của U3 
const int MOTOR_LEFT_PWM_PIN  = 18;  // PWMA của U3 

const int MOTOR_RIGHT_IN1_PIN = 16;   // BIN1 của U3 
const int MOTOR_RIGHT_IN2_PIN = 17;  // BIN2 của U3 
const int MOTOR_RIGHT_PWM_PIN = 19;  // PWMB của U3 

// U4 - U5
const int DISTANCE_SENSOR_RIGHT_PIN = 32;  // U4 OUT (GP2Y0A02YKOF_1)
const int DISTANCE_SENSOR_FRONT_PIN = 33;  // U5 OUT (GP2Y0A02YKOF_2) 

// U6
const int I2C_SDA_PIN = 21; 
const int I2C_SCL_PIN = 22; 

// MOTOR PWM CONFIGURATION ===
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;
const int PWM_CHANNEL_LEFT = 0;
const int PWM_CHANNEL_RIGHT = 1;

// LCD I2C CONFIGURATION 
const int LCD_COLUMNS = 16;
const int LCD_ROWS = 2;
const uint8_t LCD_I2C_ADDRESS = 0x27; 

enum RobotSystemState { // Đổi tên để tránh trùng với biến `currentState` có thể có ở đâu đó
    STATE_IDLE,
    STATE_WALL_FOLLOWING,
    STATE_OBSTACLE_AVOID_TURN, // Trạng thái chung khi cần rẽ vì vật cản
    STATE_MENU_MAIN,
    STATE_MENU_TUNE_PID,
    STATE_MENU_SETTINGS,
    STATE_EDIT_VALUE
};

extern int g_basePwmSpeed; // Khai báo extern
extern unsigned long g_turnDurationMs; // Khai báo extern

const int BUTTON_MODE_PIN         = 36; // SW1 (Net Switch_1)
const int BUTTON_SELECT_PIN       = 39; // SW2 (Net Switch_2)
const int BUTTON_UP_PIN           = 35; // SW3 (Net Switch_3)
const int BUTTON_DOWN_PIN         = 34; // SW4 (Net Switch_4)


extern const char* const ROBOT_STATE_NAMES[]; // Khai báo extern cho mảng tên trạng thái
extern const int NUM_ROBOT_SYSTEM_STATES;     // Số lượng trạng thái thực tế (không tính UNKNOWN nếu có)

// === WALL FOLLOWING PID CONFIGURATION ===
extern float g_kp; // Các biến toàn cục cho PID, tiền tố g_ để rõ ràng
extern float g_ki;
extern float g_kd;
extern float g_targetDistanceCm; // Setpoint có thể thay đổi

const float SETPOINT_INCREMENT_CM = 1.0f;
const float MIN_TARGET_DISTANCE_CM = 15.0f;
const float MAX_TARGET_DISTANCE_CM = 60.0f;

const int   BASE_PWM_SPEED = 90;
const float PID_OUTPUT_MIN = -70.0f;
const float PID_OUTPUT_MAX = 70.0f;

// === FRONT OBSTACLE AVOIDANCE ===
const float MIN_FRONT_DISTANCE_TO_STOP_CM = 30.0f;
const unsigned long TURN_DURATION_MS = 1400; // Thời gian quay (cần tune)
const int TURN_PWM_SPEED = 100;             // Tốc độ khi quay

// === BUTTON DEBOUNCE ===
const unsigned long DEBOUNCE_DELAY_MS = 50;

// === TIMING ===
const long SERIAL_BAUD_RATE = 115200;
const unsigned long PYTHON_DATA_SEND_INTERVAL_MS = 200;
const unsigned long CONTROL_LOOP_INTERVAL_MS = 20;
const unsigned long LCD_UPDATE_INTERVAL_MS = 250;

#endif // CONFIG_H