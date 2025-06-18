#include "ButtonMenuHandler.h"
#include "LcdDisplay.h"     
#include "Config.h"       
#include "PID_Controller.h" 
#include <Arduino.h>

RobotSystemState currentUiState = STATE_IDLE;
int mainMenuSelection = 0;
int tuneMenuSelection = 0;
int currentSettingsMenuSelection = 0;

float tempKp = 0.0f;
float tempKi = 0.0f;
float tempKd = 0.0f;
float tempTargetDist = 0.0f;
int tempBaseSpeed = 0;
unsigned long tempTurnDuration = 0;

static bool isInEditMode = false;
static float valueBeingEdited = 0.0f;
static float editStepSize = 0.1f;
static const char* editingParamName = "";
static int originalMenuSelection = 0;
static RobotSystemState menuBeforeEdit = STATE_IDLE;

// Thêm biến để theo dõi trạng thái hiển thị
static RobotSystemState lastDisplayedState = STATE_IDLE;
static int lastMainMenuSelection = -1;
static int lastTuneMenuSelection = -1;
static int lastSettingsMenuSelection = -1;
static float lastEditValue = -999.0f;
static bool forceDisplayUpdate = false;

struct Button {
    const int pin;
    bool prevState;
    unsigned long lastDebounceTime;
};

Button btnMode    = {BUTTON_MODE_PIN, HIGH, 0};
Button btnSelect  = {BUTTON_SELECT_PIN, HIGH, 0};
Button btnUp      = {BUTTON_UP_PIN, HIGH, 0};
Button btnDown    = {BUTTON_DOWN_PIN, HIGH, 0};

const char* mainMenuItems[] = {"Run WallFollow", "Tune PID Vals ", "Settings      ", "Exit to IDLE"}; 
const int numMainMenuItems = sizeof(mainMenuItems) / sizeof(mainMenuItems[0]);

const char* tuneMenuItems[] = {
    "Kp Val  :", "Ki Val  :", "Kd Val  :", "Target D:", "Apply & Run ", "Save & Exit ", "Exit NoSave"
}; 
const int numTuneMenuItems = sizeof(tuneMenuItems) / sizeof(tuneMenuItems[0]);

const char* settingsMenuItems[] = {"Base PWM:", "Turn ms :", "Back      "};
const int numSettingsMenuItems = sizeof(settingsMenuItems) / sizeof(settingsMenuItems[0]);

void setupButtons() {
    pinMode(btnMode.pin, INPUT_PULLUP);
    pinMode(btnSelect.pin, INPUT_PULLUP);
    pinMode(btnUp.pin, INPUT_PULLUP);
    pinMode(btnDown.pin, INPUT_PULLUP);

    tempKp = g_kp;
    tempKi = g_ki;
    tempKd = g_kd;
    tempTargetDist = g_targetDistanceCm;
    tempBaseSpeed = g_basePwmSpeed;
    tempTurnDuration = g_turnDurationMs;

    currentUiState = STATE_IDLE;
    forceDisplayUpdate = true; // Force first display
    Serial.println("Buttons Initialized (ButtonHandler).");
}

bool checkButtonPressed(Button& b) {
    bool currentBtnState = digitalRead(b.pin);
    bool pressed = false;
    if (currentBtnState == LOW && b.prevState == HIGH && (millis() - b.lastDebounceTime > DEBOUNCE_DELAY_MS)) {
        pressed = true;
        b.lastDebounceTime = millis();
    }
    b.prevState = currentBtnState;
    return pressed;
}

void forceDisplayRefresh() {
    forceDisplayUpdate = true;
    lastDisplayedState = STATE_IDLE; // Reset to force update
    lastMainMenuSelection = -1;
    lastTuneMenuSelection = -1;
    lastSettingsMenuSelection = -1;
    lastEditValue = -999.0f;
}

void enterEditMode(
    const char* paramName, 
    float initialValue, 
    float step, 
    int menuSelectionIndex, 
    RobotSystemState originatingMenu
) {
    isInEditMode = true;
    editingParamName = paramName;
    valueBeingEdited = initialValue;
    editStepSize = step;
    originalMenuSelection = menuSelectionIndex;
    menuBeforeEdit = originatingMenu;
    currentUiState = STATE_EDIT_VALUE;
    forceDisplayUpdate = true;
}

