#include "PID_Controller.h"
#include "Arduino.h"
PIDController::PIDController(float kp, float ki, float kd, float outMin, float outMax) {
    outputMin = outMin;
    outputMax = outMax;
    setTunings(kp, ki, kd);
    _setpoint = 0;
    reset();
}

void PIDController::setTunings(float kp, float ki, float kd) {
    if (kp < 0 || ki < 0 || kd < 0) return;
    Kp = kp;
    Ki = ki;
    Kd = kd;
}

void PIDController::setOutputLimits(float min, float max) {
    if (min >= max) return;
    outputMin = min;
    outputMax = max;

    if (Ki != 0) {
        if (integralSum * Ki > outputMax) integralSum = outputMax / Ki;
        else if (integralSum * Ki < outputMin) integralSum = outputMin / Ki;
    }
}

void PIDController::setSetpoint(float setpoint) {
    _setpoint = setpoint;
}

void PIDController::reset() {
    integralSum = 0;
    lastInput = 0; 
    lastTime = millis();
    firstRun = true;
}

float PIDController::compute(float inputValue) {
    unsigned long now = millis();
    unsigned long timeChange = (now - lastTime);

    if (firstRun || timeChange == 0) { 
        lastTime = now;
        lastInput = inputValue; 
        firstRun = false;
        return 0; 
    }

    float error = _setpoint - inputValue;

    error = constrain(error, -20, 20);
 
    float dt_seconds = (float)timeChange / 1000.0f;

    float P_out = Kp * error;

    integralSum += error * dt_seconds;
    float I_term_unclamped = Ki * integralSum; // Giữ lại giá trị chưa clamp của I
    float I_out = I_term_unclamped;

    float dInput = inputValue - lastInput;
    float D_out = Kd * (-dInput / dt_seconds);

    float output = P_out + I_out + D_out;

    // Anti-windup: Nếu output bị clamp, điều chỉnh integralSum tương ứng
    if (output > outputMax) {
        if (Ki != 0 && dt_seconds > 0.0001f) { // Tránh chia cho 0
             // Chỉ trừ đi phần vượt quá do I_out gây ra, nếu P và D không làm nó vượt
            if (I_term_unclamped > (outputMax - (P_out + D_out))) {
                 integralSum -= (I_term_unclamped - (outputMax - (P_out + D_out))) / (Ki * dt_seconds) ;
            }
        }
        output = outputMax;
    } else if (output < outputMin) {
         if (Ki != 0 && dt_seconds > 0.0001f) {
            if (I_term_unclamped < (outputMin - (P_out + D_out))) {
                integralSum -= (I_term_unclamped - (outputMin - (P_out + D_out))) / (Ki * dt_seconds) ;
            }
        }
        output = outputMin;
    }
    
    lastInput = inputValue;
    lastTime = now;

    return output;
}