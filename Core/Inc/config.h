#ifndef CONFIG_H
#define CONFIG_H

// Configuration defines for different display tasks

// Task 1: Display complete alphabet with two fonts (Font1 and Font1_2x)
#define TASK_ALPHABET_DISPLAY 0

// Task 2: Cyclic scrolling "Hello World!" text in large font
#define TASK_SCROLLING_HELLO 0

// Task 3: Draw QWERTY keyboard layout with borders for touchscreen
#define TASK_QWERTY_KEYBOARD 0

// Keyboard color palettes
#define KEYBOARD_PALETTE_CLASSIC  0  // White borders, cyan background
#define KEYBOARD_PALETTE_DARK     1  // Gray borders, dark blue background
#define KEYBOARD_PALETTE_MODERN   2  // Black borders, gradient colors

// Current keyboard palette (choose one)
#define KEYBOARD_PALETTE KEYBOARD_PALETTE_CLASSIC

// Keyboard font options
#define KEYBOARD_FONT_SMALL   0  // Font1 size 1 (5x7 pixels)
#define KEYBOARD_FONT_MEDIUM  1  // Font1 size 2 (10x14 pixels)


// Current keyboard font (choose one)
#define KEYBOARD_FONT KEYBOARD_FONT_MEDIUM

// Keyboard letter case options
#define KEYBOARD_CASE_UPPER   0  // Uppercase letters (QWERTY)
#define KEYBOARD_CASE_LOWER   1  // Lowercase letters (qwerty)

// Current keyboard case (choose one)
#define KEYBOARD_CASE KEYBOARD_CASE_LOWER

// Text to display in the input field at the top of the screen (two lines)
#define DISPLAY_TEXT_LINE1 "Hello World!"
#define DISPLAY_TEXT_LINE2 "STM32 ILI9341"

// Debug configuration
#define ENABLE_FRAMEBUFFER_DEBUG 0  // Enable detailed framebuffer coordinates logging

// Live packet task configuration
#define ENABLE_LIVE_PACKET_TASK 0    // Enable live packet output task
#define LIVE_PACKET_START_DELAY_MS 3000  // Delay before starting live packet output (30 seconds)
#define LIVE_PACKET_START_PERIOD_MS 3000

// Touchscreen configuration
#define ENABLE_TOUCHSCREEN 1         // Enable MSP2807 touchscreen support
#define ENABLE_TOUCH_DEBUG 1         // Enable touch coordinates logging
#define ENABLE_TOUCH_INIT_MINIMAL 0  // No TOUCH_Init() - test TouchTask only

/** @brief Enable touchscreen calibration */
#define TOUCHSCREEN_CALIBRATION_ENABLED 1

// Touchscreen calibration configuration is now in touch_calibration.h

#endif /* CONFIG_H */
