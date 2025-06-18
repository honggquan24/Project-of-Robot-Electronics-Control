#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>

class PIDController {
public:
    PIDController(float kp, float ki, float kd, float outMin, float outMax);

    void setTunings(float kp, float ki, float kd);
    void setOutputLimits(float min, float max);
    void setSetpoint(float setpoint);
    float compute(float inputValue);
    void reset(); 

    float getKp() const { return Kp; }
    float getKi() const { return Ki; }
    float getKd() const { return Kd; }
    float getSetpoint() const { return _setpoint; }

private:
    float Kp, Ki, Kd;
    float _setpoint;
    float integralSum;
    float lastInput;
    unsigned long lastTime;
    float outputMin, outputMax;
    bool firstRun;
};

#endif // PID_CONTROLLER_H