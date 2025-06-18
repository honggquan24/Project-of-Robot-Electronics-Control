#ifndef DISTANCE_SENSOR_H
#define DISTANCE_SENSOR_H

void setupDistanceSensors();
float readFilteredDistanceRightCm();
float readFilteredDistanceFrontCm();

// Hàm riêng cho cảm biến phải
float adcToDistanceCm_RightSensor(int adcValue, const char* sensorNameForDebug = "Right");
// Hàm riêng cho cảm biến trước
float adcToDistanceCm_FrontSensor(int adcValue, const char* sensorNameForDebug = "Front");

#endif // DISTANCE_SENSOR_H