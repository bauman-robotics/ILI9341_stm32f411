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
#include "keyboard_layout.h"
#include "touch.h"
#include "touch_calibration.h"

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

void TouchTask(void const * argument);
void CalibrationTask(void const * argument);
void LivePacketTask(void const * argument);

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

  #if ENABLE_TOUCHSCREEN
  // Create TouchTask for touchscreen handling
  //LOG_SendString("FREERTOS: Creating TouchTask...\r\n");
  osThreadDef(touchTask, TouchTask, osPriorityNormal, 0, 256);
  osThreadId touchTaskHandle = osThreadCreate(osThread(touchTask), NULL);
  if (touchTaskHandle == NULL) {
    //LOG_SendString("FREERTOS: ERROR - TouchTask creation failed!\r\n");
  } else {
    // LOG_SendString("FREERTOS: TouchTask created successfully\r\n");
  }
  #endif

  #if TOUCHSCREEN_CALIBRATION_ENABLED
  // Create CalibrationTask for touchscreen calibration
  osThreadDef(calibrationTask, CalibrationTask, CALIBRATION_TASK_PRIORITY, 0, CALIBRATION_TASK_STACK_SIZE);
  osThreadId calibrationTaskHandle = osThreadCreate(osThread(calibrationTask), NULL);
  if (calibrationTaskHandle == NULL) {
    //LOG_SendString("FREERTOS: ERROR - CalibrationTask creation failed!\r\n");
  } else {
    //LOG_SendString("FREERTOS: CalibrationTask created\r\n");
  }
  #endif

  #if ENABLE_LIVE_PACKET_TASK
  // Create LivePacketTask for live packet output (without logging to avoid USB conflicts)
  osThreadDef(livePacketTask, LivePacketTask, osPriorityBelowNormal, 0, 256);
  osThreadId livePacketTaskHandle = osThreadCreate(osThread(livePacketTask), NULL);
  #endif

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

  // Simple GPIO test at startup
  LOG_SendString("GPIO: Testing PC13 LED pin at startup\r\n");
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIOC->MODER &= ~GPIO_MODER_MODER13;    // Clear mode
  GPIOC->MODER |= GPIO_MODER_MODER13_0;   // Output mode
  GPIOC->BSRR = GPIO_PIN_13;              // Set HIGH
  LOG_SendString("GPIO: PC13 set to HIGH - check if LED turns off\r\n");

  LOG_Init();
  osDelay(1000); // Give USB CDC time to initialize
  LOG_Printf("System: Starting application\n");

  // Touchscreen initialization is now in TouchTask

  // Turn on backlight
  LOG_Printf("GPIO: Turning on backlight\n");
  HAL_GPIO_WritePin(BACK_LIGHT_GPIO_Port, BACK_LIGHT_Pin, GPIO_PIN_SET);

  // Initialize display
  ILI9341_Init();

  // Set rotation to landscape mode (working orientation)
  LOG_Printf("Setting display to landscape mode");

  #if ENABLE_TOUCHSCREEN
  LOG_SendString("FREERTOS: TouchTask was created\r\n"); // ЗДЕСЬ БЕЗОПАСНО
  #endif

  ILI9341_SetRotation(0x28);  // Landscape mode

  // Small delay after rotation
  osDelay(100);

  // Fill screen with black
  ILI9341_FillScreen(ILI9341_BLACK);

  // Additional delay to ensure display is ready
  osDelay(200);

  #if ENABLE_TOUCHSCREEN && TOUCHSCREEN_CALIBRATION_ENABLED
  // Start calibration immediately after display init
  LOG_SendString("MAIN: Starting touchscreen calibration\r\n");
  TOUCH_StartCalibration();
  #endif


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

  // Render the complete keyboard interface using the layout module
  render_keyboard_interface();

  LOG_Printf("QWERTY keyboard layout drawn");

