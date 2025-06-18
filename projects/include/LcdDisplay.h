#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Config.h"

extern LiquidCrystal_I2C lcd;

void setupLcd();
void displayMessage(const String& line1, const String& line2 = "", unsigned int clearDelayMs = 0, bool clearScreen = true);
void displayRobotStatus(int currentStateCode, float distR, float distF,
                        float targetDist, int pwmL, int pwmR); // Đã đổi steerAdjust thành targetDist

#endif // LCD_DISPLAY_H