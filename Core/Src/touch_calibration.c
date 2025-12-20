/**
 * @file touch_calibration.c
 * @brief Touchscreen calibration module implementation
 *
 * This file implements the touchscreen calibration functionality.
 */

#include "touch_calibration.h"
#include "ili9341.h"
#include "logger.h"
#include <string.h>
#include "config.h"
#include "touch.h"  // For TOUCH_MAX_X and TOUCH_MAX_Y constants

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

// Calibration state variables
uint8_t calibration_active = 0;
uint8_t calibration_step = 0;
calibration_point_t calibration_points[CALIBRATION_MAX_POINTS] = {
    {CALIBRATION_POINT_0_X, CALIBRATION_POINT_0_Y, 0, 0, 0},  // Top-left
    {CALIBRATION_POINT_1_X, CALIBRATION_POINT_1_Y, 0, 0, 0},  // Top-right
    {CALIBRATION_POINT_2_X, CALIBRATION_POINT_2_Y, 0, 0, 0},  // Bottom-right
    {CALIBRATION_POINT_3_X, CALIBRATION_POINT_3_Y, 0, 0, 0},  // Bottom-left
    {CALIBRATION_POINT_4_X, CALIBRATION_POINT_4_Y, 0, 0, 0}   // Center
};
calibration_coeffs_t calibration_coeffs = {0};

// =============================================================================
// STATIC FUNCTIONS
// =============================================================================

/**
 * @brief Draw calibration point on screen
 * @param point_index Index of the calibration point to draw
 */
static void TOUCH_DrawCalibrationPointInternal(uint8_t point_index) {
    if (point_index >= CALIBRATION_MAX_POINTS) return;

    calibration_point_t *point = &calibration_points[point_index];

    // Draw crosshair at calibration point
    uint16_t color = point->collected ? ILI9341_GREEN : ILI9341_RED;

    // Horizontal line
    ILI9341_FillRectangle(point->display_x - 10, point->display_y - 1, 20, 3, color);
    // Vertical line
    ILI9341_FillRectangle(point->display_x - 1, point->display_y - 10, 3, 20, color);

    // Draw point number
    //char num_str[2] = {'1' + point_index, '\0'};
    //ILI9341_DrawStringLarge(point->display_x + 15, point->display_y - 10, num_str, ILI9341_WHITE, ILI9341_BLACK); // не работает у крайней правой точки???
    //LOG_Printf("POINT_NUM =%d, X=%d, Y=%d\r\n", point_index, point->display_x + 15, point->display_y - 10);
}

// =============================================================================
// PUBLIC FUNCTIONS
// =============================================================================

/**
 * @brief Initialize calibration module
 */
void TOUCH_CALIBRATION_Init(void) {
    LOG_SendString("TOUCH_CAL: Calibration module initialized\r\n");
}

/**
 * @brief Start touchscreen calibration process
 */
