#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

void setupMotors();
void controlMotor(int motorIndex, int dir, int pwmValue);
void stopRobotMotors();

#endif // MOTOR_CONTROL_H