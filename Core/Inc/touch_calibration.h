/**
 * @file touch_calibration.h
 * @brief Touchscreen calibration module header
 *
 * This file contains the touchscreen calibration interface and configuration.
 */

#ifndef TOUCH_CALIBRATION_H
#define TOUCH_CALIBRATION_H

#include "main.h"
#include <stdint.h>

// =============================================================================
// CONFIGURATION DEFINES
// =============================================================================

/** @brief Delay before starting calibration task after reset (milliseconds) */
#define CALIBRATION_START_DELAY_MS 1000

/** @brief Stack size for calibration task */
#define CALIBRATION_TASK_STACK_SIZE 256

/** @brief Priority for calibration task */
#define CALIBRATION_TASK_PRIORITY osPriorityNormal

// =============================================================================
// FRAMEBUFFER PAGE DEFINITIONS
// =============================================================================

/** @brief Calibration page ID */
#define CALIBRATION_PAGE_ID 0x01

/** @brief Maximum number of calibration points */
#define CALIBRATION_MAX_POINTS 5

// =============================================================================
// CALIBRATION POINT DEFINITIONS
// =============================================================================

/** @brief Calibration points coordinates (display coordinates) */
#define CALIBRATION_POINT_0_X 10
#define CALIBRATION_POINT_0_Y 10

#define CALIBRATION_POINT_1_X 310
#define CALIBRATION_POINT_1_Y 10

#define CALIBRATION_POINT_2_X 310
#define CALIBRATION_POINT_2_Y 230

#define CALIBRATION_POINT_3_X 10
#define CALIBRATION_POINT_3_Y 230

#define CALIBRATION_POINT_4_X 160
#define CALIBRATION_POINT_4_Y 120

// =============================================================================
// STRUCTURES
// =============================================================================

/** @brief Calibration point structure */
typedef struct {
    uint16_t display_x;  /**< Expected display X coordinate */
    uint16_t display_y;  /**< Expected display Y coordinate */
    uint16_t raw_x;      /**< Raw touchscreen X ADC value */
    uint16_t raw_y;      /**< Raw touchscreen Y ADC value */
    uint8_t collected;   /**< Flag indicating if data was collected */
} calibration_point_t;

/** @brief Calibration coefficients structure */
typedef struct {
    float x_offset;
    float x_scale;
    float y_offset;
    float y_scale;
} calibration_coeffs_t;

// =============================================================================
// GLOBAL VARIABLES (extern declarations)
// =============================================================================

extern uint8_t calibration_active;
extern uint8_t calibration_step;
extern calibration_point_t calibration_points[CALIBRATION_MAX_POINTS];
extern calibration_coeffs_t calibration_coeffs;

// =============================================================================
// FUNCTION PROTOTYPES
// =============================================================================

/**
 * @brief Initialize calibration module
 */
void TOUCH_CALIBRATION_Init(void);

/**
 * @brief Start touchscreen calibration process
 */
void TOUCH_StartCalibration(void);

/**
 * @brief Process calibration touch input
 * @param x_raw Raw X coordinate from touch
 * @param y_raw Raw Y coordinate from touch
 */
void TOUCH_HandleCalibrationTouch(uint16_t x_raw, uint16_t y_raw);

/**
 * @brief Show calibration completion menu
 */
void TOUCH_ShowCalibrationMenu(void);



/**
 * @brief Calculate calibration coefficients from collected points
 */
void TOUCH_CalculateCalibrationCoefficients(void);

/**
 * @brief Draw calibration point on screen
 * @param point_index Index of the calibration point to draw
 */
void TOUCH_DrawCalibrationPoint(uint8_t point_index);

#endif /* TOUCH_CALIBRATION_H */
