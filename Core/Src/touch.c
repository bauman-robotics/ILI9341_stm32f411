/**
 * @file touch.c
 * @brief MSP2807 Touchscreen Driver Implementation
 *
 * This file implements the MSP2807 touchscreen driver for ILI9341 display.
 * Provides touch coordinate reading, pressure detection, and interrupt handling.
 */

#include "touch.h"
#include "touch_calibration.h"
#include "spi.h"
#include "gpio.h"
#include "logger.h"
#include "config.h"
#include "ili9341.h"
#include <string.h>

// Static variables
static touch_data_t last_touch_data = {0};
static uint8_t touch_initialized = 0;
static uint32_t interrupt_counter = 0;

// Note: Calibration variables are now defined in touch_calibration.c

// Forward declarations for calibration functions
void TOUCH_HandleCalibrationTouch(uint16_t x_raw, uint16_t y_raw);
void TOUCH_ShowCalibrationMenu(void);
static void TOUCH_HandleMenuTouch(uint16_t x_raw, uint16_t y_raw);

/**
 * @brief Initialize MSP2807 touchscreen
 */
void TOUCH_Init(void) {
    if (touch_initialized) return;

    LOG_SendString("TOUCH: Initializing MSP2807 touchscreen (full)\r\n");

    // Configure CS pin manually (PB13)
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIOB->MODER &= ~GPIO_MODER_MODER13;    // Clear mode
    GPIOB->MODER |= GPIO_MODER_MODER13_0;   // Output mode
    GPIOB->OTYPER &= ~GPIO_PIN_13;          // Push-pull
    GPIOB->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR13; // Low speed
    GPIOB->PUPDR &= ~GPIO_PUPDR_PUPDR13;    // No pull

    // Set CS high (inactive)
    GPIOB->BSRR = GPIO_PIN_13;

    LOG_SendString("TOUCH: CS pin (PB13) configured manually\r\n");

    // Configure IRQ pin with HAL (PB9)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = TOUCH_IRQ_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(TOUCH_IRQ_PORT, &GPIO_InitStruct);
    LOG_SendString("TOUCH: IRQ pin (PB9) configured with HAL\r\n");

    // Enable interrupt
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    LOG_SendString("TOUCH: Interrupt EXTI9_5 enabled\r\n");

    // Initialize SPI2 for touchscreen communication
    LOG_SendString("TOUCH: Initializing SPI2 for MSP2807\r\n");
    __HAL_RCC_SPI2_CLK_ENABLE();

    // Configure SPI2 with working settings
    hspi2.Instance = SPI2;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi2.Init.CRCPolynomial = 10;

    if (HAL_SPI_Init(&hspi2) != HAL_OK) {
        LOG_SendString("TOUCH: ERROR - SPI2 initialization failed\r\n");
    } else {
        LOG_SendString("TOUCH: SPI2 initialized successfully\r\n");
    }

    
    // Test communication with MSP2807
    LOG_SendString("TOUCH: Testing MSP2807 communication\r\n");

    uint8_t tx_buffer[3] = {0x80, 0x00, 0x00};
    uint8_t rx_buffer[3] = {0};

    GPIOB->BSRR = (uint32_t)GPIO_PIN_13 << 16; // CS LOW
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi2, tx_buffer, rx_buffer, 3, 10);
    GPIOB->BSRR = GPIO_PIN_13; // CS HIGH

    if (status == HAL_OK) {
        LOG_Printf("TOUCH: MSP2807 RX data: %02X %02X %02X\r\n",  rx_buffer[0], rx_buffer[1], rx_buffer[2]);
        
        // Convert to ADC value
        uint16_t adc_value = ((rx_buffer[1] & 0x7F) << 5) | (rx_buffer[2] >> 3);
        LOG_Printf("TOUCH: MSP2807 ADC value: %d\r\n", adc_value);
    } else {
        LOG_Printf("TOUCH: SPI communication failed! Status: %d\r\n", status);
    }
    
    touch_initialized = 1;
    LOG_SendString("TOUCH: MSP2807 touchscreen initialized (full)\r\n");

    // Calibration is now started from StartDefaultTask
}

/**
 * @brief Check if touchscreen is currently touched
 * @return 1 if touched, 0 otherwise
 */
