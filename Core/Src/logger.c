#include "logger.h"
#include "usbd_cdc_if.h"
#include <stdarg.h>
#include "cmsis_os.h"  // ДОБАВЬТЕ
#include "FreeRTOS.h"  // ДОБАВЬТЕ
#include "task.h"      // ДОБАВЬТЕ

// Buffer for formatted strings
#define LOG_BUFFER_SIZE 256
static char log_buffer[LOG_BUFFER_SIZE];

void LOG_Init(void) {
    // USB CDC is initialized in main.c via MX_USB_DEVICE_Init()
    // This function is for any additional logger initialization if needed
}

void LOG_SendString(const char *str) {
    if (str == NULL) return;

    uint16_t len = strlen(str);
    if (len == 0) return;

    // Check if we're in interrupt context
    BaseType_t inInterrupt = xPortIsInsideInterrupt();

    // Send via USB CDC with retry
    uint8_t result;
    uint8_t retries = 10;
    do {
        result = CDC_Transmit_FS((uint8_t*)str, len);
        if (result == USBD_BUSY && !inInterrupt) {
            if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
                osDelay(10);
            } else {
                HAL_Delay(10);
            }
        }
        retries--;
    } while (result == USBD_BUSY && retries > 0 && !inInterrupt);

    // Small delay to allow transmission (skip in interrupt)
    if (!inInterrupt) {
        if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            osDelay(5);
        } else {
            HAL_Delay(5);
        }
    }
}

void LOG_Printf(const char *format, ...) {
    if (format == NULL) return;

    va_list args;
    va_start(args, format);

    // Format the string
    vsnprintf(log_buffer, LOG_BUFFER_SIZE, format, args);

    va_end(args);

    // Send the formatted string
    LOG_SendString(log_buffer);

    // Send CRLF for proper line ending
    LOG_SendString("\r\n");
}

void LOG_HexDump(const char *label, const uint8_t *data, uint16_t len) {
    if (label) {
        LOG_Printf("%s:\n", label);
    }

    for (uint16_t i = 0; i < len; i += 16) {
        LOG_Printf("%04X: ", i);

        // Print hex values
        for (uint8_t j = 0; j < 16; j++) {
            if (i + j < len) {
                LOG_Printf("%02X ", data[i + j]);
            } else {
                LOG_Printf("   ");
            }
        }

        // Print ASCII representation
        LOG_Printf(" |");
        for (uint8_t j = 0; j < 16; j++) {
            if (i + j < len) {
                char c = data[i + j];
                if (c >= 32 && c <= 126) {
                    LOG_Printf("%c", c);
                } else {
                    LOG_Printf(".");
                }
            } else {
                LOG_Printf(" ");
            }
        }
        LOG_Printf("|\n");
    }
}
