#ifndef ILI9341_H
#define ILI9341_H

#include "main.h"
#include "spi.h"

// ILI9341 Commands
#define ILI9341_RESET             0x01
#define ILI9341_SLEEP_OUT         0x11
#define ILI9341_GAMMA             0x26
#define ILI9341_DISPLAY_OFF       0x28
#define ILI9341_DISPLAY_ON        0x29
#define ILI9341_COLUMN_ADDR       0x2A
#define ILI9341_PAGE_ADDR         0x2B
#define ILI9341_GRAM              0x2C
#define ILI9341_MAC               0x36
#define ILI9341_PIXEL_FORMAT      0x3A
#define ILI9341_WDB               0x51
#define ILI9341_WCD               0x53
#define ILI9341_RGB_INTERFACE     0xB0
#define ILI9341_FRC               0xB1
#define ILI9341_BPC               0xB5
#define ILI9341_DFC               0xB6
#define ILI9341_POWER1            0xC0
#define ILI9341_POWER2            0xC1
#define ILI9341_VCOM1             0xC5
#define ILI9341_VCOM2             0xC7
#define ILI9341_POWERA            0xCB
#define ILI9341_POWERB            0xCF
#define ILI9341_PGAMMA            0xE0
#define ILI9341_NGAMMA            0xE1
#define ILI9341_DTCA              0xE8
#define ILI9341_DTCB              0xEA
#define ILI9341_POWER_SEQ         0xED
#define ILI9341_3GAMMA_EN         0xF2
#define ILI9341_INTERFACE         0xF6
#define ILI9341_PRC               0xF7

// Colors
#define ILI9341_BLACK       0x0000
#define ILI9341_BLUE        0x001F
#define ILI9341_RED         0xF800
#define ILI9341_GREEN       0x07E0
#define ILI9341_CYAN        0x07FF
#define ILI9341_MAGENTA     0xF81F
#define ILI9341_YELLOW      0xFFE0
#define ILI9341_WHITE       0xFFFF

// Display dimensions
#define ILI9341_TFTWIDTH    320
#define ILI9341_TFTHEIGHT   240

// Pins
#define TFT_CS_LOW          HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET)
#define TFT_CS_HIGH         HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET)
#define TFT_DC_LOW          HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_RESET)
#define TFT_DC_HIGH         HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET)
#define TFT_RST_LOW         HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET)
#define TFT_RST_HIGH        HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET)

// Function prototypes
void ILI9341_Init(void);
void ILI9341_WriteCommand(uint8_t cmd);
void ILI9341_WriteData(uint8_t data);
void ILI9341_WriteData16(uint16_t data);
void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9341_FillScreen(uint16_t color);
void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9341_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size, const uint8_t *font);
void ILI9341_DrawString(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size, const uint8_t *font);
void ILI9341_DrawStringLarge(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg);
void ILI9341_DrawCharVar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t font_num);
void ILI9341_DrawStringVar(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t font_num);
void ILI9341_SetRotation(uint8_t rotation);

#endif