uint8_t TOUCH_IsTouched(void) {
    return HAL_GPIO_ReadPin(TOUCH_IRQ_PORT, TOUCH_IRQ_PIN) == GPIO_PIN_RESET;
}

/**
 * @brief Read ADC value from specified channel
 * @param channel ADC channel (0-7)
 * @return 12-bit ADC value
 */
uint16_t TOUCH_ReadADC(uint8_t channel) {
    uint8_t tx_data[3];
    uint8_t rx_data[3] = {0};

    // MSP2807 command format: START | CHANNEL | MODE
    // START = 1, CHANNEL = 0-7, MODE = 0 (12-bit)
    tx_data[0] = 0x80 | ((channel & 0x07) << 4) | 0x00;
    tx_data[1] = 0x00; // Dummy byte
    tx_data[2] = 0x00; // Dummy byte

    // Select chip
    HAL_GPIO_WritePin(TOUCH_CS_PORT, TOUCH_CS_PIN, GPIO_PIN_RESET);

    // Send command and receive data  TOUCH_SPI_TIMEOUT
    HAL_SPI_TransmitReceive(&TOUCH_SPI, tx_data, rx_data, 3, 100);

    // Deselect chip
    HAL_GPIO_WritePin(TOUCH_CS_PORT, TOUCH_CS_PIN, GPIO_PIN_SET);

    // Convert received data to 12-bit value
    uint16_t adc_value = ((rx_data[1] & 0x7F) << 5) | (rx_data[2] >> 3);

    return adc_value;
}

/**
 * @brief Read X coordinate
 * @return X coordinate (0-4095)
 */
uint16_t TOUCH_ReadX(void) {
    return TOUCH_ReadADC(1); // Channel 1 for X (swapped for landscape)
}

/**
 * @brief Read Y coordinate
 * @return Y coordinate (0-4095)
 */
uint16_t TOUCH_ReadY(void) {
    return TOUCH_ReadADC(5); // Channel 5 for Y (swapped for landscape)
}

/**
 * @brief Read touch pressure
 * @return Pressure value (0-4095)
 */
uint16_t TOUCH_ReadPressure(void) {
    return TOUCH_ReadADC(3); // Channel 3 for Z1/Z2 differential
}

/**
 * @brief Read complete touch data
 * @param data Pointer to touch_data_t structure to fill
 * @return 1 if successful, 0 otherwise
 */
uint8_t TOUCH_ReadData(touch_data_t *data) {
    
    if (!data) return 0;
    //LOG_Printf("TOUCH_ReadData__________________________2 "); 
    // Read coordinates
    uint16_t x_raw = TOUCH_ReadX();
    uint16_t y_raw = TOUCH_ReadY();
    uint16_t pressure = TOUCH_ReadPressure();

    LOG_Printf("TOUCH_ReadData_x_raw%d, %d", x_raw,  y_raw); 

    // Apply touchscreen orientation correction (inverted coordinates)
    x_raw = 4095 - x_raw;  // Invert X axis (0-4095 range)
    y_raw = 4095 - y_raw;  // Invert Y axis (0-4095 range)
    LOG_Printf("TOUCH_ReadData_x_raw_conv %d, %d", x_raw,  y_raw); 
    // Convert to display coordinates (calibration would go here)
    // For now, simple scaling to match display resolution
    data->x = (x_raw * TOUCH_MAX_X) / 4096;
    data->y = (y_raw * TOUCH_MAX_Y) / 4096;
    data->pressure = pressure;

    // Determine event type
    static uint8_t last_touched = 0;
    uint8_t currently_touched = (pressure > TOUCH_PRESS_THRESHOLD);

    if (currently_touched && !last_touched) {
        data->event = TOUCH_EVENT_PRESS;
    } else if (!currently_touched && last_touched) {
        data->event = TOUCH_EVENT_RELEASE;
    } else if (currently_touched && last_touched) {
        data->event = TOUCH_EVENT_MOVE;
    } else {
        data->event = TOUCH_EVENT_NONE;
    }

    last_touched = currently_touched;
    data->timestamp = HAL_GetTick();

    return 1;
}

/**
 * @brief Calibrate touchscreen
 * Starts calibration process if enabled in configuration
 */