void exitEditMode(bool applyChanges, PIDController &pid, float &actualTargetDist) {
    isInEditMode = false;
    RobotSystemState targetMenuAfterEdit = menuBeforeEdit; 

    if (applyChanges) { 
        displayMessage("Saving Value...", "", 500, true);
        if (menuBeforeEdit == STATE_MENU_TUNE_PID) {
            if (originalMenuSelection == 0) tempKp = valueBeingEdited;
            else if (originalMenuSelection == 1) tempKi = valueBeingEdited;
            else if (originalMenuSelection == 2) tempKd = valueBeingEdited;
            else if (originalMenuSelection == 3) tempTargetDist = valueBeingEdited;
            
            if (tuneMenuSelection == 4 || tuneMenuSelection == 5) { 
                g_kp = tempKp; 
                g_ki = tempKi; 
                g_kd = tempKd;
                g_targetDistanceCm = tempTargetDist; 
                
                pid.setTunings(g_kp, g_ki, g_kd);
                pid.setSetpoint(g_targetDistanceCm); 
                displayMessage("PID Params Set", "Target:" + String(g_targetDistanceCm,0) + "cm", 1200, true);
                if (tuneMenuSelection == 4) { 
                } else { 
                    targetMenuAfterEdit = STATE_MENU_MAIN; 
                    mainMenuSelection = 1;
                }
            } else {
                 displayMessage(editingParamName, String(valueBeingEdited, (editStepSize < 0.02f ? 3: (editStepSize < 1.0f ? 2 : 0) )) + " TempSet", 1000, true);
            }
        } else if (menuBeforeEdit == STATE_MENU_SETTINGS) {
            if (originalMenuSelection == 0) tempBaseSpeed = round(valueBeingEdited);
            else if (originalMenuSelection == 1) tempTurnDuration = round(valueBeingEdited);
            
            g_basePwmSpeed = tempBaseSpeed;
            g_turnDurationMs = tempTurnDuration;
            displayMessage(editingParamName, String(valueBeingEdited,0) + " Saved", 1200, true);
        }
    } else { 
        displayMessage("Edit Canceled", "", 1000, true);
        if (menuBeforeEdit == STATE_MENU_TUNE_PID) {
            tempKp = g_kp;
            tempKi = g_ki;
            tempKd = g_kd;
            tempTargetDist = g_targetDistanceCm;
        } else if (menuBeforeEdit == STATE_MENU_SETTINGS) {
            tempBaseSpeed = g_basePwmSpeed;
            tempTurnDuration = g_turnDurationMs;
        }
    }
    currentUiState = targetMenuAfterEdit;
    forceDisplayUpdate = true;
}

void displayMainMenu() {
    // Chỉ cập nhật nếu có thay đổi
    if (lastDisplayedState == STATE_MENU_MAIN && 
        lastMainMenuSelection == mainMenuSelection && 
        !forceDisplayUpdate) {
        return;
    }
    
    lcd.clear();
    int startItem = (mainMenuSelection / LCD_ROWS) * LCD_ROWS;
    for (int i = 0; i < LCD_ROWS; ++i) {
        int itemIndex = startItem + i;
        lcd.setCursor(0, i);
        if (itemIndex < numMainMenuItems) {
            if (itemIndex == mainMenuSelection) 
                lcd.print(">");
            else 
                lcd.print(" ");
            lcd.print(mainMenuItems[itemIndex]);
        } else {
            // Clear remaining lines
            lcd.print("                ");
        }
    }
    
    lastMainMenuSelection = mainMenuSelection;
    lastDisplayedState = STATE_MENU_MAIN;
    forceDisplayUpdate = false;
}

