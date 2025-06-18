// src/DistanceSensor.cpp

#include "DistanceSensor.h"
#include "Config.h"         
#include <Arduino.h>       
#include <algorithm> // Cho std::sort
#include <cmath>     // Cho các hàm toán học nếu cần trong tương lai

// --- Cấu hình bộ lọc EMA ---
const float EMA_ALPHA_RIGHT = 0.2; // Hệ số alpha cho cảm biến phải (0 < alpha <= 1)
const float EMA_ALPHA_FRONT = 0.2; // Hệ số alpha cho cảm biến trước

// Biến lưu trữ giá trị đã lọc EMA trước đó cho mỗi cảm biến
static float emaFilteredDistanceRightPrev = -1.0; 
static float emaFilteredDistanceFrontPrev = -1.0;

// Kích thước cửa sổ cho bộ lọc Median
const int MEDIAN_WINDOW_SIZE = 7; 
int readAdcWithMedianFilter(int pin);

void setupDistanceSensors() {
    pinMode(DISTANCE_SENSOR_RIGHT_PIN, INPUT);
    pinMode(DISTANCE_SENSOR_FRONT_PIN, INPUT);

    // Giả định các chân này là ADC1 hoặc ADC2 đã được cấu hình đúng suy giảm
    // Nếu bạn chắc chắn chúng là ADC1, bạn có thể thêm analogSetPinAttenuation ở đây.
    // Tuy nhiên, nếu chúng là ADC2, việc cấu hình suy giảm phức tạp hơn và cần cẩn thận với WiFi.
    // Tạm thời giả định rằng suy giảm đã được xử lý ở mức độ nào đó.
    // Để an toàn, bạn có thể thêm:
    if (DISTANCE_SENSOR_RIGHT_PIN >= 32 && DISTANCE_SENSOR_RIGHT_PIN <= 39) { // ADC1
         analogSetPinAttenuation(DISTANCE_SENSOR_RIGHT_PIN, ADC_11db);
    }
    if (DISTANCE_SENSOR_FRONT_PIN >= 32 && DISTANCE_SENSOR_FRONT_PIN <= 39) { // ADC1
         analogSetPinAttenuation(DISTANCE_SENSOR_FRONT_PIN, ADC_11db);
    } else if (DISTANCE_SENSOR_FRONT_PIN == 2 || DISTANCE_SENSOR_FRONT_PIN == 0 || DISTANCE_SENSOR_FRONT_PIN == 4 ||
               DISTANCE_SENSOR_FRONT_PIN == 12 || DISTANCE_SENSOR_FRONT_PIN == 13 || DISTANCE_SENSOR_FRONT_PIN == 14 ||
               DISTANCE_SENSOR_FRONT_PIN == 15 || DISTANCE_SENSOR_FRONT_PIN == 25 || DISTANCE_SENSOR_FRONT_PIN == 26 ||
               DISTANCE_SENSOR_FRONT_PIN == 27 ) { // ADC2
        // Nếu dùng ADC2, hãy chắc chắn bạn đã cấu hình đúng hoặc chấp nhận mặc định.
        // Việc gọi analogSetPinAttenuation cho ADC2 có thể không hoạt động như mong đợi mà không có cấu hình ESP-IDF.
        // Nếu không có WiFi, ADC2 thường hoạt động với Vref mặc định.
        // Nếu có WiFi, không nên dùng ADC2.
    }

    Serial.println("Distance sensors initialized (Calibrated + Median + EMA).");

    // Khởi tạo giá trị lọc EMA ban đầu
    // Đọc giá trị đã qua Median và hiệu chuẩn để khởi tạo
    float initialRightDist = 0;
    float initialFrontDist = 0;
    int initSamples = 3; // Chỉ cần vài mẫu sau khi đã có median
    for(int i=0; i<initSamples; ++i) {
        int medianAdcR = readAdcWithMedianFilter(DISTANCE_SENSOR_RIGHT_PIN);
        initialRightDist += adcToDistanceCm_RightSensor(medianAdcR, nullptr); // không cần debug name ở đây

        int medianAdcF = readAdcWithMedianFilter(DISTANCE_SENSOR_FRONT_PIN);
        initialFrontDist += adcToDistanceCm_FrontSensor(medianAdcF, nullptr);
        delay(10); // Trễ giữa các lần khởi tạo
    }
    emaFilteredDistanceRightPrev = initialRightDist / initSamples;
    emaFilteredDistanceFrontPrev = initialFrontDist / initSamples;

    Serial.print("Initial EMA Right Dist: "); Serial.println(emaFilteredDistanceRightPrev, 1);
    Serial.print("Initial EMA Front Dist: "); Serial.println(emaFilteredDistanceFrontPrev, 1);
}

