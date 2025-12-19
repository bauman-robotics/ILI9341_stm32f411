/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gpio.h"
#include "ili9341.h"
#include "fonts.h"
#include "logger.h"
#include "config.h"

// DMA transfer flag from ili9341.c
extern volatile uint8_t dma_transfer_complete;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */

  LOG_Init();
  HAL_Delay(1000); // Give USB CDC time to initialize
  LOG_Printf("System: Starting application\n");

  // Turn on backlight
  LOG_Printf("GPIO: Turning on backlight\n");
  HAL_GPIO_WritePin(BACK_LIGHT_GPIO_Port, BACK_LIGHT_Pin, GPIO_PIN_SET);

  // Initialize display
  ILI9341_Init();

  // Set rotation to landscape mode (working orientation)
  LOG_Printf("Setting display to landscape mode");
  ILI9341_SetRotation(0x28);  // Landscape mode

  // Small delay after rotation
  HAL_Delay(100);

  // Fill screen with black
  ILI9341_FillScreen(ILI9341_BLACK);

  // Additional delay to ensure display is ready
  HAL_Delay(200);

  // Debug: Check which task is active
  LOG_Printf("Config check active");

#if TASK_ALPHABET_DISPLAY == 1
  // Task 1: Display complete alphabet with two fonts
  LOG_Printf("Task: Alphabet Display");
  LOG_Printf("Font1: 5x7 pixels, Font1_2x: 10x14 pixels");

  // Font1 (5x7) - Uppercase A-Z
  const char *upper_az = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  ILI9341_DrawString(10, 10, upper_az, ILI9341_WHITE, ILI9341_BLACK, 1, Font1);

  // Font1 (5x7) - Lowercase a-z
  const char *lower_az = "abcdefghijklmnopqrstuvwxyz";
  ILI9341_DrawString(10, 25, lower_az, ILI9341_CYAN, ILI9341_BLACK, 1, Font1);

  // Font1_2x (10x14) - Uppercase A-Z
  ILI9341_DrawStringLarge(10, 45, upper_az, ILI9341_YELLOW, ILI9341_BLACK);

  // Font1_2x (10x14) - Lowercase a-z
  ILI9341_DrawStringLarge(10, 75, lower_az, ILI9341_RED, ILI9341_BLACK);

#elif TASK_SCROLLING_HELLO == 1
  // Task 2: Cyclic scrolling "Hello World!" text in large font
  LOG_Printf("Task: Scrolling Hello World!");
  LOG_Printf("Cyclic scrolling text with Font1_2x");

  static int scroll_pos = 320; // Start from right edge
  const char *hello_text = "Hello World! ";
  const int text_width = 12 * 13; // Approximate width of text in large font

  int prev_scroll_pos = scroll_pos;

  LOG_Printf("Starting scroll loop with optimized rendering...");

  // Double buffering for flicker-free scrolling
  // Buffer for the text line area: 320 pixels wide x 30 pixels high
  #define TEXT_BUFFER_WIDTH 320
  #define TEXT_BUFFER_HEIGHT 30
  #define TEXT_BUFFER_SIZE (TEXT_BUFFER_WIDTH * TEXT_BUFFER_HEIGHT)

  static uint16_t text_buffer[TEXT_BUFFER_SIZE]; // 320*30*2 = 19,200 bytes

  while (1) {
    // Clear the software buffer (fast memory operation)
    for (int i = 0; i < TEXT_BUFFER_SIZE; i++) {
      text_buffer[i] = ILI9341_BLACK;
    }

    // Draw text into the software buffer (no SPI, pure memory)
    // Simulate drawing at the current scroll position
    int buffer_x = scroll_pos;
    const char *text_ptr = hello_text;

    while (*text_ptr && buffer_x < TEXT_BUFFER_WIDTH) {
      // Draw character into buffer using actual font data
      if (*text_ptr >= 32 && *text_ptr < 127) {
        int char_index = *text_ptr - 32;

        // Render each column of the character
        for (int col = 0; col < 5 && buffer_x + col * 2 < TEXT_BUFFER_WIDTH; col++) {
          uint8_t line = Font1[char_index * 5 + col];

          // Render each row of the character (scaled 2x)
          for (int row = 0; row < 7; row++) {
            if (line & 0x1) {  // Pixel is set in font
              // Scale 2x: draw 2x2 pixel block
              int pixel_y = 5 + row * 2;  // Center vertically, scale by 2

              for (int sy = 0; sy < 2 && pixel_y + sy < TEXT_BUFFER_HEIGHT; sy++) {
                for (int sx = 0; sx < 2 && buffer_x + col * 2 + sx < TEXT_BUFFER_WIDTH; sx++) {
                  int pixel_x = buffer_x + col * 2 + sx;
                  text_buffer[(pixel_y + sy) * TEXT_BUFFER_WIDTH + pixel_x] = ILI9341_GREEN;
                }
              }
            }
            line >>= 1;  // Next row bit
          }
        }
        buffer_x += 13; // Character width (10px) + spacing (3px)
      }
      text_ptr++;
    }

    // Transfer the entire buffer to display via DMA (single operation)
    ILI9341_SetAddressWindow(0, 105, TEXT_BUFFER_WIDTH - 1, 105 + TEXT_BUFFER_HEIGHT - 1);

    // Use DMA for the buffer transfer (19,200 bytes)
    TFT_DC_HIGH;
    TFT_CS_LOW;

    // For now, use regular SPI for testing (more reliable)
    HAL_SPI_Transmit(&hspi1, (uint8_t*)text_buffer, TEXT_BUFFER_SIZE * 2, HAL_MAX_DELAY);

    TFT_CS_HIGH;

    // Update positions
    prev_scroll_pos = scroll_pos;
    scroll_pos -= 1; // Move left by 1 pixel for smoother animation

    // Reset position when text goes off screen
    if (scroll_pos < -text_width) {
      scroll_pos = 320;
      prev_scroll_pos = scroll_pos;
    }

    // Optimized delay for very smooth scrolling
    osDelay(20); // 50 FPS for ultra smooth animation

    // Yield to other tasks to prevent starvation
    taskYIELD();
  }

