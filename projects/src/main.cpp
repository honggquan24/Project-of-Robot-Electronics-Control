#include <Arduino.h>
#include <Wire.h>
#include "Config.h"      
#include "MotorControl.h"
#include "DistanceSensor.h"
#include "PID_Controller.h"
#include "LcdDisplay.h"
#include "ButtonMenuHandler.h" 

// Global variables
float g_kp = 1.0f;
float g_ki = 0.05f;
float g_kd = 0.1f;
float g_targetDistanceCm = 30.0f;
int g_basePwmSpeed = BASE_PWM_SPEED; 
unsigned long g_turnDurationMs = TURN_DURATION_MS; 

// Motor control optimization variables
float smoothPwmLeft = 0.0f;
float smoothPwmRight = 0.0f;
const float PWM_SMOOTHING_FACTOR = 0.3f; // Giảm từ 1.0 để làm mềm
const int MIN_PWM_THRESHOLD = 50; // PWM tối thiểu để động cơ chạy
const int MAX_PWM_CHANGE_PER_CYCLE = 50; // Giới hạn thay đổi PWM mỗi chu kỳ

// Distance sensor handling
const float MAX_RELIABLE_DISTANCE_CM = 100.0f; // Khoảng cách tối đa tin cậy
const float MIN_RELIABLE_DISTANCE_CM = 5.0f;   // Khoảng cách tối thiểu tin cậy
const float NO_WALL_DISTANCE_THRESHOLD = 80.0f; // Ngưỡng xác định không có tường
const int SEARCH_TURN_PWM = 120; // PWM cho việc tìm tường
unsigned long lastValidWallTime = 0; // Thời gian cuối có tường hợp lệ
const unsigned long MAX_NO_WALL_TIME_MS = 2000; // Thời gian tối đa không có tường trước khi tìm kiếm

// State names
const char* const ROBOT_STATE_NAMES[] = {
    "IDLE", "WALL_FOL", "OBST_AVD", "TURN_L", "TURN_R",
    "MAINMENU", "TUNE_PID", "SETTINGS", "EDIT_VAL", "UNKNOWN"
};
const int NUM_ROBOT_SYSTEM_STATES_PLUS_UNKNOWN = sizeof(ROBOT_STATE_NAMES) / sizeof(ROBOT_STATE_NAMES[0]);
const int NUM_ROBOT_SYSTEM_STATES = NUM_ROBOT_SYSTEM_STATES_PLUS_UNKNOWN - 1;

PIDController wallFollowPID(g_kp, g_ki, g_kd, PID_OUTPUT_MIN, PID_OUTPUT_MAX);

// System state variables
RobotSystemState currentOperationalState = STATE_IDLE;
bool pythonInterfaceControl = false;
unsigned long lastControlLoopTime = 0;
unsigned long lastPythonDataSendTime = 0;
unsigned long lastLcdUpdateByMain = 0;
unsigned long turnStartTime = 0;

// Function declarations
void handleSerialCommands();
void sendDataToPython(RobotSystemState opState, float distR, float distF, float targetR, float errorR,
                      float steerAdjust, int pwmL, int pwmR);
void smoothMotorControl(int targetPwmLeft, int targetPwmRight);
void applyDeadzonCompensation(int &pwmLeft, int &pwmRight);
bool isDistanceValid(float distance);
float sanitizeDistance(float rawDistance);
void handleNoWallSituation(unsigned long currentTime, int &targetPwmLeft, int &targetPwmRight);