void TOUCH_Calibrate(void) {
    LOG_SendString("TOUCH: TOUCH_Calibrate() called\r\n");
    #if TOUCHSCREEN_CALIBRATION_ENABLED
    LOG_SendString("TOUCH: Starting calibration automatically\r\n");
    TOUCH_StartCalibration();
    #else
    LOG_SendString("TOUCH: Calibration disabled in configuration\r\n");
    #endif
}

/**
 * @brief Process touch interrupt
 * Called from EXTI interrupt handler
 */
void TOUCH_ProcessInterrupt(void) {
    interrupt_counter++;

    // Toggle LED to indicate interrupt is working
    //HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    // LOG_SendString("TOUCH: TOUCH_ProcessInterrupt() called\r\n"); // Temporarily disabled

    // Touch processing is now handled in TouchTask
    // This function is kept for compatibility but no longer used
}

/**
 * @brief Handle touch input in menu mode
 * @param x_raw Raw X coordinate from touch
 * @param y_raw Raw Y coordinate from touch
 */
static void TOUCH_HandleMenuTouch(uint16_t x_raw, uint16_t y_raw) {
    // Define menu option areas (approximate Y coordinates)
    // 1. Save Results: Y around 40-60
    // 2. Discard Results: Y around 65-85
    // 3. Recalibrate: Y around 90-110

    if (y_raw >= 35 && y_raw <= 60) {
        // Option 1: Save Results
        LOG_SendString("TOUCH: Saving calibration results\r\n");
        ILI9341_FillScreen(ILI9341_BLACK);
        ILI9341_DrawStringLarge(10, 50, "Results Saved!", ILI9341_GREEN, ILI9341_BLACK);
        osDelay(2000);
        calibration_active = 0; // Exit calibration
    }
    else if (y_raw >= 60 && y_raw <= 85) {
        // Option 2: Discard Results
        LOG_SendString("TOUCH: Discarding calibration results\r\n");
        ILI9341_FillScreen(ILI9341_BLACK);
        ILI9341_DrawStringLarge(10, 50, "Results Discarded", ILI9341_RED, ILI9341_BLACK);
        osDelay(2000);
        calibration_active = 0; // Exit calibration
    }
    else if (y_raw >= 85 && y_raw <= 110) {
        // Option 3: Recalibrate
        LOG_SendString("TOUCH: Starting recalibration\r\n");
        TOUCH_StartCalibration();
    }
    else {
        LOG_Printf("TOUCH: Menu touch outside options: X=%d, Y=%d\r\n", x_raw, y_raw);
    }
}

/**
 * @brief GPIO diagnostic test for SPI2 pins
 * Simple test: set PB10 HIGH permanently
 */
void TOUCH_GPIO_Test(void) {
    LOG_SendString("TOUCH: Starting simple GPIO test - PB10 HIGH\r\n");

    // Deinitialize SPI2 to free pins
    HAL_SPI_DeInit(&TOUCH_SPI);

    // Enable GPIOB clock
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Configure PB10 as GPIO output
    GPIOB->MODER &= ~GPIO_MODER_MODER10;    // Clear mode bits
    GPIOB->MODER |= GPIO_MODER_MODER10_0;   // Set as output

    // Set output type to push-pull
    GPIOB->OTYPER &= ~GPIO_PIN_10;

    // Set speed to low
    GPIOB->OSPEEDR &= ~GPIO_OSPEEDER_OSPEEDR10;

    // Try pull-up to see if it affects the level
    GPIOB->PUPDR &= ~GPIO_PUPDR_PUPDR10;    // Clear pull bits
    GPIOB->PUPDR |= GPIO_PUPDR_PUPDR10_0;   // Set pull-up

    LOG_SendString("TOUCH: PB10 configured as output with pull-up\r\n");

    // Set PB10 HIGH permanently
    GPIOB->BSRR = GPIO_PIN_10;
    LOG_SendString("TOUCH: PB10 set to HIGH with pull-up - check logic analyzer!\r\n");

    // Keep it HIGH forever (for testing)
    while(1) {
        // Infinite loop to keep PB10 HIGH
        // This will show if the code reaches this point
        LOG_SendString("TOUCH: PB10 should be HIGH now\r\n");
        osDelay(1000); // Log every second
    }
}

/**
 * @brief Get last touch data
 * @return Pointer to last touch data structure
 */
touch_data_t* TOUCH_GetLastData(void) {
    return &last_touch_data;
}
