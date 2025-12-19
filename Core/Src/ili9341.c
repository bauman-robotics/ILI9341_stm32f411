#include "ili9341.h"
#include "fonts.h"
#include "logger.h"
#include <stdlib.h>
#include "cmsis_os.h"  // ДОБАВЬТЕ ЭТУ СТРОКУ
#include "FreeRTOS.h"  // ДОБАВЬТЕ ЭТУ СТРОКУ
#include "task.h"      // ДОБАВЬТЕ ЭТУ СТРОКУ

// SPI handle
extern SPI_HandleTypeDef hspi1;

// DMA flag
volatile uint8_t dma_transfer_complete = 1;

// DMA callback
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        dma_transfer_complete = 1;
        TFT_CS_HIGH;  // Release CS after DMA transfer
    }
}

static void ILI9341_Delay(uint32_t ms) {
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        osDelay(ms);
    } else {
        HAL_Delay(ms);
    }
}

void ILI9341_WriteCommand(uint8_t cmd) {
    TFT_DC_LOW;
    TFT_CS_LOW;
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    TFT_CS_HIGH;
}

void ILI9341_WriteData(uint8_t data) {
    TFT_DC_HIGH;
    TFT_CS_LOW;
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    TFT_CS_HIGH;
}

void ILI9341_WriteData16(uint16_t data) {
    uint8_t buf[2] = {(data >> 8) & 0xFF, data & 0xFF};
    TFT_DC_HIGH;
    TFT_CS_LOW;
    HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
    TFT_CS_HIGH;
}

void ILI9341_Init(void) {
    LOG_Printf("ILI9341: Starting initialization...\n");

    // Hardware reset
    LOG_Printf("ILI9341: Hardware reset\n");
    TFT_RST_LOW;
    ILI9341_Delay(10);
    TFT_RST_HIGH;
    ILI9341_Delay(10);

    LOG_Printf("ILI9341: Software reset\n");
    ILI9341_WriteCommand(ILI9341_RESET);
    ILI9341_Delay(100);

    ILI9341_WriteCommand(ILI9341_POWERA);
    ILI9341_WriteData(0x39);
    ILI9341_WriteData(0x2C);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x34);
    ILI9341_WriteData(0x02);

    ILI9341_WriteCommand(ILI9341_POWERB);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0xC1);
    ILI9341_WriteData(0x30);

    ILI9341_WriteCommand(ILI9341_DTCA);
    ILI9341_WriteData(0x85);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x78);

    ILI9341_WriteCommand(ILI9341_DTCB);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x00);

    ILI9341_WriteCommand(ILI9341_POWER_SEQ);
    ILI9341_WriteData(0x64);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x12);
    ILI9341_WriteData(0x81);

    ILI9341_WriteCommand(ILI9341_PRC);
    ILI9341_WriteData(0x20);

    ILI9341_WriteCommand(ILI9341_POWER1);
    ILI9341_WriteData(0x23);

    ILI9341_WriteCommand(ILI9341_POWER2);
    ILI9341_WriteData(0x10);

    ILI9341_WriteCommand(ILI9341_VCOM1);
    ILI9341_WriteData(0x3E);
    ILI9341_WriteData(0x28);

    ILI9341_WriteCommand(ILI9341_VCOM2);
    ILI9341_WriteData(0x86);

    ILI9341_WriteCommand(ILI9341_MAC);
    ILI9341_WriteData(0x48);

    ILI9341_WriteCommand(ILI9341_PIXEL_FORMAT);
    ILI9341_WriteData(0x55);

    ILI9341_WriteCommand(ILI9341_FRC);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x18);

    ILI9341_WriteCommand(ILI9341_DFC);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x82);
    ILI9341_WriteData(0x27);

    ILI9341_WriteCommand(ILI9341_3GAMMA_EN);
    ILI9341_WriteData(0x00);

    ILI9341_WriteCommand(ILI9341_GAMMA);
    ILI9341_WriteData(0x01);

    ILI9341_WriteCommand(ILI9341_PGAMMA);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x2B);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x4E);
    ILI9341_WriteData(0xF1);
    ILI9341_WriteData(0x37);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x10);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x09);
    ILI9341_WriteData(0x00);

    ILI9341_WriteCommand(ILI9341_NGAMMA);
    ILI9341_WriteData(0x00);
    ILI9341_WriteData(0x0E);
    ILI9341_WriteData(0x14);
    ILI9341_WriteData(0x03);
    ILI9341_WriteData(0x11);
    ILI9341_WriteData(0x07);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0xC1);
    ILI9341_WriteData(0x48);
    ILI9341_WriteData(0x08);
    ILI9341_WriteData(0x0F);
    ILI9341_WriteData(0x0C);
    ILI9341_WriteData(0x31);
    ILI9341_WriteData(0x36);
    ILI9341_WriteData(0x0F);

    LOG_Printf("ILI9341: Sleep out\n");
    ILI9341_WriteCommand(ILI9341_SLEEP_OUT);
    ILI9341_Delay(120);

    LOG_Printf("ILI9341: Display on\n");
    ILI9341_WriteCommand(ILI9341_DISPLAY_ON);

    LOG_Printf("ILI9341: Initialization complete\n");
}

