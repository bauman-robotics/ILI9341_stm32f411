#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>

// Function prototypes
void LOG_Init(void);
void LOG_SendString(const char *str);
void LOG_Printf(const char *format, ...);
void LOG_HexDump(const char *label, const uint8_t *data, uint16_t len);

#endif
