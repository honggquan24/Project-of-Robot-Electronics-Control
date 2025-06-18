#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "Config.h"         // For RobotSystemState and button pins
#include "PID_Controller.h" // To interact with PID settings

// Các biến này sẽ được định nghĩa trong ButtonHandler.cpp
extern RobotSystemState currentUiState; // Trạng thái UI/menu hiện tại
extern int mainMenuSelection;
extern int tuneMenuSelection;
// ... các biến khác cho menu và giá trị đang edit ...
extern float tempKp, tempKi, tempKd, tempTargetDist; // Giá trị tạm khi tune

void setupButtons();
// Hàm chính xử lý nút, trả về true nếu có sự thay đổi trạng thái robot lớn
bool processButtonInputs(RobotSystemState &robotOperationalState, PIDController &pid, float &actualTargetDist);
void updateLcdMenu(RobotSystemState robotOperationalState); // Cập nhật LCD dựa trên currentUiState

extern int currentSettingsMenuSelection; // Thêm nếu chưa có
extern int tempBaseSpeed;                // Biến tạm cho settings
extern unsigned long tempTurnDuration;     // Biến tạm cho settings


void displaySettingsMenu(); 
#endif // BUTTON_HANDLER_H