// Hàm đọc ADC thô với bộ lọc Median
int readAdcWithMedianFilter(int pin) {
    uint16_t readings[MEDIAN_WINDOW_SIZE]; 
    for (int i = 0; i < MEDIAN_WINDOW_SIZE; i++) {
        readings[i] = analogRead(pin);
        delay(1); // Trễ nhỏ giữa các lần đọc ADC
    }
    std::sort(readings, readings + MEDIAN_WINDOW_SIZE);
    return readings[MEDIAN_WINDOW_SIZE / 2];
}

// Hàm hiệu chuẩn cho cảm biến Phải
float adcToDistanceCm_RightSensor(int adcValue, const char* sensorNameForDebug) {
    // Bỏ qua Serial.print ở đây để hàm này chỉ tập trung vào tính toán
    // Bạn có thể gọi Serial.print bên ngoài nếu cần debug giá trị ADC cụ thể
    // if (sensorNameForDebug && strlen(sensorNameForDebug) > 0) {
    //    Serial.print(sensorNameForDebug); Serial.print(" Calib ADC: "); Serial.println(adcValue);
    // }

    float distance_cm;
    if (adcValue > 1) { // Tránh chia cho 0 hoặc giá trị ADC không hợp lệ
        distance_cm = 29586.6441f / (float)adcValue - 0.1f; // Công thức hiệu chuẩn của bạn
    } else {
        distance_cm = 150.0f; // Giá trị mặc định nếu ADC không hợp lệ
    }
    return constrain(distance_cm, 20.0f, 150.0f); // Giới hạn trong dải đo của cảm biến
}

// Hàm hiệu chuẩn cho cảm biến Trước
float adcToDistanceCm_FrontSensor(int adcValue, const char* sensorNameForDebug) {
    // if (sensorNameForDebug && strlen(sensorNameForDebug) > 0) {
    //    Serial.print(sensorNameForDebug); Serial.print(" Calib ADC: "); Serial.println(adcValue);
    // }

    float distance_cm;
    if (adcValue > 1) { 
        distance_cm = 30538.31f / (float)adcValue + 0.05f; // Công thức hiệu chuẩn của bạn
    } else {
        distance_cm = 150.0f;
    }
    return constrain(distance_cm, 20.0f, 150.0f);
}


// Hàm đọc khoảng cách đã được lọc (Median -> Hiệu chuẩn -> EMA)
float readFilteredDistanceRightCm() {
    // Bước 1: Đọc ADC với bộ lọc Median
    int medianAdc = readAdcWithMedianFilter(DISTANCE_SENSOR_RIGHT_PIN);
    
    // Bước 2: Chuyển đổi ADC đã lọc Median sang khoảng cách (cm) bằng hàm hiệu chuẩn
    float calibratedDistanceCm = adcToDistanceCm_RightSensor(medianAdc, nullptr); // Không cần debug name ở đây

    // Bước 3: Áp dụng bộ lọc EMA cho giá trị khoảng cách đã hiệu chuẩn
    // Khởi tạo bộ lọc EMA nếu đây là lần đọc đầu tiên
    if (emaFilteredDistanceRightPrev < 0.0) { 
        emaFilteredDistanceRightPrev = calibratedDistanceCm;
    }

    // Áp dụng công thức EMA
    emaFilteredDistanceRightPrev = EMA_ALPHA_RIGHT * calibratedDistanceCm + (1.0 - EMA_ALPHA_RIGHT) * emaFilteredDistanceRightPrev;
    
    return emaFilteredDistanceRightPrev;
}

float readFilteredDistanceFrontCm() {
    // Bước 1: Đọc ADC với bộ lọc Median
    int medianAdc = readAdcWithMedianFilter(DISTANCE_SENSOR_FRONT_PIN);

    // Bước 2: Chuyển đổi ADC đã lọc Median sang khoảng cách (cm)
    float calibratedDistanceCm = adcToDistanceCm_FrontSensor(medianAdc, nullptr);

    // Bước 3: Áp dụng bộ lọc EMA
    if (emaFilteredDistanceFrontPrev < 0.0) {
        emaFilteredDistanceFrontPrev = calibratedDistanceCm;
    }
    emaFilteredDistanceFrontPrev = EMA_ALPHA_FRONT * calibratedDistanceCm + (1.0 - EMA_ALPHA_FRONT) * emaFilteredDistanceFrontPrev;

    return emaFilteredDistanceFrontPrev;
}