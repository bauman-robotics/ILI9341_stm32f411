/**
 * @file touch.h
 * @brief MSP2807 Touchscreen Driver Header
 *
 * This file contains the MSP2807 touchscreen driver interface for ILI9341 display.
 * Supports touch coordinate reading and interrupt-based detection.
 */

#ifndef TOUCH_H
#define TOUCH_H

#include "main.h"
#include <stdint.h>

// Touchscreen pin definitions
#define TOUCH_IRQ_PIN     TOUCH_IRQ_Pin      // PB9
#define TOUCH_IRQ_PORT    TOUCH_IRQ_GPIO_Port
#define TOUCH_CS_PIN      SPI2_CS_Pin         // PB13
#define TOUCH_CS_PORT     SPI2_CS_GPIO_Port

// Touchscreen SPI (SPI2)
#define TOUCH_SPI         hspi2
#define TOUCH_SPI_TIMEOUT 1000

// Touchscreen constants
#define TOUCH_MAX_X       320  // Maximum X coordinate (same as display)
#define TOUCH_MAX_Y       240  // Maximum Y coordinate (same as display)
#define TOUCH_PRESS_THRESHOLD 100  // Minimum pressure to detect touch

// Touch event types
typedef enum {
    TOUCH_EVENT_NONE = 0,
    TOUCH_EVENT_PRESS,
    TOUCH_EVENT_RELEASE,
    TOUCH_EVENT_MOVE
} touch_event_t;

// Touch data structure
typedef struct {
    uint16_t x;        // X coordinate (0-320)
    uint16_t y;        // Y coordinate (0-240)
    uint16_t pressure; // Touch pressure (0-4095)
    touch_event_t event; // Touch event type
    uint32_t timestamp;   // Timestamp of the event
} touch_data_t;

// Function prototypes
void TOUCH_Init(void);
uint8_t TOUCH_IsTouched(void);
uint8_t TOUCH_ReadData(touch_data_t *data);
void TOUCH_Calibrate(void);
void TOUCH_StartCalibration(void);
void TOUCH_ProcessInterrupt(void);
void TOUCH_GPIO_Test(void);

// MSP2807 specific functions
uint16_t TOUCH_ReadADC(uint8_t channel);
uint16_t TOUCH_ReadX(void);
uint16_t TOUCH_ReadY(void);
uint16_t TOUCH_ReadPressure(void);

#endif /* TOUCH_H */