void TOUCH_StartCalibration(void) {
    LOG_SendString("TOUCH_CAL: TOUCH_StartCalibration() called - RESETTING ALL POINTS!\r\n");
    LOG_Printf("TOUCH_CAL: TOUCHSCREEN_CALIBRATION_ENABLED = %d\r\n", TOUCHSCREEN_CALIBRATION_ENABLED);
    #if TOUCHSCREEN_CALIBRATION_ENABLED
    LOG_SendString("TOUCH_CAL: Starting touchscreen calibration\r\n");

    // Give time for other tasks to finish initialization
    osDelay(CALIBRATION_START_DELAY_MS);
    LOG_SendString("TOUCH_CAL: After delay, testing display\r\n");

    // Test display with a simple rectangle first
    //ILI9341_FillRectangle(0, 0, 50, 50, ILI9341_RED);  // не работает:
    // ILI9341_FillRectangle(100, 10, 20, 20, ILI9341_RED);  // работает
    // LOG_SendString("TOUCH_CAL: Test rectangle drawn\r\n");    
    //osDelay(1000); // Wait 1 second to see the test rectangle

    // Clear screen
    ILI9341_FillScreen(ILI9341_BLACK);
    osDelay(1000); // Wait 1 second for  FillScreen
    LOG_SendString("TOUCH_CAL: Screen cleared\r\n");

    // Display calibration mode title
    ILI9341_DrawStringLarge(50, 50, "Calibration Mode", ILI9341_YELLOW, ILI9341_BLACK);
    //ILI9341_DrawStringLarge(10, 45, "Calibration Mode", ILI9341_YELLOW, ILI9341_BLACK);
    // LOG_SendString("TOUCH_CAL: Title drawn\r\n");

    ILI9341_DrawStringLarge(50, 80, "Touch the points", ILI9341_YELLOW, ILI9341_BLACK);
    // LOG_SendString("TOUCH_CAL: Instructions drawn\r\n");

    // Reset calibration state
    calibration_active = 1;
    calibration_step = 0;

    LOG_Printf("TOUCH_CAL: Set calibration_active=%d, calibration_step=%d\r\n", calibration_active, calibration_step);

    // Mark all points as not collected
    for (uint8_t i = 0; i < CALIBRATION_MAX_POINTS; i++) {
        calibration_points[i].collected = 0;
    }

    // Draw first calibration point
    TOUCH_DrawCalibrationPoint(calibration_step);
    
    LOG_SendString("TOUCH_CAL: First point drawn\r\n");

    LOG_Printf("TOUCH_CAL: Calibration point %d expected at: X=%d, Y=%d\r\n",
               calibration_step + 1,
               calibration_points[calibration_step].display_x,
               calibration_points[calibration_step].display_y);

    LOG_SendString("TOUCH_CAL: Touch the displayed point to begin calibration\r\n");
    #else
    LOG_SendString("TOUCH_CAL: Calibration disabled in configuration\r\n");
    #endif
}

/**
 * @brief Draw calibration point on screen
 * @param point_index Index of the calibration point to draw
 */
void TOUCH_DrawCalibrationPoint(uint8_t point_index) {
    TOUCH_DrawCalibrationPointInternal(point_index);
}

/**
 * @brief Process calibration touch input
 * @param x_raw Raw X coordinate from touch (0-4095)
 * @param y_raw Raw Y coordinate from touch (0-4095)
 */
void TOUCH_HandleCalibrationTouch(uint16_t x_raw, uint16_t y_raw) {
    if (!calibration_active || calibration_step >= CALIBRATION_MAX_POINTS) return;

    calibration_point_t *expected = &calibration_points[calibration_step];

    // Convert raw coordinates to display coordinates for comparison
    // Apply same correction as in TOUCH_ReadData (invert coordinates)
    uint16_t corrected_x = 4095 - x_raw;
    uint16_t corrected_y = 4095 - y_raw;
    uint16_t display_x = (corrected_x * TOUCH_MAX_X) / 4096;
    uint16_t display_y = (corrected_y * TOUCH_MAX_Y) / 4096;

    // Simple validation - accept touch within reasonable distance
    int16_t diff_x = (int16_t)display_x - (int16_t)expected->display_x;
    int16_t diff_y = (int16_t)display_y - (int16_t)expected->display_y;
    int32_t distance_squared = (int32_t)diff_x * diff_x + (int32_t)diff_y * diff_y;
    int16_t max_distance = 200; // Allow reasonable tolerance
    int32_t max_distance_squared = (int32_t)max_distance * max_distance;

    LOG_Printf("TOUCH_CAL: Touch at display X=%d, Y=%d (raw: %d, %d)\r\n", display_x, display_y, x_raw, y_raw);
    LOG_Printf("TOUCH_CAL: Expected point %d at X=%d, Y=%d\r\n", calibration_step + 1, expected->display_x, expected->display_y);
    LOG_Printf("TOUCH_CAL: Distance: %d (max: %d)\r\n", (int16_t)sqrt(distance_squared), max_distance);

    // Check if touch is reasonably close to the expected point
    //if (distance_squared <= max_distance_squared) {
    if (1) {        
        // Store raw coordinates for current point
        expected->raw_x = x_raw;
        expected->raw_y = y_raw;
        expected->collected = 1;

        LOG_Printf("TOUCH_CAL: Point %d collected successfully!\r\n", calibration_step + 1);
        LOG_Printf("TOUCH_CAL: Point %d raw coordinates: X=%d, Y=%d\r\n", calibration_step + 1, x_raw, y_raw);

        // Just mark that we need to show touch feedback - don't draw in interrupt!
        calibration_step++;

        // Wait for user to release finger before showing next point
        LOG_SendString("TOUCH_CAL: Release finger and touch next point...\r\n");
        osDelay(1500);  // 1.5 second delay
    } else {
        LOG_Printf("TOUCH_CAL: Touch too far from expected point\r\n");
    }
}

