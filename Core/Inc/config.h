#ifndef CONFIG_H
#define CONFIG_H

// Configuration defines for different display tasks

// Task 1: Display complete alphabet with two fonts (Font1 and Font1_2x)
#define TASK_ALPHABET_DISPLAY 0

// Task 2: Cyclic scrolling "Hello World!" text in large font
#define TASK_SCROLLING_HELLO 0

// Task 3: Draw QWERTY keyboard layout with borders for touchscreen
#define TASK_QWERTY_KEYBOARD 1

// Keyboard color palettes
#define KEYBOARD_PALETTE_CLASSIC  0  // White borders, cyan background
#define KEYBOARD_PALETTE_DARK     1  // Gray borders, dark blue background
#define KEYBOARD_PALETTE_MODERN   2  // Black borders, gradient colors

// Current keyboard palette (choose one)
#define KEYBOARD_PALETTE KEYBOARD_PALETTE_CLASSIC

#endif /* CONFIG_H */