void setup() {
    delay(1000);
    Serial.begin(SERIAL_BAUD_RATE);
    Wire.begin();
    delay(3000);

    setupMotors();
    setupDistanceSensors();
    setupLcd();  
    setupButtons();
    
    Serial.println("ESP_READY_FULL_PROJECT_V2");

    // Initialize PID with global values
    wallFollowPID.setSetpoint(g_targetDistanceCm);
    wallFollowPID.setTunings(g_kp, g_ki, g_kd);

    currentOperationalState = STATE_IDLE;
    currentUiState = STATE_IDLE;
    updateLcdMenu(currentOperationalState);

    // Initialize smooth PWM values
    smoothPwmLeft = 0.0f;
    smoothPwmRight = 0.0f;
    lastValidWallTime = 0; // Initialize wall detection time

    Serial.println("System Initialized. Use buttons or Python GUI.");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Process button inputs
    bool robotStateChangedByButtons = processButtonInputs(currentOperationalState, wallFollowPID, g_targetDistanceCm);
    if (robotStateChangedByButtons) {
        if (currentOperationalState == STATE_WALL_FOLLOWING) {
            wallFollowPID.reset(); 
            wallFollowPID.setSetpoint(g_targetDistanceCm);
            displayMessage("Starting Run", "By Button", 1000, true); 
        } else if (currentOperationalState == STATE_IDLE) {
            stopRobotMotors();
            // Reset smooth PWM when stopping
            smoothPwmLeft = 0.0f;
            smoothPwmRight = 0.0f;
            displayMessage("Robot Stopped", "By Button", 1000, true);
        }
    }

    // Main control loop - only run when not in menu states
    if (currentOperationalState != STATE_IDLE &&
        currentUiState != STATE_MENU_MAIN &&
        currentUiState != STATE_MENU_TUNE_PID &&
        currentUiState != STATE_MENU_SETTINGS &&
        currentUiState != STATE_EDIT_VALUE) {

        if (currentTime - lastControlLoopTime >= CONTROL_LOOP_INTERVAL_MS) {
            lastControlLoopTime = currentTime;

            float distanceRight = readFilteredDistanceRightCm();
            float distanceFront = readFilteredDistanceFrontCm();
            
            float pidError = 0;
            float steeringAdjustment = 0;
            int targetPwmLeft = 0;
            int targetPwmRight = 0;
            
            RobotSystemState nextOperationalState = currentOperationalState;

            switch (currentOperationalState) {
                case STATE_WALL_FOLLOWING:
                    if (distanceFront <= MIN_FRONT_DISTANCE_TO_STOP_CM) {
                        Serial.println("MainLoop: Front Obstacle! -> OBSTACLE_AVOID_TURN");
                        
                        // Smooth stop before turning
                        smoothMotorControl(0, 0);
                        delay(100); // Brief pause for stability
                        
                        nextOperationalState = STATE_OBSTACLE_AVOID_TURN;
                        turnStartTime = currentTime;
                    } else {
                        // Sanitize distance reading
                        float sanitizedDistance = sanitizeDistance(distanceRight);
                        
                        // Check if we have a valid wall to follow
                        if (isDistanceValid(sanitizedDistance)) {
                            // We have a valid wall - update timer and follow normally
                            lastValidWallTime = currentTime;
                            
                            // Update PID setpoint and compute
                            wallFollowPID.setSetpoint(g_targetDistanceCm);
                            steeringAdjustment = wallFollowPID.compute(sanitizedDistance);
                            pidError = g_targetDistanceCm - sanitizedDistance;
                            
                            // Calculate target PWM values
                            targetPwmLeft = g_basePwmSpeed - (int)steeringAdjustment;
                            targetPwmRight = g_basePwmSpeed + (int)steeringAdjustment;
                            
                            // Apply constraints
                            targetPwmLeft = constrain(targetPwmLeft, 0, 255);
                            targetPwmRight = constrain(targetPwmRight, 0, 255);
                            
                            Serial.print("Wall following - Dist: "); Serial.print(sanitizedDistance);
                            Serial.print(" Target: "); Serial.print(targetPwmLeft);
                            Serial.print("/"); Serial.println(targetPwmRight);
                        } else {
                            // No valid wall detected - handle no wall situation
                            Serial.print("No wall detected - Raw: "); Serial.print(distanceRight);
                            Serial.print(" Sanitized: "); Serial.println(sanitizedDistance);
                            
                            handleNoWallSituation(currentTime, targetPwmLeft, targetPwmRight);
                        }
                        
                        controlMotor(0, 1, abs(targetPwmLeft));
                        controlMotor(1, 1, abs(targetPwmRight));  
                        // Apply smooth motor control regardless of wall presence
                        // smoothMotorControl(targetPwmLeft, targetPwmRight);
                    }
                    break;

                case STATE_OBSTACLE_AVOID_TURN:
                    if (currentTime - turnStartTime < g_turnDurationMs) {
                        // Smooth turning motion
                        targetPwmLeft = -TURN_PWM_SPEED;
                        targetPwmRight = TURN_PWM_SPEED;
                        
                        // Apply turn with smooth acceleration
                        int actualTurnPwm = map(currentTime - turnStartTime, 0, min(500UL, g_turnDurationMs), 
                                             TURN_PWM_SPEED/2, TURN_PWM_SPEED);
                        
                        controlMotor(0, -1, actualTurnPwm); // Left Backward
                        controlMotor(1, 1, actualTurnPwm);  // Right Forward
                    } else {
                        Serial.println("MainLoop: Auto Turn Complete -> WALL_FOLLOWING");
                        
                        // Smooth transition back to wall following
                        smoothMotorControl(0, 0);
                        delay(200);
                        
                        wallFollowPID.reset();
                        wallFollowPID.setSetpoint(g_targetDistanceCm);
                        nextOperationalState = STATE_WALL_FOLLOWING;
                        currentUiState = STATE_WALL_FOLLOWING;
                        
                        targetPwmLeft = 0; 
                        targetPwmRight = 0;
                    }
                    break;
                
                default:
                    break;
            }
            
            currentOperationalState = nextOperationalState;

            // Update LCD periodically
            if (currentTime - lastLcdUpdateByMain >= LCD_UPDATE_INTERVAL_MS) {
                lastLcdUpdateByMain = currentTime;
                displayRobotStatus(static_cast<int>(currentOperationalState),
                                   distanceRight, distanceFront,
                                   g_targetDistanceCm,
                                   (int)smoothPwmLeft, (int)smoothPwmRight);
            }

            // Send data to Python (uncomment if needed)
            /*
            if (currentTime - lastPythonDataSendTime >= PYTHON_DATA_SEND_INTERVAL_MS) {
                lastPythonDataSendTime = currentTime;
                if (currentOperationalState != STATE_WALL_FOLLOWING) {
                     pidError = g_targetDistanceCm - distanceRight;
                     steeringAdjustment = 0;
                }
                sendDataToPython(currentOperationalState, distanceRight, distanceFront,
                                 g_targetDistanceCm, pidError,
                                 steeringAdjustment, (int)smoothPwmLeft, (int)smoothPwmRight);
            }
            */
        }
    }
}