#endif

  // Static variables for calibration state tracking
  static uint8_t last_calibration_active = 0;
  static uint8_t last_calibration_step = 0;
  static uint8_t touch_feedback_shown = 0;

  /* Infinite loop */
  for(;;)
  {
    // Handle calibration UI updates in main task (not in interrupt!)
    if (calibration_active == 1) {  // Calibration mode
        // Check if we need to update the UI after a touch
        if (calibration_step != last_calibration_step) {
            // Clear screen and show touch feedback
            ILI9341_FillScreen(ILI9341_BLACK);

            // Show blue touch point briefly
            if (!touch_feedback_shown && calibration_step > 0) {
                calibration_point_t *last_point = &calibration_points[calibration_step - 1];
                ILI9341_FillRectangle(last_point->raw_x - 2, last_point->raw_y - 2, 5, 5, ILI9341_BLUE);
                touch_feedback_shown = 1;
                osDelay(300);  // Show touch point for 300ms
            }

            // Clear screen again and redraw
            ILI9341_FillScreen(ILI9341_BLACK);

            if (calibration_step < 5) {
                // Redisplay title and instructions
                ILI9341_DrawStringLarge(10, 10, "Calibration Mode", ILI9341_WHITE, ILI9341_BLACK);
                ILI9341_DrawStringLarge(10, 35, "Touch the points", ILI9341_YELLOW, ILI9341_BLACK);

                // Draw next point (inline since function is static)
                if (calibration_step < 5) {
                    calibration_point_t *point = &calibration_points[calibration_step];
                    uint16_t color = point->collected ? ILI9341_GREEN : ILI9341_RED;
                    ILI9341_FillRectangle(point->display_x - 10, point->display_y - 1, 20, 3, color);
                    ILI9341_FillRectangle(point->display_x - 1, point->display_y - 10, 3, 20, color);
                    char num_str[2] = {'1' + calibration_step, '\0'};
                    ILI9341_DrawString(point->display_x + 15, point->display_y - 10, num_str, ILI9341_WHITE, ILI9341_BLACK, 2, 0);
                }

                LOG_Printf("TOUCH: Ready for point %d at X=%d, Y=%d\r\n",
                           calibration_step + 1,
                           calibration_points[calibration_step].display_x,
                           calibration_points[calibration_step].display_y);
            } else {
                // All points collected, show completion menu
                TOUCH_ShowCalibrationMenu();
            }

            last_calibration_step = calibration_step;
            touch_feedback_shown = 0;
        }
    }
    else if (calibration_active == 0 && last_calibration_active != 0) {
        // Calibration finished, redraw the normal interface
        ILI9341_FillScreen(ILI9341_BLACK);

        #if TASK_QWERTY_KEYBOARD == 1
        // Redraw keyboard after calibration
        render_keyboard_interface();
        LOG_SendString("TOUCH: Keyboard redrawn after calibration\r\n");
        #endif

        last_calibration_active = 0;
    }

    last_calibration_active = calibration_active;

    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    osDelay(500);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    /* Called if stack overflow is detected */
    LOG_Printf("STACK OVERFLOW in task: %s\r\n", pcTaskName);
    taskDISABLE_INTERRUPTS();
    while(1);
}

void vApplicationMallocFailedHook(void)
{
    /* Called if pvPortMalloc() fails */
    LOG_SendString("MALLOC FAILED - Out of heap memory!\r\n");
    taskDISABLE_INTERRUPTS();
    while(1);
}

void TouchTask(void const * argument) {
    LOG_SendString("TOUCH: TouchTask started\r\n");

    // Подождите немного перед инициализацией
    osDelay(100);

    // Initialize touchscreen in the task
    TOUCH_Init();

    LOG_SendString("TOUCH: Initialization complete\r\n");

    while (1) {
        // Check if touchscreen is currently touched
        if (TOUCH_IsTouched()) {
            // Small delay to debounce
            osDelay(10);

            // Read touch data if still touched
            if (TOUCH_IsTouched()) {
                touch_data_t touch_data;
                if (TOUCH_ReadData(&touch_data)) {
                    #if ENABLE_TOUCH_DEBUG
                    LOG_Printf("TOUCH: Event=%d, X=%d, Y=%d, Pressure=%d\r\n",
                               touch_data.event, touch_data.x, touch_data.y,
                               touch_data.pressure);
                    #endif
                }
            }
        }

        // Small delay to prevent CPU hogging
        osDelay(50);
    }
}

void CalibrationTask(void const * argument) {
    LOG_SendString("CALIBRATION: Task started\r\n");

    // Initialize calibration module
    TOUCH_CALIBRATION_Init();

    // // Wait for the specified delay before starting calibration
    osDelay(CALIBRATION_START_DELAY_MS);

    LOG_SendString("CALIBRATION: Starting calibration process\r\n");

    // Start calibration
    TOUCH_StartCalibration();

    // Task ends after starting calibration - UI updates are handled in StartDefaultTask
    LOG_SendString("CALIBRATION: Task completed\r\n");

    // Delete the task since it's no longer needed
    vTaskDelete(NULL);
}

/* USER CODE END Application */