void displayTunePIDMenu() {
    // Chỉ cập nhật nếu có thay đổi
    if (lastDisplayedState == STATE_MENU_TUNE_PID && 
        lastTuneMenuSelection == tuneMenuSelection && 
        !forceDisplayUpdate) {
        return;
    }
    
    lcd.clear();
    char buffer[LCD_COLUMNS + 1];
    float valuesToDisplay[] = {tempKp, tempKi, tempKd, tempTargetDist, 0, 0, 0};

    int startItem = (tuneMenuSelection / LCD_ROWS) * LCD_ROWS;
    for (int i = 0; i < LCD_ROWS; ++i) {
        int itemIndex = startItem + i;
        lcd.setCursor(0, i);
        if (itemIndex < numTuneMenuItems) {
            if (itemIndex == tuneMenuSelection) lcd.print(">");
            else lcd.print(" ");

            if (itemIndex < 4) { // Kp, Ki, Kd, Target
                int decimals = 2;
                if (itemIndex == 0) decimals = 2;      // Kp
                else if (itemIndex == 1) decimals = 3; // Ki
                else if (itemIndex == 2) decimals = 2; // Kd
                else if (itemIndex == 3) decimals = 0; // Target (cm)
                
                String valStr = String(valuesToDisplay[itemIndex], decimals);
                String menuItem = String(tuneMenuItems[itemIndex]);
                snprintf(buffer, sizeof(buffer), "%s %s", menuItem.c_str(), valStr.c_str());
            } else { // Apply, Save, Exit
                snprintf(buffer, sizeof(buffer), "%s", tuneMenuItems[itemIndex]);
            }
            lcd.print(buffer);
        } else {
            lcd.print("                ");
        }
    }
    
    lastTuneMenuSelection = tuneMenuSelection;
    lastDisplayedState = STATE_MENU_TUNE_PID;
    forceDisplayUpdate = false;
}

void displaySettingsMenu() {
    // Chỉ cập nhật nếu có thay đổi
    if (lastDisplayedState == STATE_MENU_SETTINGS && 
        lastSettingsMenuSelection == currentSettingsMenuSelection && 
        !forceDisplayUpdate) {
        return;
    }
    
    lcd.clear();
    char buffer[LCD_COLUMNS + 1];
    long valuesToDisplay[] = {(long)tempBaseSpeed, (long)tempTurnDuration, 0};

    int startItem = (currentSettingsMenuSelection / LCD_ROWS) * LCD_ROWS;
    for (int i = 0; i < LCD_ROWS; ++i) {
        int itemIndex = startItem + i;
        lcd.setCursor(0, i);
        if (itemIndex < numSettingsMenuItems) {
            if (itemIndex == currentSettingsMenuSelection) lcd.print(">");
            else lcd.print(" ");

            if (itemIndex < 2) { // Base PWM, Turn Duration
                snprintf(buffer, sizeof(buffer), "%-10s %ld", settingsMenuItems[itemIndex], valuesToDisplay[itemIndex]);
            } else { // Back
                snprintf(buffer, sizeof(buffer), "%s", settingsMenuItems[itemIndex]);
            }
            lcd.print(buffer);
        } else {
            lcd.print("                ");
        }
    }
    
    lastSettingsMenuSelection = currentSettingsMenuSelection;
    lastDisplayedState = STATE_MENU_SETTINGS;
    forceDisplayUpdate = false;
}

void displayEditValueScreen(const char* paramName, float valToEdit, float step) {
    // Chỉ cập nhật nếu giá trị thay đổi
    if (lastDisplayedState == STATE_EDIT_VALUE && 
        abs(lastEditValue - valToEdit) < 0.0001f && 
        !forceDisplayUpdate) {
        return;
    }
    
    char buffer[LCD_COLUMNS + 1];
    lcd.clear();
    String pName = String(paramName);
    if(pName.endsWith(":")) pName.remove(pName.length()-1);
    
    snprintf(buffer, sizeof(buffer), "Edit: %-10.10s", pName.c_str());
    lcd.setCursor(0,0);
    lcd.print(buffer);
    
    int decimals = 2;
    if (step < 0.01f && step >= 0.001f) decimals = 3; // Cho Ki
    else if (step >= 1.0f) decimals = 0;             // Cho Target, BaseSpeed, TurnDura

    snprintf(buffer, sizeof(buffer), "Val: %s", String(valToEdit, decimals).c_str());
    lcd.setCursor(0,1);
    lcd.print(buffer);
    
    lastEditValue = valToEdit;
    lastDisplayedState = STATE_EDIT_VALUE;
    forceDisplayUpdate = false;
}

void updateLcdMenu(RobotSystemState robotOperationalState) {
    if (currentUiState == STATE_WALL_FOLLOWING ||
        currentUiState == STATE_OBSTACLE_AVOID_TURN) {
        return;
    }
    
    switch (currentUiState) {
        case STATE_IDLE:
            if (lastDisplayedState != STATE_IDLE || forceDisplayUpdate) {
                displayMessage("Robot IDLE", "SEL:Run/MODE:Menu", 0, false);
                lastDisplayedState = STATE_IDLE;
                forceDisplayUpdate = false;
            }
            break;
        case STATE_MENU_MAIN:
            displayMainMenu();
            break;
        case STATE_MENU_TUNE_PID:
            displayTunePIDMenu();
            break;
        case STATE_MENU_SETTINGS:
            displaySettingsMenu();
            break;
        case STATE_EDIT_VALUE:
            displayEditValueScreen(editingParamName, valueBeingEdited, editStepSize);
            break;
        default:
            break;
    }
}