/**
 * @brief Show calibration completion menu
 */
void TOUCH_ShowCalibrationMenu(void) {
    LOG_SendString("TOUCH_CAL: TOUCH_ShowCalibrationMenu() called - showing completion menu\r\n");

    // Clear screen
    ILI9341_FillScreen(ILI9341_BLACK);

    // Display menu title
    ILI9341_DrawStringLarge(10, 10, "Calibration Complete", ILI9341_GREEN, ILI9341_BLACK);

    // Display menu options
    ILI9341_DrawStringLarge(10, 40, "1. Save Results", ILI9341_WHITE, ILI9341_BLACK);
    ILI9341_DrawStringLarge(10, 65, "2. Discard Results", ILI9341_WHITE, ILI9341_BLACK);
    ILI9341_DrawStringLarge(10, 90, "3. Recalibrate", ILI9341_WHITE, ILI9341_BLACK);

    // Calculate coefficients
    TOUCH_CalculateCalibrationCoefficients();

    LOG_SendString("TOUCH_CAL: Calibration menu displayed\r\n");
    LOG_SendString("TOUCH_CAL: Touch numbers 1-3 to select option\r\n");

    // Reset calibration state for menu handling
    calibration_active = 2; // Menu mode
    calibration_step = 0;
}

/**
 * @brief Handle touch input in menu mode
 * @param x_raw Raw X coordinate from touch
 * @param y_raw Raw Y coordinate from touch
 */
void TOUCH_HandleMenuTouch(uint16_t x_raw, uint16_t y_raw) {
    // Define menu option areas (approximate Y coordinates)
    // 1. Save Results: Y around 40-60
    // 2. Discard Results: Y around 65-85
    // 3. Recalibrate: Y around 90-110

    if (y_raw >= 35 && y_raw <= 60) {
        // Option 1: Save Results
        LOG_SendString("TOUCH_CAL: Saving calibration results\r\n");
        ILI9341_FillScreen(ILI9341_BLACK);
        ILI9341_DrawStringLarge(10, 50, "Results Saved!", ILI9341_GREEN, ILI9341_BLACK);
        osDelay(2000);
        calibration_active = 0; // Exit calibration
    }
    else if (y_raw >= 60 && y_raw <= 85) {
        // Option 2: Discard Results
        LOG_SendString("TOUCH_CAL: Discarding calibration results\r\n");
        ILI9341_FillScreen(ILI9341_BLACK);
        ILI9341_DrawStringLarge(10, 50, "Results Discarded", ILI9341_RED, ILI9341_BLACK);
        osDelay(2000);
        calibration_active = 0; // Exit calibration
    }
    else if (y_raw >= 85 && y_raw <= 110) {
        // Option 3: Recalibrate
        LOG_SendString("TOUCH_CAL: Starting recalibration\r\n");
        TOUCH_StartCalibration();
    }
    else {
        LOG_Printf("TOUCH_CAL: Menu touch outside options: X=%d, Y=%d\r\n", x_raw, y_raw);
    }
}

/**
 * @brief Calculate calibration coefficients from collected points
 */