void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    ILI9341_WriteCommand(ILI9341_COLUMN_ADDR);
    ILI9341_WriteData16(x0);
    ILI9341_WriteData16(x1);

    ILI9341_WriteCommand(ILI9341_PAGE_ADDR);
    ILI9341_WriteData16(y0);
    ILI9341_WriteData16(y1);

    ILI9341_WriteCommand(ILI9341_GRAM);
}

void ILI9341_SetRotation(uint8_t rotation) {
    ILI9341_WriteCommand(ILI9341_MAC);
    ILI9341_WriteData(rotation);
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if ((x >= ILI9341_TFTWIDTH) || (y >= ILI9341_TFTHEIGHT)) return;

    ILI9341_SetAddressWindow(x, y, x+1, y+1);
    ILI9341_WriteData16(color);
}

void ILI9341_FillScreen(uint16_t color) {
    ILI9341_FillRectangle(0, 0, ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT, color);
}

void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if ((x >= ILI9341_TFTWIDTH) || (y >= ILI9341_TFTHEIGHT)) return;

    if ((x + w - 1) >= ILI9341_TFTWIDTH) w = ILI9341_TFTWIDTH - x;
    if ((y + h - 1) >= ILI9341_TFTHEIGHT) h = ILI9341_TFTHEIGHT - y;

    ILI9341_SetAddressWindow(x, y, x+w-1, y+h-1);

    TFT_DC_HIGH;
    TFT_CS_LOW;

    uint32_t total_pixels = (uint32_t)w * (uint32_t)h;

    // Use DMA for medium rectangles (> 1000 but < 5000 pixels to avoid issues with large clears)
    if (total_pixels > 1000 && total_pixels < 5000) {
        // Create buffer for DMA transfer
        uint32_t buffer_size = total_pixels * 2; // 2 bytes per pixel
        uint8_t *dma_buffer = (uint8_t*)malloc(buffer_size);

        if (dma_buffer != NULL) {
            // Fill buffer with color data
            uint8_t color_high = (color >> 8) & 0xFF;
            uint8_t color_low = color & 0xFF;

            for (uint32_t i = 0; i < buffer_size; i += 2) {
                dma_buffer[i] = color_high;
                dma_buffer[i + 1] = color_low;
            }

            // Wait for any previous DMA transfer to complete
            while (!dma_transfer_complete) {
                osDelay(1);
            }

            dma_transfer_complete = 0;

            // Start DMA transfer
            if (HAL_SPI_Transmit_DMA(&hspi1, dma_buffer, buffer_size) == HAL_OK) {
                // Wait for DMA completion
                uint32_t timeout = 1000; // 1 second timeout
                while (!dma_transfer_complete && timeout--) {
                    osDelay(1);
                }

                if (!dma_transfer_complete) {
                    // DMA failed, fallback to regular SPI
                    HAL_SPI_Abort(&hspi1);
                    dma_transfer_complete = 1;

                    for (uint32_t i = 0; i < total_pixels; i++) {
                        uint8_t buf[2] = {color_high, color_low};
                        HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
                    }
                }
            } else {
                // DMA not available, use regular SPI
                dma_transfer_complete = 1;
                for (uint32_t i = 0; i < total_pixels; i++) {
                    uint8_t buf[2] = {color_high, color_low};
                    HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
                }
            }

            free(dma_buffer);
        } else {
            // Memory allocation failed, use regular SPI
            for (uint32_t i = 0; i < total_pixels; i++) {
                uint8_t buf[2] = {(color >> 8) & 0xFF, color & 0xFF};
                HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
            }
        }
    } else {
        // Use regular SPI for small rectangles
        for (uint32_t i = 0; i < total_pixels; i++) {
            uint8_t buf[2] = {(color >> 8) & 0xFF, color & 0xFF};
            HAL_SPI_Transmit(&hspi1, buf, 2, HAL_MAX_DELAY);
        }
    }

    TFT_CS_HIGH;
}