// Improved smooth motor control function
void smoothMotorControl(int targetPwmLeft, int targetPwmRight) {
    // Apply exponential smoothing
    smoothPwmLeft += PWM_SMOOTHING_FACTOR * (targetPwmLeft - smoothPwmLeft);
    smoothPwmRight += PWM_SMOOTHING_FACTOR * (targetPwmRight - smoothPwmRight);
    
    // Limit PWM change rate to prevent jerky movements
    static float lastPwmLeft = 0;
    static float lastPwmRight = 0;
    
    float pwmLeftChange = smoothPwmLeft - lastPwmLeft;
    float pwmRightChange = smoothPwmRight - lastPwmRight;
    
    // Constrain change rate
    if (abs(pwmLeftChange) > MAX_PWM_CHANGE_PER_CYCLE) {
        smoothPwmLeft = lastPwmLeft + (pwmLeftChange > 0 ? MAX_PWM_CHANGE_PER_CYCLE : -MAX_PWM_CHANGE_PER_CYCLE);
    }
    if (abs(pwmRightChange) > MAX_PWM_CHANGE_PER_CYCLE) {
        smoothPwmRight = lastPwmRight + (pwmRightChange > 0 ? MAX_PWM_CHANGE_PER_CYCLE : -MAX_PWM_CHANGE_PER_CYCLE);
    }
    
    // Convert to integer and apply deadzone compensation
    int finalPwmLeft = (int)smoothPwmLeft;
    int finalPwmRight = (int)smoothPwmRight;
    
    applyDeadzonCompensation(finalPwmLeft, finalPwmRight);
    
    // Apply motor control
    if (finalPwmLeft == 0 && finalPwmRight == 0) {
        stopRobotMotors();
    } else {
        // Determine direction based on PWM sign
        int leftDir = (finalPwmLeft >= 0) ? 1 : -1;
        int rightDir = (finalPwmRight >= 0) ? 1 : -1;
        
        controlMotor(0, leftDir, abs(finalPwmLeft));
        controlMotor(1, rightDir, abs(finalPwmRight));
    }
    
    // Update last values
    lastPwmLeft = smoothPwmLeft;
    lastPwmRight = smoothPwmRight;
}

