#include "MotorControl.h"
#include "Config.h" // Để lấy các PINMOTOR_...
#include <Arduino.h>

void setupMotors() {
    pinMode(MOTOR_LEFT_IN1_PIN, OUTPUT);
    pinMode(MOTOR_LEFT_IN2_PIN, OUTPUT);
    pinMode(MOTOR_LEFT_PWM_PIN, OUTPUT);
    pinMode(MOTOR_RIGHT_IN1_PIN, OUTPUT);
    pinMode(MOTOR_RIGHT_IN2_PIN, OUTPUT);
    pinMode(MOTOR_RIGHT_PWM_PIN, OUTPUT);

    ledcSetup(PWM_CHANNEL_LEFT, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(MOTOR_LEFT_PWM_PIN, PWM_CHANNEL_LEFT);

    ledcSetup(PWM_CHANNEL_RIGHT, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(MOTOR_RIGHT_PWM_PIN, PWM_CHANNEL_RIGHT);

    stopRobotMotors();
}

void controlMotor(int motorIndex, int dir, int pwmValue) {
    pwmValue = constrain(pwmValue, 0, 255);

    int in1_pin, in2_pin, pwm_channel_esp;
    bool forward_is_in1_high; 

    if (motorIndex == 1) { // Left Motor
        in1_pin = MOTOR_LEFT_IN1_PIN;
        in2_pin = MOTOR_LEFT_IN2_PIN;
        pwm_channel_esp = PWM_CHANNEL_LEFT;
        forward_is_in1_high = true; // BẠN CẦN XÁC MINH CHO ĐỘNG CƠ TRÁI
    } else { // Right Motor
        in1_pin = MOTOR_RIGHT_IN1_PIN;
        in2_pin = MOTOR_RIGHT_IN2_PIN;
        pwm_channel_esp = PWM_CHANNEL_RIGHT;
        forward_is_in1_high = false; // BẠN CẦN XÁC MINH CHO ĐỘNG CƠ PHẢI
    }

    switch (dir) {
        case 1: // Forward
            digitalWrite(in1_pin, forward_is_in1_high ? HIGH : LOW);
            digitalWrite(in2_pin, forward_is_in1_high ? LOW : HIGH);
            ledcWrite(pwm_channel_esp, pwmValue);
            break;
        case -1: // Backward
            digitalWrite(in1_pin, forward_is_in1_high ? LOW : HIGH);
            digitalWrite(in2_pin, forward_is_in1_high ? HIGH : LOW);
            ledcWrite(pwm_channel_esp, pwmValue);
            break;
        case 0: // Coast
            digitalWrite(in1_pin, LOW);
            digitalWrite(in2_pin, LOW);
            ledcWrite(pwm_channel_esp, 0);
            break;
        case 2: // Brake
            digitalWrite(in1_pin, HIGH);
            digitalWrite(in2_pin, HIGH);
            ledcWrite(pwm_channel_esp, 255);
            break;
    }
}

void stopRobotMotors() {
    controlMotor(0, 0, 0);
    controlMotor(1, 0, 0);
}