void TOUCH_CalculateCalibrationCoefficients(void) {
    LOG_SendString("TOUCH_CAL: Raw calibration data collected:\r\n");

    // Log all collected points - simplified to avoid USB buffer issues
    LOG_SendString("TOUCH_CAL: Collected points summary:\r\n");
    uint8_t collected_count = 0;
    for (uint8_t i = 0; i < CALIBRATION_MAX_POINTS; i++) {
        if (calibration_points[i].collected) {
            collected_count++;
        }
    }
    LOG_Printf("TOUCH_CAL: Total collected: %d/5\r\n", collected_count);

    LOG_SendString("TOUCH_CAL: Starting coefficient calculation...\r\n");

    // Use 3-point calibration (top-left, top-right, bottom-left)
    calibration_point_t *tl = &calibration_points[0]; // Top-left
    calibration_point_t *tr = &calibration_points[1]; // Top-right
    calibration_point_t *bl = &calibration_points[3]; // Bottom-left

    if (!tl->collected || !tr->collected || !bl->collected) {
        LOG_SendString("TOUCH_CAL: ERROR - Not enough calibration points collected\r\n");
        return;
    }

    // Log the 3 points used for calculation
    LOG_Printf("TOUCH_CAL: Using points: TL(%d,%d)->(%d,%d), TR(%d,%d)->(%d,%d), BL(%d,%d)->(%d,%d)\r\n",
               tl->display_x, tl->display_y, tl->raw_x, tl->raw_y,
               tr->display_x, tr->display_y, tr->raw_x, tr->raw_y,
               bl->display_x, bl->display_y, bl->raw_x, bl->raw_y);

    // Calculate X coefficients using linear interpolation
    // display_x = (raw_x - x_offset) / x_scale
    // So: x_scale = (display_range) / raw_range
    //     x_offset = raw_x - (display_x / x_scale)

    float x_raw_min = (float)tl->raw_x;  // Left side (high raw values)
    float x_raw_max = (float)tr->raw_x;  // Right side (low raw values)
    float x_display_min = (float)tl->display_x;  // Left side (low display values)
    float x_display_max = (float)tr->display_x;  // Right side (high display values)

    float x_raw_range = x_raw_max - x_raw_min;
    float x_display_range = x_display_max - x_display_min;

    if (x_raw_range != 0) {
        calibration_coeffs.x_scale = x_display_range / x_raw_range;
        // At raw_x = tl->raw_x (left), we want display_x = tl->display_x (left)
        calibration_coeffs.x_offset = (float)tl->display_x - (calibration_coeffs.x_scale * (float)tl->raw_x);
    } else {
        LOG_SendString("TOUCH_CAL: ERROR - Invalid X calibration data\r\n");
        calibration_coeffs.x_scale = 1.0f;
        calibration_coeffs.x_offset = 0.0f;
    }

    // Calculate Y coefficients
    float y_raw_min = (float)tl->raw_y;  // Top side (high raw values)
    float y_raw_max = (float)bl->raw_y;  // Bottom side (low raw values)
    float y_display_min = (float)tl->display_y;  // Top side (low display values)
    float y_display_max = (float)bl->display_y;  // Bottom side (high display values)

    float y_raw_range = y_raw_max - y_raw_min;
    float y_display_range = y_display_max - y_display_min;

    if (y_raw_range != 0) {
        calibration_coeffs.y_scale = y_display_range / y_raw_range;
        calibration_coeffs.y_offset = (float)tl->display_y - (calibration_coeffs.y_scale * (float)tl->raw_y);
    } else {
        LOG_SendString("TOUCH_CAL: ERROR - Invalid Y calibration data\r\n");
        calibration_coeffs.y_scale = 1.0f;
        calibration_coeffs.y_offset = 0.0f;
    }

    // Log coefficients
    LOG_SendString("TOUCH_CAL: Calibration coefficients calculated:\r\n");
    LOG_Printf("TOUCH_CAL: X_offset: %.3f, X_scale: %.6f\r\n", calibration_coeffs.x_offset, calibration_coeffs.x_scale);
    LOG_Printf("TOUCH_CAL: Y_offset: %.3f, Y_scale: %.6f\r\n", calibration_coeffs.y_offset, calibration_coeffs.y_scale);

    // Test the coefficients with the calibration points
    LOG_SendString("TOUCH_CAL: Testing coefficients:\r\n");
    for (uint8_t i = 0; i < CALIBRATION_MAX_POINTS; i++) {
        calibration_point_t *point = &calibration_points[i];
        if (point->collected) {
            float test_x = calibration_coeffs.x_offset + (calibration_coeffs.x_scale * (float)point->raw_x);
            float test_y = calibration_coeffs.y_offset + (calibration_coeffs.y_scale * (float)point->raw_y);
            LOG_Printf("TOUCH_CAL: Point %d: Expected(%d,%d) -> Calculated(%.1f,%.1f)\r\n",
                       i+1, point->display_x, point->display_y, test_x, test_y);
        }
    }

    // Save to config file format (for manual copying)
    LOG_SendString("TOUCH_CAL: Copy these values to config.h:\r\n");
    LOG_Printf("TOUCH_CAL: #define TOUCH_CAL_X_OFFSET %.3f\r\n", calibration_coeffs.x_offset);
    LOG_Printf("TOUCH_CAL: #define TOUCH_CAL_X_SCALE %.6f\r\n", calibration_coeffs.x_scale);
    LOG_Printf("TOUCH_CAL: #define TOUCH_CAL_Y_OFFSET %.3f\r\n", calibration_coeffs.y_offset);
    LOG_Printf("TOUCH_CAL: #define TOUCH_CAL_Y_SCALE %.6f\r\n", calibration_coeffs.y_scale);
}