#elif TASK_QWERTY_KEYBOARD == 1
  // Task 3: Draw QWERTY keyboard layout with borders for touchscreen
  LOG_Printf("Task: QWERTY Keyboard Layout");
  LOG_Printf("Drawing keyboard with borders for touchscreen");

  // Clear screen to black
  ILI9341_FillScreen(ILI9341_BLACK);



  // Simple QWERTY keyboard layout (like original working version)
  const int key_width = 28;   // Key width
  const int key_height = 28;  // Key height
  const int key_spacing = 2;  // Space between keys
  const int start_x = 5;      // Starting X position
  const int start_y = 10 + key_height + 10; // Starting Y position (shifted down)

  // Color palette selection
  uint16_t border_color, key_color, text_color;
  switch (KEYBOARD_PALETTE) {
    case KEYBOARD_PALETTE_CLASSIC:
      border_color = ILI9341_WHITE;
      key_color = ILI9341_CYAN;
      text_color = ILI9341_BLACK;
      break;
    case KEYBOARD_PALETTE_DARK:
      border_color = ILI9341_GRAY;
      key_color = ILI9341_NAVY;
      text_color = ILI9341_WHITE;
      break;
    case KEYBOARD_PALETTE_MODERN:
      border_color = ILI9341_BLACK;
      key_color = ILI9341_MAGENTA;
      text_color = ILI9341_WHITE;
      break;
    default:
      border_color = ILI9341_WHITE;
      key_color = ILI9341_CYAN;
      text_color = ILI9341_BLACK;
  }

  // Helper function to draw a key (inline implementation)
  #define drawKey(x, y, label, width_mult) \
    do { \
      int actual_width = key_width * width_mult + (width_mult - 1) * key_spacing; \
      ILI9341_FillRectangle(x - 1, y - 1, actual_width + 2, key_height + 2, border_color); \
      ILI9341_FillRectangle(x, y, actual_width, key_height, key_color); \
      /* Font selection */ \
      if (KEYBOARD_FONT == KEYBOARD_FONT_SMALL) { \
        int text_x = x + (actual_width - strlen(label) * 6) / 2; \
        int text_y = y + 6; \
        ILI9341_DrawString(text_x, text_y, label, text_color, key_color, 1, Font1); \
      } else if (KEYBOARD_FONT == KEYBOARD_FONT_MEDIUM) { \
        int text_x = x + (actual_width - strlen(label) * 12) / 2; \
        int text_y = y + 4; \
        ILI9341_DrawString(text_x, text_y, label, text_color, key_color, 2, Font1); \
      } \
    } while(0)

  // Numbers row: 1 2 3 4 5 6 7 8 9 0
  const char *numbers = "1234567890";
  int num_x = start_x;
  int num_y = start_y - key_height - key_spacing - 5; // Above keyboard

  for (int i = 0; numbers[i] != '\0'; i++) {
    char num_label[2] = {numbers[i], '\0'};
    drawKey(num_x, num_y, num_label, 1);
    num_x += key_width + key_spacing;
  }

  // Row 1: Q W E R T Y U I O P
  const char *qwerty1 = "QWERTYUIOP";
  int current_x = start_x;
  int current_y = start_y;

  for (int i = 0; qwerty1[i] != '\0'; i++) {
    char key_label[2] = {qwerty1[i], '\0'};
    drawKey(current_x, current_y, key_label, 1);
    current_x += key_width + key_spacing;
  }

  // Row 2: A S D F G H J K L
  const char *qwerty2 = "ASDFGHJKL";
  current_x = start_x + (key_width + key_spacing) / 2; // Offset for QWERTY layout
  current_y = start_y + key_height + key_spacing + 5;

  for (int i = 0; qwerty2[i] != '\0'; i++) {
    char key_label[2] = {qwerty2[i], '\0'};
    drawKey(current_x, current_y, key_label, 1);
    current_x += key_width + key_spacing;
  }

  // Row 3: Z X C V B N M
  const char *qwerty3 = "ZXCVBNM";
  current_x = start_x + (key_width + key_spacing); // Offset for QWERTY layout
  current_y = current_y + key_height + key_spacing + 5;

  for (int i = 0; qwerty3[i] != '\0'; i++) {
    char key_label[2] = {qwerty3[i], '\0'};
    drawKey(current_x, current_y, key_label, 1);
    current_x += key_width + key_spacing;
  }

  // Space bar
  current_x = start_x + 2 * (key_width + key_spacing);
  current_y = current_y + key_height + key_spacing + 10;
  int space_width = 5 * (key_width + key_spacing) - key_spacing;

  ILI9341_FillRectangle(current_x - 1, current_y - 1, space_width + 2, key_height + 2, border_color);
  ILI9341_FillRectangle(current_x, current_y, space_width, key_height, key_color);
  ILI9341_DrawString(current_x + space_width/2 - 20, current_y + 8, "SPACE", text_color, key_color, 1, Font1);

  LOG_Printf("Keyboard drawing complete");

  LOG_Printf("QWERTY keyboard layout drawn");

#endif

  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    osDelay(500);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
