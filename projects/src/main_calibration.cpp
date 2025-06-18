// src/main_test_calibration.cpp
// ĐỔI TÊN FILE NÀY THÀNH main.cpp KHI BẠN MUỐN CHẠY CHẾ ĐỘ NÀY
// HOẶC DÙNG src_filter TRONG platformio.ini ĐỂ CHỈ BUILD FILE NÀY
// VÍ DỤ: src_filter = +<main_test_calibration.cpp>

#include <Arduino.h>

// --- Cấu hình cho chế độ hiệu chuẩn ---
// THAY ĐỔI CHÂN NÀY CHO PHÙ HỢP VỚI KẾT NỐI CỦA BẠN
const int CALIB_SENSOR_PIN = 32; // Ví dụ: GPIO32 (ADC1_CH4)

const int NUM_SAMPLES_FOR_CALIB = 30;
const int DELAY_BETWEEN_CALIB_SAMPLES_MS = 3;
const long SERIAL_CALIB_BAUD_RATE = 115200;


// --- Các hàm cho chế độ hiệu chuẩn (trước đây trong calibration_manager.cpp) ---
void setup_calibration_logic() {
    pinMode(CALIB_SENSOR_PIN, INPUT);

    if (CALIB_SENSOR_PIN >= 32 && CALIB_SENSOR_PIN <= 39) { // ADC1
        analogSetPinAttenuation(CALIB_SENSOR_PIN, ADC_11db);
        Serial.print("ADC Attenuation for GPIO");
        Serial.print(CALIB_SENSOR_PIN);
        Serial.println(" set to 11dB (ADC1).");
    } else if (digitalPinToAnalogChannel(CALIB_SENSOR_PIN) != -1) {
        Serial.print("Warning: GPIO");
        Serial.print(CALIB_SENSOR_PIN);
        Serial.println(" is an ADC2 pin or non-ADC1 pin. Attenuation might not be set as expected via Arduino API if it's ADC2. Ensure WiFi is off for ADC2.");
    } else {
        Serial.print("Error: GPIO");
        Serial.print(CALIB_SENSOR_PIN);
        Serial.println(" is not a valid ADC pin!");
        while(true); // Dừng nếu chân không hợp lệ
    }
    // analogReadResolution(12); // Đảm bảo 12 bit

    Serial.println("ESP_CALIB_READY");
    Serial.println("-------------------------------------");
    Serial.println("Commands from Python GUI expected (e.g., CALIB_REQUEST:distance_cm)");
    Serial.println("-------------------------------------");
}

void loop_calibration_logic() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        Serial.print("ESP32 RX: ["); Serial.print(command); Serial.println("]");

        if (command.startsWith("CALIB_REQUEST:")) {
            String dist_str = command.substring(14);
            float known_distance_cm = dist_str.toFloat();

            if (known_distance_cm > 0 && known_distance_cm <= 200) {
                Serial.print("Processing CALIB_REQUEST for distance: ");
                Serial.print(known_distance_cm, 1);
                Serial.println(" cm");

                uint16_t adc_values[NUM_SAMPLES_FOR_CALIB];
                for (int i = 0; i < NUM_SAMPLES_FOR_CALIB; i++) {
                    adc_values[i] = analogRead(CALIB_SENSOR_PIN);
                    delay(DELAY_BETWEEN_CALIB_SAMPLES_MS);
                }

                for (int i = 0; i < NUM_SAMPLES_FOR_CALIB - 1; i++) {
                    for (int j = i + 1; j < NUM_SAMPLES_FOR_CALIB; j++) {
                        if (adc_values[j] < adc_values[i]) {
                            uint16_t temp = adc_values[i];
                            adc_values[i] = adc_values[j];
                            adc_values[j] = temp;
                        }
                    }
                }
                uint16_t median_adc_value = adc_values[NUM_SAMPLES_FOR_CALIB / 2];
                Serial.print("Median ADC value: "); Serial.println(median_adc_value);

                Serial.print("CALIB_DATA:");
                Serial.print(known_distance_cm, 1);
                Serial.print(",");
                Serial.println(median_adc_value);
            } else {
                Serial.println("CALIB_ERROR:Invalid or out-of-range distance received.");
            }
        } else {
            Serial.print("ESP32 RX: Unknown command prefix - "); Serial.println(command);
        }
    }
}

// --- Hàm setup() và loop() chính cho file test này ---
void setup() {
    Serial.begin(SERIAL_CALIB_BAUD_RATE); // Sử dụng hằng số baud rate riêng
    unsigned long startTime = millis();
    while (!Serial && (millis() - startTime < 5000));

    Serial.println("\n\n--- ESP32 Sensor Calibration Test ---");
    setup_calibration_logic(); // Gọi hàm setup logic hiệu chuẩn
}

void loop() {
    loop_calibration_logic(); // Gọi hàm loop logic hiệu chuẩn
}