/**
 * @brief Process calibration UI updates (call from main task loop)
 */
void TOUCH_ProcessCalibrationUI(void) {
    // Static variables for calibration state tracking
    static uint8_t last_calibration_active = 0;
    static uint8_t last_calibration_step = 0;
    static uint8_t touch_feedback_shown = 0;

    // Touch processing is now done in TouchTask

    // Handle calibration UI updates in main task (not in interrupt!)
    if (calibration_active == 1) {  // Calibration mode
        // Check if we need to update the UI after a touch
        if (calibration_step != last_calibration_step) {
            // Clear screen and show touch feedback
            ILI9341_FillScreen(ILI9341_BLACK);

            // Show blue touch point briefly (without delay to avoid blocking main loop)
            if (!touch_feedback_shown && calibration_step > 0) {
                calibration_point_t *last_point = &calibration_points[calibration_step - 1];
                ILI9341_FillRectangle(last_point->display_x - 2, last_point->display_y - 2, 5, 5, ILI9341_BLUE);
                touch_feedback_shown = 1;
                // Removed osDelay to prevent blocking main task
            }

            // Clear screen again and redraw
            ILI9341_FillScreen(ILI9341_BLACK);

            if (calibration_step < CALIBRATION_MAX_POINTS) {
                // Redisplay title and instructions
                ILI9341_DrawStringLarge(10, 10, "Calibration Mode", ILI9341_WHITE, ILI9341_BLACK);
                ILI9341_DrawStringLarge(10, 35, "Touch the points", ILI9341_YELLOW, ILI9341_BLACK);

                // Draw next point
                TOUCH_DrawCalibrationPoint(calibration_step);

                LOG_Printf("TOUCH_CAL: Ready for point %d at X=%d, Y=%d\r\n",
                           calibration_step + 1,
                           calibration_points[calibration_step].display_x,
                           calibration_points[calibration_step].display_y);
            } else {
                // All points collected, show completion menu
                LOG_SendString("TOUCH_CAL: All points collected, calling TOUCH_ShowCalibrationMenu()\r\n");
                TOUCH_ShowCalibrationMenu();
            }

            last_calibration_step = calibration_step;
            touch_feedback_shown = 0;
        }
    }
    else if (calibration_active == 0 && last_calibration_active != 0) {
        // Calibration finished, screen will be cleared by main application
        LOG_SendString("TOUCH_CAL: Calibration finished\r\n");
        last_calibration_active = 0;
    }

    last_calibration_active = calibration_active;
}