void ILI9341_DrawChar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size, const uint8_t *font) {
    if ((x >= ILI9341_TFTWIDTH) || (y >= ILI9341_TFTHEIGHT) || ((x + 5 * size - 1) < 0) || ((y + 7 * size - 1) < 0))
        return;

    for (int8_t i = 0; i < 5; i++) {
        uint8_t line = font[(c - 32) * 5 + i];
        for (int8_t j = 0; j < 7; j++) {
            if (line & 0x1) {
                if (size == 1)
                    ILI9341_DrawPixel(x + i, y + j, color);
                else
                    ILI9341_FillRectangle(x + i * size, y + j * size, size, size, color);
            } else if (bg != color) {
                if (size == 1)
                    ILI9341_DrawPixel(x + i, y + j, bg);
                else
                    ILI9341_FillRectangle(x + i * size, y + j * size, size, size, bg);
            }
            line >>= 1;
        }
    }
}

void ILI9341_DrawString(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size, const uint8_t *font) {
    while (*str) {
        ILI9341_DrawChar(x, y, *str, color, bg, size, font);
        x += 6 * size;
        str++;
    }
}

void ILI9341_DrawCharVar(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t font_num) {
    uint8_t char_index;
    const uint8_t *font_data;
    const uint16_t (*font_info)[2];
    uint8_t height;

    if (font_num == 0) {  // Font_13
        font_data = Font_13p;
        font_info = Font_13p_info;
        height = 13;
    } else if (font_num == 1) {  // Font_19
        font_data = Font_19p;
        font_info = Font_19p_info;
        height = 19;
    } else {
        return;  // Invalid font
    }

    // Find character index
    // Font arrays start from '!' (ASCII 33) at index 0
    if (c >= 33 && c <= 126) {
        char_index = c - 33;
    } else {
        return;  // Unsupported character
    }

    uint8_t width = font_info[char_index][0];
    uint16_t offset = font_info[char_index][1];

    // For each row of the character
    for (uint8_t row = 0; row < height; row++) {
        uint8_t font_byte = font_data[offset + row];

        // For each column in width
        for (uint8_t col = 0; col < width; col++) {
            // Check if bit is set (MSB first)
            if (font_byte & (0x80 >> col)) {
                ILI9341_DrawPixel(x + col, y + row, color);
            } else if (bg != color) {
                ILI9341_DrawPixel(x + col, y + row, bg);
            }
        }
    }
}

void ILI9341_DrawStringVar(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t font_num) {
    while (*str) {
        ILI9341_DrawCharVar(x, y, *str, color, bg, font_num);
        // For variable width, need to get width from font_info
        uint8_t char_index = (*str >= 33 && *str <= 126) ? *str - 33 : 0;
        uint8_t width = (font_num == 0) ? Font_13p_info[char_index][0] : Font_19p_info[char_index][0];
        x += width + 1;  // +1 for spacing
        str++;
    }
}

void ILI9341_DrawStringLarge(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg) {
    // Draw using Font1 scaled to size 2 (10x14 pixels)
    ILI9341_DrawString(x, y, str, color, bg, 2, Font1);
}
