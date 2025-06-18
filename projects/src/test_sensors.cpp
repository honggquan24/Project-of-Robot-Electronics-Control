// src/test_sensors.cpp
#include <Arduino.h>
#include "Config.h"         // Để lấy các PIN cảm biến
#include "DistanceSensor.h" // Module cảm biến của bạn

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(1000);
    Serial.println("Distance Sensor Test Sketch");

    setupDistanceSensors(); // Khởi tạo chân cảm biến

    Serial.println("Di chuyển vật cản trước cảm biến Phải và Trước để xem giá trị.");
    Serial.println("Định dạng: SensorName: ADC_Raw -> Distance_cm");
    Serial.println("LƯU Ý: Hàm adcToDistanceCm_GP2Y0A02YK0F CẦN ĐƯỢC HIỆU CHUẨN CHÍNH XÁC!");
    Serial.println("-----------------------------------------------------");
}

void loop() {
    // Đọc cảm biến bên phải
    // !! CẢNH BÁO XUNG ĐỘT CHÂN !!
    // Nếu DISTANCE_SENSOR_RIGHT_PIN (GPIO5) đang được dùng bởi MOTOR_LEFT_IN1_PIN,
    // giá trị đọc có thể không chính xác khi động cơ đó hoạt động.
    // Trong bài test này, động cơ không chạy, nên có thể đọc được.
    int rawAdcRight = analogRead(DISTANCE_SENSOR_RIGHT_PIN); // Đọc ADC thô
    float distanceRight = readFilteredDistanceRightCm(); // Đọc khoảng cách đã lọc và chuyển đổi

    // Đọc cảm biến phía trước
    int rawAdcFront = analogRead(DISTANCE_SENSOR_FRONT_PIN); // Đọc ADC thô
    float distanceFront = readFilteredDistanceFrontCm(); // Đọc khoảng cách đã lọc và chuyển đổi

    Serial.print("Right Sensor: ");
    Serial.print(rawAdcRight);
    Serial.print(" -> ");
    Serial.print(distanceRight, 1); // In 1 chữ số thập phân
    Serial.print(" cm\t | \t");

    Serial.print("Front Sensor: ");
    Serial.print(rawAdcFront);
    Serial.print(" -> ");
    Serial.print(distanceFront, 1);
    Serial.println(" cm");

    delay(500); // Đọc và in mỗi nửa giây
}