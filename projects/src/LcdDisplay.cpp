#include "LcdDisplay.h"

LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// Static variables for LCD optimization
static String lastStatusLine0 = "";
static String lastStatusLine1 = "";
static unsigned long lastStatusUpdate = 0;
static const unsigned long STATUS_UPDATE_INTERVAL = 100; // Update every 100ms

void setupLcd() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.print("LCD Init OK");
    delay(1000);
    lcd.clear(); // Clear after initialization message
}

void displayRobotStatus(int currentStateCode, float distR, float distF,
                        float targetDist, int pwmL, int pwmR) {
    // Only update if enough time has passed (reduce flickering)
    unsigned long currentTime = millis();
    if (currentTime - lastStatusUpdate < STATUS_UPDATE_INTERVAL) {
        return;
    }
    lastStatusUpdate = currentTime;
    
    char buffer[LCD_COLUMNS + 1];
    String stateStr = "UNKNOWN"; // Default if code doesn't match
    
    // Use NUM_ROBOT_SYSTEM_STATES from Config.h
    if (currentStateCode >= 0 && currentStateCode < NUM_ROBOT_SYSTEM_STATES) {
        stateStr = ROBOT_STATE_NAMES[currentStateCode];
    }
    
    String line1_str = stateStr;
    while(line1_str.length() < 7) line1_str += " ";
    line1_str = line1_str.substring(0,7);

    // Prepare new content
    snprintf(buffer, sizeof(buffer), "%s T%2.0f F%3.0f", line1_str.c_str(), targetDist, distF);
    String newLine0 = String(buffer);
    
    snprintf(buffer, sizeof(buffer), "R:%3.0f L%3d R%3d", distR, pwmL, pwmR);
    String newLine1 = String(buffer);
    
    // Only update lines that have changed
    if (lastStatusLine0 != newLine0) {
        lcd.setCursor(0, 0);
        // Pad with spaces to clear any remaining characters
        String paddedLine = newLine0;
        while(paddedLine.length() < LCD_COLUMNS) {
            paddedLine += " ";
        }
        lcd.print(paddedLine.substring(0, LCD_COLUMNS));
        lastStatusLine0 = newLine0;
    }
    
    if (lastStatusLine1 != newLine1) {
        lcd.setCursor(0, 1);
        // Pad with spaces to clear any remaining characters
        String paddedLine = newLine1;
        while(paddedLine.length() < LCD_COLUMNS) {
            paddedLine += " ";
        }
        lcd.print(paddedLine.substring(0, LCD_COLUMNS));
        lastStatusLine1 = newLine1;
    }
}

void displayMessage(const String& line1, const String& line2, unsigned int clearDelayMs, bool clearScreen) {
    if (clearScreen) {
        lcd.clear();
        // Reset cached values since we cleared the screen
        lastStatusLine0 = "";
        lastStatusLine1 = "";
    }
    
    lcd.setCursor(0, 0); 
    String paddedLine1 = line1.substring(0, LCD_COLUMNS);
    while(paddedLine1.length() < LCD_COLUMNS) {
        paddedLine1 += " ";
    }
    lcd.print(paddedLine1);
    
    if (line2.length() > 0) {
        lcd.setCursor(0, 1);
        String paddedLine2 = line2.substring(0, LCD_COLUMNS);
        while(paddedLine2.length() < LCD_COLUMNS) {
            paddedLine2 += " ";
        }
        lcd.print(paddedLine2);
    } else if (clearScreen) {
        // Clear second line if no content provided
        lcd.setCursor(0, 1);
        String emptyLine = "";
        for(int i = 0; i < LCD_COLUMNS; i++) {
            emptyLine += " ";
        }
        lcd.print(emptyLine);
    }
    
    if (clearDelayMs > 0) {
        delay(clearDelayMs);
        if(clearScreen) {
            lcd.clear();
            // Reset cached values after clearing
            lastStatusLine0 = "";
            lastStatusLine1 = "";
        }
    }
}