bool processButtonInputs(RobotSystemState &robotOperationalState, PIDController &pid, float &actualTargetDist) {
    bool mainStateChanged = false; 
    
    if (checkButtonPressed(btnMode)) {
        Serial.println("BTN MODE");
        if (isInEditMode) {
            exitEditMode(false, pid, actualTargetDist); 
        } else {
            switch (currentUiState) {
                case STATE_IDLE: 
                case STATE_WALL_FOLLOWING: 
                case STATE_OBSTACLE_AVOID_TURN:
                    if(robotOperationalState != STATE_IDLE) { 
                        robotOperationalState = STATE_IDLE; 
                        mainStateChanged = true;
                        displayMessage("Paused by Mode", "Enter Menu", 1000, true);
                        forceDisplayRefresh(); // Reset display state after message
                    }
                    currentUiState = STATE_MENU_MAIN; 
                    mainMenuSelection = 0;
                    forceDisplayUpdate = true;
                    break;
                case STATE_MENU_MAIN: 
                    currentUiState = STATE_IDLE; 
                    robotOperationalState = STATE_IDLE;
                    mainStateChanged = true;
                    forceDisplayUpdate = true;
                    break;
                case STATE_MENU_TUNE_PID:
                    currentUiState = STATE_MENU_MAIN; 
                    mainMenuSelection = 1;
                    forceDisplayUpdate = true;
                    break;
                case STATE_MENU_SETTINGS:
                    currentUiState = STATE_MENU_MAIN; 
                    mainMenuSelection = 2;
                    forceDisplayUpdate = true;
                    break;
                default: break;
            }
        }
    }

    if (checkButtonPressed(btnSelect)) {
        Serial.println("BTN SELECT");
        if (isInEditMode) {
            exitEditMode(true, pid, actualTargetDist); 
        } else {
            switch (currentUiState) {
                case STATE_IDLE:
                    robotOperationalState = STATE_WALL_FOLLOWING; 
                    currentUiState = STATE_WALL_FOLLOWING;
                    mainStateChanged = true;
                    break;
                case STATE_MENU_MAIN:
                    if (mainMenuSelection == 0) { 
                        robotOperationalState = STATE_WALL_FOLLOWING; 
                        currentUiState = STATE_WALL_FOLLOWING;
                        mainStateChanged = true;
                    } else if (mainMenuSelection == 1) { 
                        currentUiState = STATE_MENU_TUNE_PID; 
                        tuneMenuSelection = 0;
                        tempKp = g_kp; tempKi = g_ki; tempKd = g_kd; tempTargetDist = g_targetDistanceCm;
                        forceDisplayUpdate = true;
                    } else if (mainMenuSelection == 2) { 
                        currentUiState = STATE_MENU_SETTINGS; 
                        currentSettingsMenuSelection = 0;
                        tempBaseSpeed = g_basePwmSpeed; tempTurnDuration = g_turnDurationMs;
                        forceDisplayUpdate = true;
                    } else if (mainMenuSelection == 3) { 
                        currentUiState = STATE_IDLE; 
                        robotOperationalState = STATE_IDLE;
                        mainStateChanged = true;
                        forceDisplayUpdate = true;
                    }
                    break;
                case STATE_MENU_TUNE_PID:
                    if (tuneMenuSelection == 0) 
                        enterEditMode(tuneMenuItems[0], tempKp, 0.01f, 0, STATE_MENU_TUNE_PID);
                    else if (tuneMenuSelection == 1) 
                        enterEditMode(tuneMenuItems[1], tempKi, 0.001f, 1, STATE_MENU_TUNE_PID);
                    else if (tuneMenuSelection == 2) 
                        enterEditMode(tuneMenuItems[2], tempKd, 0.01f, 2, STATE_MENU_TUNE_PID);
                    else if (tuneMenuSelection == 3) 
                        enterEditMode(tuneMenuItems[3], tempTargetDist, SETPOINT_INCREMENT_CM, 3, STATE_MENU_TUNE_PID);
                    else if (tuneMenuSelection == 4) { 
                        g_kp = tempKp; g_ki = tempKi; g_kd = tempKd; g_targetDistanceCm = tempTargetDist;
                        pid.setTunings(g_kp, g_ki, g_kd);
                        pid.setSetpoint(g_targetDistanceCm);
                        robotOperationalState = STATE_WALL_FOLLOWING; 
                        currentUiState = STATE_WALL_FOLLOWING;
                        mainStateChanged = true;
                        displayMessage("Test Run", "New Params", 1000, true);
                        forceDisplayRefresh();
                    } else if (tuneMenuSelection == 5) { // Save & Exit Tune
                        g_kp = tempKp; g_ki = tempKi; g_kd = tempKd; g_targetDistanceCm = tempTargetDist;
                        pid.setTunings(g_kp, g_ki, g_kd); 
                        pid.setSetpoint(g_targetDistanceCm);
                        currentUiState = STATE_MENU_MAIN; mainMenuSelection = 1;
                        displayMessage("Params Saved", "", 1000, true);
                        forceDisplayRefresh();
                    } else if (tuneMenuSelection == 6) { // Exit No Save
                        currentUiState = STATE_MENU_MAIN; mainMenuSelection = 1;
                        forceDisplayUpdate = true;
                    }
                    break;
                case STATE_MENU_SETTINGS:
                    if (currentSettingsMenuSelection == 0) 
                        enterEditMode(settingsMenuItems[0], tempBaseSpeed, 5.0f, 0, STATE_MENU_SETTINGS);
                    else if (currentSettingsMenuSelection == 1) 
                        enterEditMode(settingsMenuItems[1], tempTurnDuration, 100.0f, 1, STATE_MENU_SETTINGS);
                    else if (currentSettingsMenuSelection == 2) { // Back
                        currentUiState = STATE_MENU_MAIN; mainMenuSelection = 2;
                        forceDisplayUpdate = true;
                    }
                    break;
                default: break;
            }
        }
    }

    if (checkButtonPressed(btnUp)) {
        Serial.println("BTN UP");
        if (isInEditMode) {
            valueBeingEdited += editStepSize;
            if (menuBeforeEdit == STATE_MENU_TUNE_PID && originalMenuSelection == 3) {
                valueBeingEdited = constrain(valueBeingEdited, MIN_TARGET_DISTANCE_CM, MAX_TARGET_DISTANCE_CM);
            }
            // Không cần forceDisplayUpdate ở đây, displayEditValueScreen sẽ tự kiểm tra
        } else {
            switch (currentUiState) {
                case STATE_MENU_MAIN: 
                    mainMenuSelection = (mainMenuSelection - 1 + numMainMenuItems) % numMainMenuItems; 
                    break;
                case STATE_MENU_TUNE_PID: 
                    tuneMenuSelection = (tuneMenuSelection - 1 + numTuneMenuItems) % numTuneMenuItems; 
                    break;
                case STATE_MENU_SETTINGS: 
                    currentSettingsMenuSelection = (currentSettingsMenuSelection - 1 + numSettingsMenuItems) % numSettingsMenuItems; 
                    break;
                default: break;
            }
        }
    }

    if (checkButtonPressed(btnDown)) {
        Serial.println("BTN DOWN");
        if (isInEditMode) {
            valueBeingEdited -= editStepSize;
            if (menuBeforeEdit == STATE_MENU_TUNE_PID && originalMenuSelection == 3) {
                valueBeingEdited = constrain(valueBeingEdited, MIN_TARGET_DISTANCE_CM, MAX_TARGET_DISTANCE_CM);
            }
            // Không cần forceDisplayUpdate ở đây, displayEditValueScreen sẽ tự kiểm tra
        } else {
            switch (currentUiState) {
                case STATE_MENU_MAIN: 
                    mainMenuSelection = (mainMenuSelection + 1) % numMainMenuItems; 
                    break;
                case STATE_MENU_TUNE_PID: 
                    tuneMenuSelection = (tuneMenuSelection + 1) % numTuneMenuItems; 
                    break;
                case STATE_MENU_SETTINGS: 
                    currentSettingsMenuSelection = (currentSettingsMenuSelection + 1) % numSettingsMenuItems; 
                    break;
                default: break;
            }
        }
    }

    // Cập nhật hiển thị LCD (chỉ khi cần thiết)
    if (isInEditMode) {
        displayEditValueScreen(editingParamName, valueBeingEdited, editStepSize);
    } else {
        updateLcdMenu(robotOperationalState);
    }
    
    return mainStateChanged;
}