// Apply deadzone compensation to overcome motor static friction
void applyDeadzonCompensation(int &pwmLeft, int &pwmRight) {
    // Only apply compensation for low PWM values
    if (pwmLeft > 0 && pwmLeft < MIN_PWM_THRESHOLD) {
        pwmLeft = MIN_PWM_THRESHOLD;
    }
    if (pwmRight > 0 && pwmRight < MIN_PWM_THRESHOLD) {
        pwmRight = MIN_PWM_THRESHOLD;
    }
    
    // Handle negative PWM (reverse direction)
    if (pwmLeft < 0 && pwmLeft > -MIN_PWM_THRESHOLD) {
        pwmLeft = -MIN_PWM_THRESHOLD;
    }
    if (pwmRight < 0 && pwmRight > -MIN_PWM_THRESHOLD) {
        pwmRight = -MIN_PWM_THRESHOLD;
    }
}

// Send data to Python GUI
void sendDataToPython(RobotSystemState opState, float distR, float distF, float targetR, float errorR,
                      float steerAdjust, int pwmL, int pwmR) {
    Serial.print("DATA,");
    Serial.print(static_cast<int>(opState)); Serial.print(",");
    Serial.print(distR, 1); Serial.print(",");
    Serial.print(distF, 1); Serial.print(",");
    Serial.print(targetR, 1); Serial.print(",");
    Serial.print(errorR, 2); Serial.print(",");
    Serial.print(steerAdjust, 2); Serial.print(",");
    Serial.print(pwmL); Serial.print(",");
    Serial.print(pwmR); Serial.print(",");
    Serial.print(g_kp, 3); Serial.print(",");
    Serial.print(g_ki, 4); Serial.print(",");
    Serial.print(g_kd, 3); Serial.print(",");
    Serial.println(pythonInterfaceControl ? 1 : 0);
}

// Handle Serial commands from Python GUI
void handleSerialCommands() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        Serial.print("CMD RX: ["); Serial.print(command); Serial.println("]");

        if (command.startsWith("KP:")) {
            g_kp = command.substring(3).toFloat();
            tempKp = g_kp;
            wallFollowPID.setTunings(g_kp, g_ki, g_kd);
            pythonInterfaceControl = true;
            Serial.print("ACK_KP:"); Serial.println(g_kp, 3);
        } else if (command.startsWith("KI:")) {
            g_ki = command.substring(3).toFloat();
            tempKi = g_ki;
            wallFollowPID.setTunings(g_kp, g_ki, g_kd);
            pythonInterfaceControl = true;
            Serial.print("ACK_KI:"); Serial.println(g_ki, 4);
        } else if (command.startsWith("KD:")) {
            g_kd = command.substring(3).toFloat();
            tempKd = g_kd;
            wallFollowPID.setTunings(g_kp, g_ki, g_kd);
            pythonInterfaceControl = true;
            Serial.print("ACK_KD:"); Serial.println(g_kd, 3);
        } else if (command.equalsIgnoreCase("START")) {
            if (currentUiState == STATE_IDLE || currentUiState == STATE_MENU_MAIN || currentUiState == STATE_MENU_TUNE_PID) {
                Serial.println("ACK_START_CMD: Starting via Python.");
                wallFollowPID.reset();
                wallFollowPID.setSetpoint(g_targetDistanceCm);
                
                // Reset smooth PWM for clean start
                smoothPwmLeft = 0.0f;
                smoothPwmRight = 0.0f;
                lastValidWallTime = millis(); // Reset wall detection timer
                
                currentOperationalState = STATE_WALL_FOLLOWING;
                currentUiState = STATE_WALL_FOLLOWING;
                updateLcdMenu(currentOperationalState);
                pythonInterfaceControl = true;
            } else { 
                Serial.println("NACK_START_CMD: Robot not in suitable UI state for PyStart."); 
            }
        } else if (command.equalsIgnoreCase("STOP_ROBOT")) {
            Serial.println("ACK_STOP_CMD: Halting via Python.");
            
            // Smooth stop
            smoothMotorControl(0, 0);
            delay(100);
            stopRobotMotors();
            
            // Reset smooth PWM
            smoothPwmLeft = 0.0f;
            smoothPwmRight = 0.0f;
            
            currentOperationalState = STATE_IDLE;
            currentUiState = STATE_IDLE;
            updateLcdMenu(currentOperationalState);
            pythonInterfaceControl = false;
        } else if (command.startsWith("TARGET:")) {
            float newTarget = command.substring(7).toFloat();
            if (newTarget >= MIN_TARGET_DISTANCE_CM && newTarget <= MAX_TARGET_DISTANCE_CM) {
                g_targetDistanceCm = newTarget;
                tempTargetDist = newTarget;
                wallFollowPID.setSetpoint(g_targetDistanceCm);
                Serial.print("ACK_TARGET:"); Serial.println(g_targetDistanceCm, 1);
                pythonInterfaceControl = true;
            } else { 
                Serial.println("NACK_TARGET: Value out of range.");
            }
        }
    }
}

// Check if distance reading is valid and reliable
bool isDistanceValid(float distance) {
    return (distance >= MIN_RELIABLE_DISTANCE_CM && 
            distance <= MAX_RELIABLE_DISTANCE_CM && 
            distance < NO_WALL_DISTANCE_THRESHOLD);
}

// Sanitize distance reading - handle invalid readings
float sanitizeDistance(float rawDistance) {
    // Handle clearly invalid readings (0, negative, or extremely large)
    if (rawDistance <= 0 || rawDistance > 400.0f) {
        return NO_WALL_DISTANCE_THRESHOLD + 10.0f; // Return a value indicating no wall
    }
    
    // Return the raw distance if it seems reasonable
    return rawDistance;
}

// Handle situation when no wall is detected
void handleNoWallSituation(unsigned long currentTime, int &targetPwmLeft, int &targetPwmRight) {
    unsigned long timeSinceLastWall = currentTime - lastValidWallTime;
    
    if (timeSinceLastWall < MAX_NO_WALL_TIME_MS) {
        // Recently lost wall - continue moving forward while searching
        Serial.println("Recently lost wall - moving forward");
        targetPwmLeft = g_basePwmSpeed;
        targetPwmRight = g_basePwmSpeed;
    } else {
        // No wall for too long - start search pattern (turn right to find wall)
        Serial.println("No wall for too long - searching right");
        targetPwmLeft = SEARCH_TURN_PWM;   // Turn right to search for wall
        targetPwmRight = g_basePwmSpeed/2; // Slower right wheel
        
        // Reset the timer periodically to avoid infinite search
        if (timeSinceLastWall > MAX_NO_WALL_TIME_MS + 3000) {
            lastValidWallTime = currentTime - MAX_NO_WALL_TIME_MS/2;
        }
    }
}